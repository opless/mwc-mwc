/*
 * File:	main.c
 */

/*
 * Includes.
 */
#include <sys/coherent.h>
#include <sys/devices.h>
#include <sys/fdisk.h>
#include <sys/proc.h>
#include <sys/seg.h>
#include <sys/stat.h>
#include <sys/typed.h>
#include <sys/param.h>

/*
 * Definitions.
 *	Constants.
 *	Macros with argument lists.
 *	Typedefs.
 *	Enums.
 */
#ifndef VERSION		/* This should be specified at compile time */
#define VERSION	"..."
#endif
#ifndef RELEASE
#define RELEASE "0"
#endif

/*
 * Functions.
 *	Import Functions.
 *	Export Functions.
 *	Local Functions.
 */
int read_cmos();

static void atcount();
static void rpdev();

/*
 * Global Data.
 *	Import Variables.
 *	Export Variables.
 *	Local Variables.
 */
extern dev_t rootdev;
extern dev_t pipedev;
extern int ronflag;
extern int PHYS_MEM;

/*
 * Patchable variable.
 *
 * PS1DRIVES is the number of PS1 type drives in the system. It is 0 unless you
 *     are actually on a PS1.
 */
int 	PS1DRIVES = 0;

short n_atdr;
char version[] = VERSION;
char release[] = RELEASE;
char copyright[] = "Copyright 1982,1993 Mark Williams Company\n";

unsigned long	_entry = 0;		/* really the serial number */
unsigned long	__ = 0;			/* really the serial number also */

main()
{
	register SEG *sp;
#ifdef _I386
	extern int BPFMAX;
	int speed1, speed2;
	char * ndpTypeName();
	extern int (*ndpEmFn)();
	extern short ndpType;
#else
	extern int realmode;
#endif

	CHIRP('a');

#ifdef _I386
	wrNdpUser(0);
	wrNdpSaved(0);
	u.u_bpfmax = BPFMAX;
#endif
	u.u_error = 0;
	bufinit();
	_CHIRP('0', 156);
	cltinit();
	_CHIRP('1', 156);
	pcsinit();
	_CHIRP('2', 156);
	seginit();
	_CHIRP('3', 156);
	atcount();
	_CHIRP('4', 156);
	rpdev();
	_CHIRP('5', 156);
	devinit();
	_CHIRP('6', 156);
#ifdef _I386
	rlinit();
	_CHIRP('7', 156);

	putchar_init();
	_CHIRP('8', 156);
	printf("*** COHERENT Version %s - 386 Mode.  %uKB free memory. ***\n",
		release, ctob(allocno())/1024);
	if ((int11() & 0x30) == 0x30)
		printf("Monochrome.  ");
	else
		printf("Color.  ");
	senseNdp();
	printf(ndpTypeName());
	if (ndpType <= 1 && ndpEmFn)
		printf("FP Emulation.  ");
#if 0
	if (int11() & 2)
		printf("x87.  ");
	else
		printf("No x87.  ");
#endif
#else
	printf("*** COHERENT Version %s - %s Mode.  %uKB free memory. ***\n",
		release, (realmode ? "Real" : "Protected"), msize);
#endif
	printf( "%u buffers.  %u clists.\n", NBUF, NCLIST);
	printf( "%uKB kalloc pool.  %u KB phys pool.\n",
	  ALLSIZE/1024, PHYS_MEM/1024);
	printf(copyright);

#ifdef _I386
	/*
	 * Make sure that we get a speed rating that does not cross 0.
	 */
	do {
		speed1 = read_t0();
		speed2 = read_t0();
	} while (speed1 < speed2);

	T_PIGGY(0x400,printf("CPU snail rating: %d\n", speed1 - speed2));
#endif /* _I386 */

	if (_entry) {
		printf("Serial Number ");
		printf("%U\n", _entry);
	}

	/*
	 * Verify correct serial number
	 */
	if (_entry != __)
panic("Verification error - call Mark Williams Company at +1-708-291-6700\n");

	/*
	 * Turn on clock, mount root device, start off processes
	 * and return.
	 */
	batflag = 1;
#ifdef _I386
	iprocp = SELF;
	CHIRP('b');
	if (pfork()) {
		CHIRP('i');
		idle();
	} else {
		fsminit();
		CHIRP('-');
		eprocp = SELF;
		eveinit();
		CHIRP('=');
	}
#else
	if ((sp=salloc((fsize_t)UPASIZE, SFNCLR|SFNSWP)) == NULL)
		panic("Cannot allocate user area");
	if ((iprocp=process(idle))==NULL || (eprocp=process(NULL))==NULL)
		panic("Cannot create process");
	eveinit(sp);
	fsminit();
#endif
	CHIRP('c');
}

/*
 * atcount()
 *
 * Read CMOS and return 0,1, or 2 as number of installed "at" drives.
 */
void
atcount()
{
	int u;
	n_atdr = 0;

        if (PS1DRIVES > 0)
		n_atdr = PS1DRIVES;
	else {
		/*
		 * Count nonzero drive types.
		 *
		 *	High nibble of CMOS 0x12 is drive 0's type.
		 *	Low  nibble of CMOS 0x12 is drive 1's type.
		 */
		u = read_cmos(0x12);
		if (u & 0x00F0)
			n_atdr++;
		if (u & 0x000F)
			n_atdr++;
	}
}

/*
 * rpdev()
 *
 * If rootdev is zero, try to use data from tboot to set it.
 * If pipedev is zero, make it 0x883 if ronflag == 1, else make it rootdev.
 * Call rpdev() AFTER calling atcount().
 */
static void rpdev()
{
	FIFO *ffp;
	typed_space *tp;
	int found;
	extern typed_space boot_gift;
	unsigned root_ptn, root_drv, root_maj, root_min;

	if (rootdev == makedev(0,0)) {
		found = 0;
		if (ffp = fifo_open(&boot_gift, 0)) {
			int rc;

			for (rc = 0; !found && (tp = fifo_read(ffp)); rc++) {
				BIOS_ROOTDEV *brp = (BIOS_ROOTDEV *)tp->ts_data;
				if (T_BIOS_ROOTDEV == tp->ts_type) {
					found = 1;
					root_ptn = brp->rd_partition;
				}
			}
			fifo_close(ffp);

		}

		if (found) {
			/*
			 * root_drv = BIOS # of root drive
			 * root_ptn = partition # in range 0..3
			 * if root on second "at" device, add 4 to minor #
			 * if root on second scsi device, add 16 to minor #
			 */
			root_drv = root_ptn/NPARTN;
			root_ptn %= NPARTN;
			if (n_atdr > root_drv) {
				root_maj = AT_MAJOR;
				root_min = root_drv*NPARTN + root_ptn;
			} else { /* root on SCSI device */
				root_maj = SCSI_MAJOR;
				root_min = (root_drv-n_atdr)*16 + root_ptn;
			}
			rootdev = makedev(root_maj, root_min);
			printf("rootdev=(%d,%d)\n", root_maj, root_min);
		}
	}
	if (pipedev == makedev(0,0)) {
		if (ronflag)
			pipedev = makedev(RM_MAJOR, 0x83);
		else
			pipedev = rootdev;
		printf("pipedev=(%d,%d)\n", (pipedev>>8)&0xff, pipedev&0xff);
	}
}
