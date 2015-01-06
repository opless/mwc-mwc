/*
 * dosdrives.c
 * 12/5/90
 * Display MS-DOS drive information.
 */

#include <stdio.h>
#include <sys/fdisk.h>

extern	long lseek();

/* Globals. */
char	buf[512];
char	buf2[512];
char	*devp;
char	devname[10];
int	drive;
int	fd;
int	pass;

/*
 * List of partition tables to search.
 * This must be updated as new devices are added to the system.
 */
char *devices[] = {
	"/dev/at0x", "/dev/at1x",
	"/dev/sd0x", "/dev/sd1x", "/dev/sd2x", "/dev/sd3x",
	"/dev/sd4x", "/dev/sd5x", "/dev/sd6x"
};
#define	DEVMAX	(sizeof(devices)/sizeof(devices[0]))

void	checkdev();
void	fatal();

main(argc, argv) int argc; char	*argv[];
{
	register int dev;

	printf("A:\nB:\n");
	drive = 2;
	for (pass = 1; pass <= 2; pass++) {
		for (dev = 0; dev < DEVMAX; dev++) {
			devp = devices[dev];
			if ((fd = open(devp, 0)) < 0)
				continue;
			checkdev();
			close(fd);
		}
	}
	exit(0);
}

void
checkdev()
{
	register int i, j, sys;
	FDISK_S	*fdp1, *fdp2;
	HDISK_S	*hdp, *hdp2;
	long partseek1, partseek2;

	/* Read the partition table. */
	if (read(fd, buf, sizeof(buf)) != sizeof(buf))
		fatal("read1 failed on \"%s\"", devp);
	hdp = buf;
	if (hdp->hd_sig != HDSIG)
		return;				/* no signature, ignore */
	for (i = 0; i < NPARTN; i++) {
		sys = (hdp->hd_partn[i]).p_sys;
		if (sys != SYS_DOS_12
		 && sys != SYS_DOS_16
		 && sys != SYS_DOS_XP
		 && sys != SYS_DOS_LARGE)
			continue;		/* not a DOS partition */
		strcpy(devname, devp);
		devname[strlen(devname)-1] = 'a' + i;
		if (pass == 1 && sys != SYS_DOS_XP) {
			/* Count non-extended partitions on pass 1. */
			printf("%c: %s\n", 'A'+drive, devname);
			++drive;
			continue;
		} else if (pass == 2 && sys == SYS_DOS_XP) {
			/* Count extended partitions on pass 2. */
			fdp2 = &(hdp->hd_partn[i]);
			partseek1 = fdp2->p_base * 512;	/* base of first */
			partseek2 = 0L;			/* offset from first */
			for (j = 1; ; j++) {
				if (lseek(fd, partseek1+partseek2, 0) == -1L)
					fatal("lseek failed");
				/* Read the extended DOS drive partition table. */
				if (read(fd, buf2, sizeof(buf2)) != sizeof(buf2))
					fatal("read2 failed on \"%s\" at %lx",
						devp, partseek1+partseek2);
				hdp2 = buf2;
				fdp1 = &(hdp2->hd_partn[0]);
				fdp2 = &(hdp2->hd_partn[1]);
				printf("%c: %s %d\n", 'A'+drive, devname, j);
				++drive;
				if (fdp2->p_sys != SYS_DOS_XP)
					break;		/* done */
				partseek2 = fdp2->p_base * 512;
			}
		}
	}
}

void
fatal(s) char *s;
{
	fprintf(stderr, "dosdrives: %r\n", &s);
	exit(1);
}

/* end of dosdrives.c */
