/*
 * File:	asy.c
 *
 * Purpose:	8250-family async port device driver
 *
 *	Devices are named /dev/asy[00..31]{fpl}
 *	Minor number bit assignments:
 *		x... ....	1 for NO modem control, "l"
 *		.x.. ....	1 for polled operation (no irq service), "p"
 *		..x. ....	1 for RTS/CTS flow control, "f"
 *		...x xxxx	channel number - 0..31
 */

/*
 * -----------------------------------------------------------------
 * Includes.
 */
#include <sys/coherent.h>
#include <sys/stat.h>
#include <sys/proc.h>
#include <sys/tty.h>
#include <sys/con.h>
#include <sys/devices.h>
#include <sys/errno.h>
#include <poll.h>
#include <sys/sched.h>		/* CVTTOUT, IVTTOUT, SVTTOUT */
#include <sys/asy.h>
#include <sys/ins8250.h>
#include <sys/poll_clk.h>
#ifdef _I386
#include <termio.h>
#endif

/*
 * -----------------------------------------------------------------
 * Definitions.
 *	Constants.
 *	Macros with argument lists.
 *	Typedefs.
 *	Enums.
 */
#define	IEN_USE_MSI	(IE_RxI | IE_TxI | IE_LSI | IE_MSI)
#define	IEN_NO_MSI	(IE_RxI | IE_TxI | IE_LSI)

#define CTLQ	0021
#define CTLS	0023

#define NUM_IRQ		16	/* PC allows irq numbers 0..15		*/
#define BPB		8	/* 8 bits per byte			*/
#define DTRTMOUT  	3	/* DTR seconds for close		*/
#define LOOP_LIMIT	100	/* safety valve on irq loops		*/

/*
 * For rawin silo (see poll_clk.h), use last element of si_buf to count
 * the number of characters in the silo.
 */
#define SILO_CHAR_COUNT	si_buf[SI_BUFSIZ-1]
#define SILO_HIGH_MARK	(SI_BUFSIZ-SI_BUFSIZ/4)
#define SILO_LOW_MARK	(SI_BUFSIZ/4)
#define MAX_SILO_INDEX	(SI_BUFSIZ-2)
#define MAX_SILO_CHARS	(SI_BUFSIZ-1)

#define RAWIN_FLUSH(in_silo)	{ \
  in_silo->si_ox = in_silo->si_ix; \
  in_silo->SILO_CHAR_COUNT = 0; }
#define RAWOUT_FLUSH(out_silo)	{ out_silo->si_ox = out_silo->si_ix; }
#define channel(dev)	(dev & 0x1F)

#define IEN	((a0->a_nms)?IEN_NO_MSI:IEN_USE_MSI)
#ifdef _I386
#define	EEBUSY	EBUSY
#else
#define	EEBUSY	EDBUSY
#endif

#define NW_OUTSILO	1	/* bits in need_wake[] entries		*/

typedef void (* VPTR)();	/* pointer to function returning void	*/
typedef void (* FPTR)();	/* pointer to function returning int	*/

/*
 * -----------------------------------------------------------------
 * Functions.
 *	Import Functions.
 *	Export Functions.
 *	Local Functions.
 */
int nulldev();

void asy_putchar();

/*
 * Configuration functions (local).
 */
static void asyclose();
static void asyioctl();
static void asyload();
static void asyopen();
static void asyread();
static void asytimer();
static void asyunload();
static void asywrite();
static int asypoll();
static void cinit();

/*
 * Support functions (local).
 */
static void add_irq();
static void asy_irq();
static int asy_send();
static void asybreak();
static void asyclock();
static void asycycle();
static void asydump();
static int asyintr();
static void asyparam();
static void asysph();
static void asyspr();
static void asystart();
static void endbrk();
static void irqdummy();
static void upd_irq1();

static void i2(),i3(),i4(),i5(),i6(),i7(),i8(),i9();
static void i10(),i11(),i12(),i13(),i14(),i15();
static int p1(),p2(),p3(),p4();

/*
 * -----------------------------------------------------------------
 * Global Data.
 *	Import Variables.
 *	Export Variables.
 *	Local Variables.
 */
extern int albaud[], alp_rate[];

/*
 * When asypatch runs, it checks whether its internal value for
 * ASY_VERSION matches this driver's value, so as to prevent the patch
 * utility and the driver from getting out of phase.
 */
int ASY_VER = ASY_VERSION;
int ASY_HPCL = 1;
int ASY_NUM = 0;
int ASYGP_NUM = 0;
asy0_t asy0[MAX_ASY] = {
	{ 0 }
};
asy_gp_t asy_gp[MAX_ASYGP] = {
	{ 0 }
};

static asy1_t * asy1;		/* unused entries have type US_NONE	*/
static short dummy_port;	/* used only during driver startup	*/
static int poll_divisor;	/* set by asyspr(), read by asyclk()	*/
static char pptbl[MAX_ASY];	/* channel numbers of polled ports	*/
static int ppnum;		/* number of channels in pptbl		*/

/*
 * itbl keeps function pointers for irq service routines, for ease of setting
 *   and clearing vectors.
 *
 * irq0[x] and irq1[x] are lists for irq number x.
 * irq0 has nodes that may possibly cause an irq.
 * irq1 contains nodes for active devices.
 * Whenever a device becomes active or inactive, irq1 is rebuilt from irq0.
 *
 * nodespace is an array of available nodes used in making the lists.
 * nextnode points to the next free node.
 * Nodes are taken from nodespace only during driver load.
 */
static VPTR itbl[NUM_IRQ] = {
	0,0,i2,i3,i4,i5,i6,i7,i8,i9,i10,i11,i12,i13,i14,i15 };
static FPTR ptbl[PT_MAX] = { asyintr,p1,p2,p3,p4 };
static struct irqnode *irq0[NUM_IRQ], *irq1[NUM_IRQ];
static struct irqnode nodespace[MAX_ASY];
static char need_wake[MAX_ASY];
static char nextnode;
static int initialized;	/* for asy_putchar() */

/*
 * Configuration table (export data).
 */
CON asycon ={
	DFCHR|DFPOL,			/* Flags */
	ASY_MAJOR,			/* Major index */
	asyopen,			/* Open */
	asyclose,			/* Close */
	nulldev,			/* Block */
	asyread,			/* Read */
	asywrite,			/* Write */
	asyioctl,			/* Ioctl */
	nulldev,			/* Powerfail */
	asytimer,			/* Timeout */
	asyload,			/* Load */
	asyunload,			/* Unload */
	asypoll				/* Poll */
};

/*
 * -----------------------------------------------------------------
 * Code.
 */

/*
 * asyload()
 */
static void
asyload()
{
	int	s, chan;
	asy0_t	*a0;
	asy1_t	*a1;
	TTY	*tp;
	short	port;
	char	irq;
	char	speed;
	char	g;
	char	sense_ct = 0;

	/*
	 * Allocate space for asy structs.  Possible error return.
	 */
	asy1 = (asy1_t *)kalloc(ASY_NUM * sizeof(asy1_t));
	if (asy1 == 0) {
		printf("asyload: can't allocate space for %d async devices\n",
		  ASY_NUM);
		return;
	}
	kclear(asy1, ASY_NUM*sizeof(asy1_t));

	/*
	 * For each non-null port:
	 * 	if port is uses irq
	 *		set dummy routine in case uart_sense causes bogus irpts
	 *	sense chip type
	 *	write baud rate to sgtty/termio structs
	 *	disable port interrupts
	 *	hang up port
	 *	set default baud rate (also resets UART)
	 *	hook "start" function into line discipline module
	 *	hook "param" function into line discipline module
	 *	hook CS into line discipline module
	 * 	if port is uses irq
	 *		release dummy routine
	 *		if not in a port group
	 *			add to irq list
	 */
	for (chan = 0; chan < ASY_NUM; chan++) {
		a0 = asy0 + chan;
		a1 = asy1 + chan;
		tp = &a1->a_tty;
		speed = a0->a_speed;
		tp->t_sgttyb.sg_ispeed = tp->t_sgttyb.sg_ospeed = speed;
		tp->t_dispeed = tp->t_dospeed = speed;
		port = a0->a_port;

		/*
		 * A port address of zero means a skipped entry in the table.
		 * In this case a1->a_ut keeps its initial value of US_NONE.
		 */
		if (port) {
			dummy_port = port;
			if (a0->a_irqno)
				setivec(a0->a_irqno, irqdummy);
			/*
			 * uart_sense() prints port info.
			 * Do this four times per line.
			 */
			a1->a_ut = uart_sense(port);
			sense_ct++;
			if ((sense_ct & 1) == 0)
				putchar('\n');
			else
				putchar('\t');
			s = sphi();
			outb(port+MCR, 0);
			outb(port+LCR, LC_DLAB);
			outb(port+DLL, albaud[speed]);
			outb(port+DLH, albaud[speed] >> 8);
			outb(port+LCR, LC_CS8);
			tp->t_start = asystart;
			/* leave tp->t_param at 0 */
			tp->t_cs_sel = cs_sel();
			tp->t_ddp = (int *)chan;
			spl(s);
			if (a0->a_irqno) {
				clrivec(a0->a_irqno);
				if (a0->a_asy_gp == NO_ASYGP)
					add_irq(a0->a_irqno, asyintr, chan);
			}
		}
	}
	if (sense_ct & 1)
		putchar('\n');

	/*
	 * for each port group
	 *	add group to irq list
	 */
	for (g = 0; g < ASYGP_NUM; g++) {
		add_irq(asy_gp[g].irq, ptbl[asy_gp[g].gp_type], g);
	}

	/*
	 * Attach irq routines.
	 */
	for (irq = 0; irq < NUM_IRQ; irq++)
		if (irq0[irq]) {
			setivec(irq, itbl[irq]);
		}
}

/*
 * asyunload()
 */
static void
asyunload()
{
	char chan, irq;

	/*
	 * for each channel
	 *	disable UART interrupts
	 *	hangup port
	 *	cancel timer
	 */
	for (chan = 0; chan < ASY_NUM; chan++) {
		asy0_t * a0 = asy0 + chan;
		asy1_t * a1 = asy1 + chan;
		short port = a0->a_port;
		TTY *tp = &a1->a_tty;

		outb(port+IER, 0);
		outb(port+MCR, 0);
		timeout(tp->t_rawtim, 0, NULL, 0);
	}

	/*
	 * for each irq
	 *	if irq routine was attached
	 *		detach it
	 */
	for (irq = 0; irq < NUM_IRQ; irq++)
		if (irq0[irq])
			clrivec(irq);

	/*
	 * Deallocate dynamic asy storage.
	 */
	if (asy1)
		kfree(asy1);
}

/*
 * asyopen()
 */
static void
asyopen(dev, mode)
dev_t dev;
int mode;
{
	int	s;
	char	msr, mcr;
	char	chan = channel(dev);
	asy0_t	*a0 = asy0 + chan;
	asy1_t	*a1 = asy1 + chan;
	TTY	*tp = &a1->a_tty;
	short	port = a0->a_port;

	if (a1->a_ut == US_NONE) { /* chip not found */
		T_HAL(4, devmsg(dev, "no UART"));
		u.u_error = ENXIO;
		goto bad_open;
	}

	if ((tp->t_flags & T_EXCL) && !super()) {
		T_HAL(4, devmsg(dev, "exclusive use"));
		u.u_error = ENODEV;
		goto bad_open;
	}

#if 0
	if (drvl[major(dev)].d_time != 0) {	/* Modem settling */
		T_HAL(4, devmsg(dev, "modem settling"));
		u.u_error = EEBUSY;
		goto bad_open;
	}
#endif

	/*
	 * Can't open for hardware flow control if modem status
	 * interrupts are disallowed.
	 */
	if (a0->a_nms && (dev & CFLOW)) {
		T_HAL(4, devmsg(dev, "no modem status irq's"));
		u.u_error = ENXIO;
		goto bad_open;
	}

	/*
	 * Can't open a polled port if another driver is using polling.
	 */
	if (dev & CPOLL && poll_owner & ~ POLL_ASY) {
		T_HAL(4, devmsg(dev, "polling unavailable"));
		u.u_error = EEBUSY;
		goto bad_open;
	}

	/*
	 * Can't have both com[13] or both com[24] IRQ at once.
	 */
	if (!(dev & CPOLL) && a0->a_ixc) {
		struct irqnode *np = irq1[a0->a_irqno];
		while (np) {
			if (np->func != ptbl[0] || np->arg != chan) {
				T_HAL(4, devmsg(dev, "irq conflict"));
				u.u_error = EEBUSY;
				goto bad_open;
			}
			np = np->next_actv;
		}
	}

	/*
	 * If port already in use, are new and old open modes compatible?
	 */
	if (a1->a_in_use) {
		int oldmode = 0, newmode = 0; /* mctl:1 irq:2 flow:4 */

		if (a1->a_modc)
			oldmode += 1;
		if (a1->a_irq)
			oldmode += 2;
		if (a1->a_flc)
			oldmode += 4;
		if ((dev & NMODC) == 0)
			newmode += 1;
		if ((dev & CPOLL) == 0)
			newmode += 2;
		if (dev & CFLOW)
			newmode += 4;
		if (oldmode != newmode) {
			T_HAL(4, devmsg(dev, "conflicting open modes"));
			u.u_error = EEBUSY;
			goto bad_open;
		}
	}

	/*
	 * Sleep here if another process is opening or closing the port.
	 * This can happen if:
	 *   another process is trying a first open and awaiting CD;
	 *   another process is closing the port after losing CD;
	 *   a remote process opened the port, spawned a daemon,
	 *     and disconnected, and the daemon ignored SIGHUP and is
	 *     improperly keeping the port open.
	 * Don't try to set tp->t_flags before this sleep!  During
	 *   the sleep, ttclose() may be called and clear the flags.
	 */
	while (a1->a_in_use && (a1->a_hcls ||
	  ((dev & NMODC) == 0 && (inb(port+MSR) & MS_RLSD) == 0))) {
#ifdef _I386
		if (x_sleep ((char *) & tp->t_open, pritty, slpriSigCatch,
			     "asyblk") == PROCESS_SIGNALLED) {
#else
		v_sleep((char *)(&tp->t_open), CVTTOUT, IVTTOUT, SVTTOUT,
		  "asyblk");
		if (nondsig ()) {  /* signal? */
#endif
			u.u_error = EINTR;
			goto bad_open;
		}
	}

	/*
	 * If channel not in use, mark it as such.
	 */
	if (a1->a_in_use == 0) {
		/*
		 * Save modes for this open attempt to avoid future conflicts.
		 * Then start asycycle() for this port.
		 */
		if (dev & NMODC) {
			tp->t_flags &= ~T_MODC;
			a1->a_modc = 0;
		} else {
			tp->t_flags |= T_MODC;
			a1->a_modc = 1;
		}
		if (dev & CPOLL)
			a1->a_irq = 0;
		else
			a1->a_irq = 1;
		if (dev & CFLOW) {
			tp->t_flags |= T_CFLOW;
			a1->a_flc = 1;
		} else {
			tp->t_flags &= ~T_CFLOW;
			a1->a_flc = 0;
		}
	}
	a1->a_in_use++;

	/*
	 * From here, error exit is bad_open_u.
	 */

	if (tp->t_open == 0) {        /* not already open */
		silo_t	* in_silo = &a1->a_in;

		if (!(dev & CPOLL)) {
			upd_irq1(a0->a_irqno);
			a1->a_has_irq = 1;
		}

		/*
		 * Need to start cycling to scan for CD.
		 */
		asycycle(chan);

		s = sphi();
		/*
		 * Raise basic modem control lines even if modem
		 * control hasn't been specified.
		 * MC_OUT2 turns on NON-open-collector IRQ line from the UART.
		 * since we can't have two UART's on same IRQ with MC_OUT2 on
		 */
		mcr = MC_RTS | MC_DTR;
		if (dev & CPOLL) {
			outb(port+MCR, mcr);
		} else {
			outb(port+MCR, mcr | a0->a_outs);
			outb(port+IER, IEN);
		}

		if ((dev & NMODC) == 0) {	/* want modem control? */
			tp->t_flags |= T_HOPEN | T_STOP;
			for (;;) {	/* wait for carrier */
				msr = inb(port+MSR);
				/*
				 * If carrier detect present
				 *   if port not already open
				 *     break out of loop and finish first open
				 *   else
				 *     do second (or third, etc.) open
				 */
				if (msr & MS_RLSD)
					break;
				/* wait for carrier */
#ifdef _I386
				if (x_sleep ((char *) & tp->t_open, pritty,
					     slpriSigCatch, "need CD")
				    == PROCESS_SIGNALLED) {
#else
	   	  		v_sleep((char *)(&tp->t_open), CVTTOUT, IVTTOUT,
				  SVTTOUT, "need CD");
		 		if (nondsig ()) {  /* signal? */
#endif
					outb(port+MCR, 0);
			    		outb(port+IER, 0);
					u.u_error = EINTR;
					tp->t_flags &= ~(T_HOPEN | T_STOP);
					spl(s);
					goto bad_open_u;
				}
			}

			/*
			 * Mark that we are no longer hanging in open.
			 * Allow output over the port unless hardware flow
			 * control says not to.
			 */
			tp->t_flags &= ~T_HOPEN;
			tp->t_flags &= ~T_STOP;
			if (!(tp->t_flags & T_CFLOW) || (msr & MS_CTS))
				a1->a_ohlt = 0;
			else
				a1->a_ohlt = 1;

			/*
			 * Awaken any other opens on same device.
			 */
			wakeup((char *)(&tp->t_open));
		}
		ttopen(tp);				/* stty inits */
		tp->t_flags |= T_CARR;
		if (ASY_HPCL)
			tp->t_flags |= T_HPCL;

		asyparam(tp);	/* gimmick: do this while t_open is zero */

		/*
		 * TO DO: flush UART input register(s).
		 */

		spl(s);

		/*
		 * Turn on polling for the port.
		 */
		if (dev & CPOLL) {
			a1->a_poll = 1;
			asyspr();
		}
	} /* end of first-open case */

	tp->t_open++;
	T_HAL(0x400, printf("ch%d open + %d\n", chan, tp->t_open));
	ttsetgrp(tp, dev, mode);

	return;

bad_open_u:
	a1->a_in_use--;
	wakeup((char *)(&tp->t_open));
bad_open:
	return;
}

/*
 * asyclose()
 */
static void
asyclose(dev, mode)
dev_t dev;
int mode;
{
	int	chan = channel(dev);
	asy0_t	*a0 = asy0 + chan;
	asy1_t	*a1 = asy1 + chan;
	TTY	*tp = &a1->a_tty;
	silo_t	* out_silo = &a1->a_out;
	silo_t	* in_silo = &a1->a_in;
	int	flags, maj;
	int	s;
	short	port = a0->a_port;
	char	lsr;

	if (--tp->t_open)
		goto not_last_close;
	T_HAL(0x400, printf("ch%d open - %d\n", chan, tp->t_open));
	s = sphi();

	a1->a_hcls = 1;			/* disallow reopen til done closing */
	flags = tp->t_flags;		/* save flags - ttclose zeroes them */
	ttclose(tp);

	/*
	 * Wait for output silo and UART xmit buffer to empty.
	 * Allow signal to break the sleep.
	 */
	for (;;) {
		int chipEmpty = 0, siloEmpty = 0;

		lsr = inb(port + LSR);
		chipEmpty = (lsr & LS_TxIDLE);
		T_HAL(0x400, printf("ch%d chipEmpty=%d\n", chan, chipEmpty));
		siloEmpty = (out_silo->si_ix == out_silo->si_ox);
		T_HAL(0x400, printf("ch%d siloEmpty=%d\n", chan, siloEmpty));

		if (chipEmpty && siloEmpty)
			break;
		need_wake[chan] |= NW_OUTSILO;
#ifdef _I386
		if (x_sleep ((char *) out_silo, pritty, slpriSigCatch,
			     "asyclose") == PROCESS_SIGNALLED) {
#else
		v_sleep((char *)out_silo, CVTTOUT, IVTTOUT, SVTTOUT,
		  "asyclose");
		if (nondsig ()) {  /* signal? */
#endif
			RAWOUT_FLUSH(out_silo);
			break;
		}
	}
	need_wake[chan] &= ~NW_OUTSILO;

	/*
	 * If not hanging in open
	 */
	if ((flags & T_HOPEN) == 0) {
		/*
		 * Disable interrupts.
		 */
		outb(port+IER, 0);
		outb(port+MCR, inb(port+MCR) & ~MC_OUTS);
	}

	/*
	 * If hupcls
	 */
	if (flags & T_HPCL) {
		T_HAL(0x400, printf("ch%d drop DTR\n", chan));
		/*
		 * Hangup port - drop DTR and RTS.
		 */
		outb(port+MCR, inb(port+MCR) & MC_OUTS);

		/*
		 * Hold dtr low for timeout
		 */
		maj = major(dev);
		drvl[maj].d_time = 1;
#ifdef _I386
		x_sleep ((char *) & drvl [maj].d_time, pritty, slpriNoSig,
			 "drop DTR");
#else
		v_sleep((char *)&drvl[maj].d_time, CVTTOUT, IVTTOUT, SVTTOUT,
		  "drop DTR");
#endif
		drvl[maj].d_time = 0;
	}

	a1->a_poll = 0;
	asyspr();
	RAWIN_FLUSH(in_silo);
	a1->a_hcls = 0;		/* allow reopen - done closing */
	wakeup((char *)(&tp->t_open));
	spl(s);
	a1->a_in_use--;

	if (!(dev & CPOLL))
		upd_irq1(a0->a_irqno);
	return;

not_last_close:
	T_HAL(0x400, printf("ch%d open - %d\n", chan, tp->t_open));
	a1->a_in_use--;
	wakeup((char *)(&tp->t_open));
	return;
}

/*
 * asyread()
 */
static void
asyread(dev, iop)
dev_t dev;
register IO * iop;
{
	int 	chan = channel(dev);
	asy1_t	*a1 = asy1 + chan;
	TTY	*tp = &a1->a_tty;

	ttread(tp, iop);
}

/*
 * asytimer()
 */
static void
asytimer(dev)
dev_t dev;
{
	if (++drvl[major(dev)].d_time > DTRTMOUT)
		wakeup((char *)&drvl[major(dev)].d_time);
}

/*
 * asywrite()
 */
static void
asywrite(dev, iop)
dev_t dev;
register IO * iop;
{
	int	chan = channel(dev);
	asy0_t	*a0 = asy0 + chan;
	asy1_t	*a1 = asy1 + chan;
	TTY	*tp = &a1->a_tty;
	short	port = a0->a_port;
	register int c;

	/*
	 * Treat user writes through tty driver.
	 */
	if (iop->io_seg != IOSYS) {
		ttwrite(tp, iop);
		return;
	}

	/*
	 * Treat kernel writes by blocking on transmit buffer.
	 */
	while ((c = iogetc(iop)) >= 0) {
		/*
		 * Wait until transmit buffer is empty.
		 * Check twice to prevent critical race with interrupt handler.
		 */
		for (;;) {
			if (inb(port+LSR) & LS_TxRDY)
				if (inb(port+LSR) & LS_TxRDY)
					break;
		}

		/*
		 * Output the next character.
		 */
		outb(port+DREG, c);
	}
}

/*
 * asyioctl()
 */
static void
asyioctl(dev, com, vec)
dev_t	dev;
int	com; struct sgttyb *vec;
{
	int	chan = channel(dev);
	asy0_t	*a0 = asy0 + chan;
	asy1_t	*a1 = asy1 + chan;
	TTY	*tp = &a1->a_tty;
	int	s;
	int	temp;
	silo_t	*out_silo = &a1->a_out;
	silo_t	*in_silo = &a1->a_in;
	short	port = a0->a_port;
	unsigned char	msr, mcr, lcr, ier;
	char	do_ttioctl = 0;
	char	do_asyparam = 0;

	s = sphi();
	ier = inb(port+IER);
	mcr = inb(port+MCR);		/* get current MCR register status */
	lcr = inb(port+LCR);		/* get current LCR register status */

#ifdef _I386
	/*
	 * If command will drain output, do the drain now
	 * before calling ttioctl().
	 * Don't do this for 286 kernel:  we're running out of code space.
	 */
	switch(com) {
	case TCSETAW:
	case TCSETAF:
	case TCSBRK:
	case TIOCSETP:
		/*
		 * Wait for output silo and UART xmit buffer to empty.
		 * Allow signal to break the sleep.
		 */
		for (;;) {
			if (!ttoutp(tp)
			  && (out_silo->si_ix == out_silo->si_ox)
			  && (inb(port + LSR) & LS_TxIDLE))
				break;
			need_wake[chan] |= NW_OUTSILO;
#ifdef _I386
			if (x_sleep ((char *) out_silo, pritty, slpriSigCatch,
				     "asydrain") == PROCESS_SIGNALLED) {
#else
			v_sleep((char *)out_silo, CVTTOUT, IVTTOUT, SVTTOUT,
			  "asydrain");
			if (nondsig ()) {  /* signal? */
#endif
				break;
			}
		}
		need_wake[chan] &= ~NW_OUTSILO;
	}
#endif

	switch(com) {
	case TIOCSBRK:			/* set BREAK */
		outb(port+LCR, lcr|LC_SBRK);
		break;
	case TIOCCBRK:			/* clear BREAK */
		outb(port+LCR, lcr & ~LC_SBRK);
		break;
	case TIOCSDTR:			/* set DTR */
		outb(port+MCR, mcr|MC_DTR);
		break;
	case TIOCCDTR:			/* clear DTR */
		outb(port+MCR, mcr & ~MC_DTR);
		break;
	case TIOCSRTS:			/* set RTS */
		outb(port+MCR, mcr|MC_RTS);
		break;
	case TIOCCRTS:			/* clear RTS */
		outb(port+MCR, mcr & ~MC_RTS);
		break;
	case TIOCRSPEED:		/* set "raw" I/O speed divisor */
		outb(port+LCR, lcr|LC_DLAB);  /* set speed latch bit */
		outb(port+DLL, (unsigned) vec);
		outb(port+DLH, (unsigned) vec >> 8);
		outb(port+LCR, lcr);       /* reset latch bit */
		break;
	case TIOCWORDL:		/* set word length and stop bits */
		outb(port+LCR, ((lcr&~0x7) | ((unsigned) vec & 0x7)));
		break;
	case TIOCRMSR:		/* get CTS/DSR/RI/RLSD (MSR) */
		msr = inb(port+MSR);
		temp = msr >> 4;
		kucopy(&temp, (unsigned *) vec, sizeof(unsigned));
		break;
	case TIOCFLUSH:		/* Flush silos here, queues in tty.c */
		RAWIN_FLUSH(in_silo);
		RAWOUT_FLUSH(out_silo);
		do_ttioctl = 1;
		break;

		/*
		 * If port parameters change, plan to call asyparam().
		 * Need to check now before structs are updated.
		 */
#ifdef _I386
	case TCSETA:
	case TCSETAW:
	case TCSETAF:
		{
			struct	termio	trm;

			ukcopy(vec, &trm, sizeof(struct termio));
			if (trm.c_cflag != tp->t_termio.c_cflag)
				do_asyparam = 1;
		}
		do_ttioctl = 1;
		break;
#endif
	case TIOCSETP:
	case TIOCSETN:
		{
			struct	sgttyb	sg;

			ukcopy(vec, &sg, sizeof(struct sgttyb));
			if (sg.sg_ispeed != tp->t_sgttyb.sg_ispeed
			  || ((sg.sg_flags ^ tp->t_sgttyb.sg_flags) & ANYP))
				do_asyparam = 1;
		}
		do_ttioctl = 1;
		break;
	default:
		do_ttioctl = 1;
	}
	outb(port+IER, ier);
	if (do_ttioctl)
		ttioctl(tp, com, vec);
	spl(s);
	if (do_asyparam)
		asyparam(tp);
#ifdef _I386
	/*
	 * Things to be done after calling ttioctl().
	 */
	switch(com) {
	case TCSBRK:
		/*
		 * Send 0.25 second break:
		 * 1.  Turn on break level.
		 * 2.  Set timer to turn off break level 0.25 sec later.
		 * 3.  Sleep till timer expires.
		 * 4.  Turn off break level.
		 */
		outb(port+LCR, lcr|LC_SBRK);
		a1->a_brk = 1;
		timeout(&tp->t_sbrk, HZ/4, endbrk, chan);
		while(a1->a_brk) {
#ifdef _I386
			x_sleep (a1, pritty, slpriNoSig, "asybreak");
#else
			v_sleep(a1, CVTTOUT, IVTTOUT, SVTTOUT, "asybreak");
#endif
		}
		outb(port+LCR, lcr & ~LC_SBRK);
	}
#endif
}

#ifdef _I386
/*
 * Turn off the break level.
 * Called from timeout after ioctl(fd, TCSBRK, 0).
 */
void
endbrk(chan)
int chan;
{
	asy1_t	*a1 = asy1 + chan;

	a1->a_brk = 0;
	wakeup(a1);
}
#endif

/*
 * asyparam()
 */
static void
asyparam(tp)
TTY * tp;
{
	int	chan = (int)tp->t_ddp;
	asy0_t	*a0 = asy0 + chan;
	asy1_t	*a1 = asy1 + chan;
	short	port = a0->a_port;
	int	s;
	int	write_baud=1, write_lcr=1;
	unsigned char	mcr, newlcr, speed, oldSpeed;

#ifdef _I386
	unsigned short cflag = tp->t_termio.c_cflag;

	T_HAL(4, printf("ch%d asyparam cflag=%x\n", chan, cflag));
	speed = cflag & CBAUD;
	switch (cflag & CSIZE) {
	case CS5:  newlcr = LC_CS5;  break;
	case CS6:  newlcr = LC_CS6;  break;
	case CS7:  newlcr = LC_CS7;  break;
	case CS8:  newlcr = LC_CS8;  break;
	}
	if (cflag & CSTOPB)
		newlcr |= LC_STOPB;
	if (cflag & PARENB) {
		newlcr |= LC_PARENB;
		if ((cflag & PARODD) == 0)
			newlcr |= LC_PAREVEN;
	}
#else
	speed = tp->t_sgttyb.sg_ispeed;
	switch (tp->t_sgttyb.sg_flags & (EVENP|ODDP|RAW)) {
	case ODDP:
		newlcr = LC_CS7|LC_PARENB;
		break;
	case EVENP:
		newlcr = LC_CS7|LC_PARENB|LC_PAREVEN;
		break;
	default:
		newlcr = LC_CS8;
		break;
	}
#endif

	/*
	 * Don't bang on the UART needlessly.
	 * Writing baud rate resets the port, which loses characters.
	 * You want this on first open, NOT on later opens.
	 */
	oldSpeed = a0->a_speed;
	if (speed == oldSpeed && tp->t_open) {
		write_baud = 0;
		if (newlcr == a1->a_lcr) {
			write_lcr = 0;
		}
	}
	a0->a_speed = speed;
	a1->a_lcr = newlcr;

	if (write_lcr) {
		char ier_save;
		s = sphi();
		ier_save = inb(port+IER);
		if (write_baud) {
			if (speed) {
				short divisor = albaud[speed];

				if (oldSpeed == 0) {
					/* if previous baud rate was zero,
					 * need to go off hook. */
					mcr = inb(port+MCR) | (MC_RTS | MC_DTR);
					outb(port+MCR, mcr);
				}
				T_HAL(4, printf("CH%d speed=%x\n", chan, speed));
				outb(port+LCR, LC_DLAB);
				outb(port+DLL, divisor);
				outb(port+DLH, divisor >> 8);
			} else {
				/* Baud rate of zero means hang up. */
				mcr = inb(port+MCR) & ~(MC_RTS | MC_DTR);
				outb(port+MCR, mcr);
			}
		}
		T_HAL(4, printf("CH%d newlcr=%x\n", chan, newlcr));
		outb(port+LCR, newlcr);
		if (a1->a_ut == US_16550A)
			outb(port+FCR, FC_ENABLE | FC_Rx_RST | FC_Rx_08);
		outb(port+IER, ier_save);
		spl(s);
	}
	if (write_baud)
		asyspr();
}

/*
 * asystart()
 */
static void
asystart(tp)
TTY * tp;
{
	int	chan = (int)tp->t_ddp;
	asy0_t	*a0 = asy0 + chan;
	asy1_t	*a1 = asy1 + chan;
	short	port = a0->a_port;
	int	s;
	int	need_xmit = 1;	/* True if should start sending data now. */
	silo_t	*out_silo = &a1->a_out;
	char	lsr;

	/*
	 * Read line status register AFTER disabling interrupts.
	 */
	s = sphi();
	lsr = inb(port + LSR);

	/*
	 * Process break indication.
	 * NOTE: Break indication cleared when line status register was read.
	 */
	if (lsr & LS_BREAK)
		defer(asybreak, chan);

	/*
	 * If no output data, it may be time to finish closing the port;
	 * but won't need another xmit interrupt.
	 */
	if (out_silo->si_ix == out_silo->si_ox) {
		if (need_wake[chan] & NW_OUTSILO) {
			need_wake[chan] &= ~NW_OUTSILO;
			wakeup((char *)out_silo);
		}
		need_xmit = 0;
	}

	/*
	 * Do nothing if output is stopped.
	 */
	if (tp->t_flags & T_STOP)
		need_xmit = 0;
	if (a1->a_ohlt)
		need_xmit = 0;

	/*
	 * Start data transmission by writing to UART xmit reg.
	 */
	if ((lsr & LS_TxRDY) && need_xmit) {
		int xmit_count;
		xmit_count = (a1->a_ut == US_16550A)?16:1;
		asy_send(out_silo, port+DREG, xmit_count);
	}
	spl(s);
}

/*
 * asypoll()
 */
static int
asypoll(dev, ev, msec)
dev_t dev;
int ev;
int msec;
{
	int	chan = channel(dev);
	asy1_t	*a1 = asy1 + chan;
	TTY	*tp = &a1->a_tty;

	return ttpoll(tp, ev, msec);
}

/*
 * asycycle()
 *
 * Do a wakeup of any sleeping asy's at regular intervals.
 */
static void
asycycle(chan)
int chan;
{
	asy0_t	*a0 = asy0 + chan;
	asy1_t	*a1 = asy1 + chan;
	TTY	*tp = &a1->a_tty;
	short	port = a0->a_port;
	int	s;
	char	msr, mcr;
	silo_t	*out_silo = &a1->a_out;
	silo_t	*in_silo = &a1->a_in;
	int	n, ch;
	int	do_start = 1;
	unsigned char	iir;

	/*
	 * Check Carrier Detect (RLSD).
	 *
	 * Modem status interrupts were not enabled due to 8250 hardware bug.
	 * Enabling modem status and receive interrupts may cause lockup
	 * on older parts.
	 */
	if (tp->t_flags & T_MODC) {

		/*
		 * Get status
		 */
		msr = inb(port+MSR);

		/*
		 * Carrier changed.
		 */
		if ((msr & MS_RLSD) && !(tp->t_flags & T_CARR)) {
			/*
			 * Carrier is on - wakeup open.
			 */
			s = sphi();
			tp->t_flags |= T_CARR;
			spl(s);
			wakeup((char *)(&tp->t_open));
		}

		if (!(msr & MS_RLSD) && (tp->t_flags & T_CARR)) {
			s = sphi();
			RAWIN_FLUSH(in_silo);
			RAWOUT_FLUSH(out_silo);
			tp->t_flags &= ~T_CARR;
			spl(s);
			tthup(tp);
		}
	}

	/*
	 * Empty raw input buffer.
	 *
	 * The line discipline module (tty.c) will set T_ISTOP true when the
	 * tt input queue is nearly full (tp->t_iq.cq_cc >= IHILIM), and make
	 * T_ISTOP false when it's ready for more input.
	 *
	 * When T_ISTOP is true, ttin() simply discards the character passed.
	 */
	if (!(tp->t_flags & T_ISTOP)) {
		while (in_silo->SILO_CHAR_COUNT > 0) {
			s = sphi();
			ttin(tp, in_silo->si_buf[in_silo->si_ox]);
			if (in_silo->si_ox < MAX_SILO_INDEX)
				in_silo->si_ox++;
			else
				in_silo->si_ox = 0;
			in_silo->SILO_CHAR_COUNT--;
			spl(s);
		}
	}

	/*
	 * Hardware flow control.
	 *	Check CTS to see if we need to halt output.
	 *	(MS_INTR should have done this - repeat code here to be sure)
	 *	Check input silo to see if we need to raise RTS.
	 */
	if (tp->t_flags & T_CFLOW) {

		/*
		 * Get status
		 */
		msr = inb(port+MSR);
		s = sphi();
		if (msr & MS_CTS)
			a1->a_ohlt = 0;
		else
			a1->a_ohlt = 1;
		spl(s);
#if	0
	/*
	 * NIGEL: From now on, trace macros need to be given expressions. This
	 * one needs some work to fix...
	 */
T_HAL(4, {static cts = 0; if (!cts && (msr & MS_CTS)) { cts = 1; putchar('[');\
} else if (cts && !(msr & MS_CTS)) { cts = 0; putchar(']'); }});
#endif

		/*
		 * If using hardware flow control, see if we need to drop RTS.
		 */
		if ((tp->t_flags & T_CFLOW)
		&& (in_silo->SILO_CHAR_COUNT > SILO_HIGH_MARK)) {
			s = sphi();
			mcr = inb(port+MCR);
			if (mcr & MC_RTS) {
				outb(port+MCR, mcr & ~MC_RTS);
				T_HAL(4, putchar('-'));
			}
			spl(s);
		}

		/*
		 * If input silo below low mark, assert RTS.
		 */
		if (in_silo->SILO_CHAR_COUNT <= SILO_LOW_MARK) {
			s = sphi();
			mcr = inb(port+MCR);
			if ((mcr & MC_RTS) == 0) {
				outb(port+MCR, mcr | MC_RTS);
				T_HAL(4, putchar('+'));
			}
			spl(s);
		}
	}

	/*
	 * Calculate free output slot count.
	 */
	n  = sizeof(out_silo->si_buf) - 1;
	n += out_silo->si_ox - out_silo->si_ix;
	n %= sizeof(out_silo->si_buf);

	/*
	 * Fill raw output buffer.
	 */
	for (;;) {
		if (--n < 0)
			break;
		s = sphi();
		ch = ttout(tp);
		spl(s);
		if (ch < 0)
			break;

		s = sphi();
		out_silo->si_buf[out_silo->si_ix] = ch;
		if (out_silo->si_ix >= sizeof(out_silo->si_buf) - 1)
			out_silo->si_ix = 0;
		else
			out_silo->si_ix++;
		spl(s);
	}

#ifdef _I386
	/*
	 * if port has an interrupt pending (probably missed an irq)
	 *	the following two loops should not be merged
	 *	- need ALL port irq's inactive at once
	 *	for each port on this irq line (use irq1 for this)
	 *		disable interrupts (clear IER)
	 *	for each port on this irq line
	 *		restore interrupts
	 */
	if (a1->a_has_irq && ((iir=inb(port+IIR)) & 1) == 0) {
		struct irqnode	*ip;
		asy_gp_t	*gp;
		int	s;
		short	p;
		char	c, slot;

		T_HAL(4, printf("CH%d missed iir:x\n", chan, iir));

		do_start = 0;
		s = sphi();
		ip = irq1[a0->a_irqno];
		while(ip) {
			if (ip->func == asyintr) {
				p = ip->arg;
				outb(p + IER, 0);
			} else {
				gp = asy_gp + ip->arg;
				for (slot = 0; slot < MAX_SLOTS; slot++) {
					if ((c=gp->chan_list[slot]) < MAX_ASY){
						p = asy0[c].a_port;
						outb(p + IER, 0);
					}
				}
			}
			ip = ip->next_actv;
		}
		/*
		 * Now, all ports on the offending irq line have irq off.
		 */
		ip = irq1[a0->a_irqno];
		while(ip) {
			if (ip->func == asyintr) {
				p = ip->arg;
				outb(p + IER, IEN);
			} else {
				gp = asy_gp + ip->arg;
				for (slot = 0; slot < MAX_SLOTS; slot++) {
					if ((c=gp->chan_list[slot]) < MAX_ASY){
						p = asy0[c].a_port;
						outb(p + IER, IEN);
					}
				}
			}
			ip = ip->next_actv;
		}
		spl(s);
	}
#endif

	if(do_start)
		ttstart(tp);

	/*
	 * Schedule next cycle.
	 */
	if (a1->a_in_use) {
		timeout(&tp->t_rawtim, HZ/10, asycycle, chan);
	}
}

/*
 * irqdummy()
 *
 * Suppress interrupts that may occur during chip sensing.
 */
static void
irqdummy()
{
	/*
	 * Try to clear all pending interrupts.
	 */
	inb(dummy_port+IIR);
	inb(dummy_port+LSR);
	inb(dummy_port+MSR);
	inb(dummy_port+DREG);
}

/*
 * add_irq()
 *
 * Given channel number, add port info to irq0 list.
 */
static void
add_irq(irq, func, arg)
int irq;
void (*func)();
int arg;
{
	struct irqnode * np;

	/*
	 * Sanity check.
	 */
	if (irq <=0 || irq >= NUM_IRQ || itbl[irq] == 0)
		return;

	if (nextnode < MAX_ASY) {
		np = nodespace + nextnode++;
		np->func = func;
		np->arg = arg;
		np->next = irq0[irq];
		irq0[irq] = np;
	} else {
		printf("asy: too many irq nodes (%d)\n", nextnode);
	}
}

/*
 * Interrupt handlers.
 */
static void i2() { asy_irq(irq1[2]); }
static void i3() { asy_irq(irq1[3]); }
static void i4() { asy_irq(irq1[4]); }
static void i5() { asy_irq(irq1[5]); }
static void i6() { asy_irq(irq1[6]); }
static void i7() { asy_irq(irq1[7]); }
static void i8() { asy_irq(irq1[8]); }
static void i9() { asy_irq(irq1[9]); }
static void i10() { asy_irq(irq1[10]); }
static void i11() { asy_irq(irq1[11]); }
static void i12() { asy_irq(irq1[12]); }
static void i13() { asy_irq(irq1[13]); }
static void i14() { asy_irq(irq1[14]); }
static void i15() { asy_irq(irq1[15]); }

/*
 * asy_irq()
 *
 * Given pointer to node list, service async interrupt.
 */
static void
asy_irq(ip)
struct irqnode * ip;
{
	struct irqnode	*here;
	int		doit;

	do {
		doit = 0;
		here = ip;
		while(here) {
			doit |= (*(here->func))(here->arg);
			here = here->next_actv;
		}
	} while(doit);
}

/*
 * upd_irq1()
 *
 * Given an irq number, rebuild the links for active devices.
 */
static void
upd_irq1(irq)
int irq;
{
	struct irqnode	*np;
	asy1_t	*a1;
	int	chan;
	int	s;

	/*
	 * Sanity check.
	 */
	if (irq <=0 || irq >= NUM_IRQ || itbl[irq] == 0)
		return;

	/*
	 * For each node in the irq0 list
	 *	if node is for irq status port
	 *		for each channel using the status port
	 *			if channel in use, in irq mode
	 *				add node to irq1 list
	 *				skip rest of channels for this node
	 *	else - node is for simple UART
	 *		if channel in use, in irq mode
	 *			add node to irq1 list
	 */
	s = sphi();
	np = irq0[irq];
	irq1[irq] = 0;
	while (np) {
		if (np->func != asyintr) {
			char ix, loop = 1;
			asy_gp_t *gp = asy_gp + np->arg;

			for (ix = 0; ix < MAX_SLOTS && loop; ix++) {
				if ((chan = gp->chan_list[ix]) < MAX_ASY) {
					a1 = asy1 + chan;
					if (a1->a_in_use && a1->a_irq) {
						np->next_actv = irq1[irq];
						irq1[irq] = np;
						loop = 0;
					}
				}
			}
		} else {
			a1 = asy1 + np->arg;
			if (a1->a_in_use && a1->a_irq) {
				np->next_actv = irq1[irq];
				irq1[irq] = np;
			}
		}
		np = np->next;
	}
	spl(s);
}

/*
 * asybreak()
 */
static void
asybreak(chan)
int chan;
{
	int	s;
	asy1_t	*a1 = asy1 + chan;
	silo_t	*out_silo = &a1->a_out;
	silo_t	*in_silo = &a1->a_in;
	TTY	*tp = &a1->a_tty;

	s = sphi();
	RAWIN_FLUSH(in_silo);
	RAWOUT_FLUSH(out_silo);
	spl(s);
	ttsignal(tp, SIGINT);
}

/*
 * asyintr()
 *
 * Handle interrupt for a single channel.
 */
static int
asyintr(chan)
int chan;
{
	asy0_t	*a0 = asy0 + chan;
	asy1_t	*a1 = asy1 + chan;
	TTY	*tp = &a1->a_tty;
	silo_t	*out_silo = &a1->a_out;
	silo_t	*in_silo = &a1->a_in;
	int	c, xmit_count;
	int	ret = 0;
	short	port = a0->a_port;
	unsigned char	msr, lsr;

	if (chan >= MAX_ASY) {
		printf("asy: irq on channel %d\n", chan);
		return 0;
	}

rescan:
	switch (inb(port+IIR) & 0x07) {

	case LS_INTR:
		ret = 1;
		lsr = inb(port + LSR);
		T_HAL(0x800, printf("[%d:L%x]", chan, lsr));
		if (lsr & LS_BREAK)
			defer(asybreak, chan);
		goto rescan;

	case Rx_INTR:
		T_HAL(0x800, printf("[%d:R]", chan));
		ret = 1;
		c = inb(port+DREG);
		if (tp->t_open == 0)
			goto rescan;
		/*
		 * Must recognize XOFF quickly to avoid transmit overrun.
		 * Recognize XON here as well to avoid race conditions.
		 */
		if (_IS_IXON_MODE (tp)) {
			/*
			 * XON.
			 */
#if _I386
			if (_IS_START_CHAR (tp, c) ||
			    (_IS_IXANY_MODE (tp) &&
			     (tp->t_flags & T_STOP) != 0)) {
				tp->t_flags &= ~(T_STOP | T_XSTOP);
				goto rescan;
			}
#else
			if (_IS_START_CHAR (tp, c)) {
				tp->t_flags &= ~T_STOP;
				goto rescan;
			}
#endif

			/*
			 * XOFF.
			 */
			if (_IS_STOP_CHAR (tp, c)) {
				tp->t_flags |= T_STOP;
				goto rescan;
			}
		}

		/*
		 * Save char in raw input buffer.
		 */
		if (in_silo->SILO_CHAR_COUNT < MAX_SILO_CHARS) {
			in_silo->si_buf[in_silo->si_ix] = c;
			if (in_silo->si_ix < MAX_SILO_INDEX)
				in_silo->si_ix++;
			else
				in_silo->si_ix = 0;
			in_silo->SILO_CHAR_COUNT++;
		}

		/*
		 * If using hardware flow control, see if we need to drop RTS.
		 */
		if ((tp->t_flags & T_CFLOW)
		&& (in_silo->SILO_CHAR_COUNT > SILO_HIGH_MARK)) {
			unsigned char mcr = inb(port+MCR);
			if (mcr & MC_RTS) {
				outb(port+MCR, mcr & ~MC_RTS);
			}
		}

		goto rescan;

	case Tx_INTR:
		T_HAL(0x800, printf("[%d:T]", chan));
		ret = 1;
		/*
		 * Do nothing if output is stopped.
		 */
		if (tp->t_flags & T_STOP) {
			goto rescan;
		}
		if (a1->a_ohlt)
			goto rescan;

		/*
		 * Transmit next char in raw output buffer.
		 */
		xmit_count = (a1->a_ut == US_16550A)?16:1;
		asy_send(out_silo, port+DREG, xmit_count);
		goto rescan;

	case MS_INTR:
		ret = 1;
		/*
		 * Get status (and clear interrupt).
		 */
		msr = inb(port+MSR);
		T_HAL(0x800, printf("[%d:M%x]", chan, msr));

		/*
		 * Hardware flow control.
		 *	Check CTS to see if we need to halt output.
		 */
		if (tp->t_flags & T_CFLOW) {
			if (msr & MS_CTS)
				a1->a_ohlt = 0;
			else
				a1->a_ohlt = 1;
		}

		goto rescan;
	default:
		return ret;
	} /* endswitch */
}

/*
 * asyclk()
 *
 * Called every time T0 interrupts.- if it returns 0,
 * the usual system timer interrupt stuff is done.
 * Poll all pollable ports.
 */
static int
asyclk()
{
	static	int count;
	int	ix;

	for (ix = 0; ix < ppnum; ix++)
		asysph(pptbl[ix]);

	count++;
	if (count >= poll_divisor)
		count = 0;
	return count;
}

/*
 * asyspr()
 *
 * asyspr is called when a port is opened or closed or changes speed
 * It sets the polling rate only as fast as needed, and shuts off polling
 * whenever possible.
 * It updates the links in irq1[0], which lists polled-mode ports.
 */
static void
asyspr()
{
	asy0_t	*a0;
	asy1_t	*a1;
	int	chan;
	int	s;
	int	ix, max_rate, port_rate;

	/*
	 * Rebuild table of pollable ports.
	 */
	s = sphi();
	ppnum = 0;
	for (chan = 0; chan < ASY_NUM; chan++) {
		a1 = asy1 + chan;
		if (a1->a_poll)
			pptbl[ppnum++] = chan;
	}
	spl(s);

	/*
	 * If another driver has the polling clock, do nothing.
	 */
	if (poll_owner & ~ POLL_ASY)
		return;

	/*
	 * Find highest valid polling rate in units of HZ/10.
	 * If using FIFO chip, can poll at 1/16 the usual rate.
	 */
	max_rate = 0;
	for (ix = 0; ix < ppnum; ix++) {
		chan = pptbl[ix];
		a0 = asy0 + chan;
		a1 = asy1 + chan;
		port_rate = alp_rate[a0->a_speed];
		if (a1->a_ut == US_16550A) {
			port_rate /= 16;
			if (port_rate % HZ)
				port_rate += HZ - (port_rate % HZ);
		}
		if (max_rate < port_rate)
			max_rate = port_rate;
	}

	/*
	 * if max_rate is not current rate, adjust the system clock
	 */
	if (max_rate != poll_rate) {
		poll_rate = max_rate;
		poll_divisor = poll_rate/HZ;  /* used in asyclk() */
		altclk_out();		/* stop previous polling */
		poll_owner &= ~ POLL_ASY;
		if (poll_rate) {  /* resume polling at new rate if needed */
			poll_owner |= POLL_ASY;
			altclk_in(poll_rate, asyclk);
		}
	}
}

/*
 * asysph()
 *
 * Serial polling handler.
 */
static void
asysph(chan)
int chan;
{
	asy0_t	*a0 = asy0 + chan;
	asy1_t	*a1 = asy1 + chan;
	TTY	*tp = &a1->a_tty;
	silo_t	*out_silo = &a1->a_out;
	silo_t	*in_silo = &a1->a_in;
	int	c, xmit_count;
	short	port = a0->a_port;
	char	lsr;

	/*
	 * Check for received break first.
	 * This status is wiped out on reading the LSR.
	 */
	lsr = inb(port + LSR);
	if (lsr & LS_BREAK)
		defer(asybreak, chan);

	/*
	 * Handle all incoming characters.
	 */
	for (;;) {
		lsr = inb(port + LSR);
		if ((lsr & LS_RxRDY) == 0)
			break;
		c = inb(port+DREG);
		if (tp->t_open == 0)
			continue;
		/*
		 * Must recognize XOFF quickly to avoid transmit overrun.
		 * Recognize XON here as well to avoid race conditions.
		 */
		if (_IS_IXON_MODE (tp)) {
			/*
			 * XOFF.
			 */
			if (_IS_STOP_CHAR (tp, c)) {
				tp->t_flags |= T_STOP;
				continue;
			}

			/*
			 * XON.
			 */
			if (_IS_START_CHAR (tp, c)) {
				tp->t_flags &= ~T_STOP;
				continue;
			}
		}

		/*
		 * Save char in raw input buffer.
		 */
		if (in_silo->SILO_CHAR_COUNT < MAX_SILO_CHARS) {
			in_silo->si_buf[in_silo->si_ix] = c;
			if (in_silo->si_ix < MAX_SILO_INDEX)
				in_silo->si_ix++;
			else
				in_silo->si_ix = 0;
			in_silo->SILO_CHAR_COUNT++;
		}

		/*
		 * If using hardware flow control, see if we need to drop RTS.
		 */
		if ((tp->t_flags & T_CFLOW)
		  && (in_silo->SILO_CHAR_COUNT > SILO_HIGH_MARK)) {
			unsigned char mcr = inb(port+MCR);
			if (mcr & MC_RTS) {
				outb(port+MCR, mcr & ~MC_RTS);
			}
		}
	}

	/*
	 * Handle outgoing characters.
	 * Do nothing if output is stopped.
	 */
	lsr = inb(port + LSR);
	if ((lsr & LS_TxRDY)
	  && !(tp->t_flags & T_STOP)
	  && !(a1->a_ohlt)) {
		/*
		 * Transmit next char in raw output buffer.
		 */
		xmit_count = (a1->a_ut == US_16550A)?16:1;
		asy_send(out_silo, port+DREG, xmit_count);
	}

	/*
	 * Hardware flow control.
	 *	Check CTS to see if we need to halt output.
	 */
	if (tp->t_flags & T_CFLOW) {
		if (inb(port+MSR) & MS_CTS)
			a1->a_ohlt = 0;
		else
			a1->a_ohlt = 1;
	}
}

/*
 * asy_send()
 *
 * Write to xmit data register of the UART.
 * Assume all checking about whether it's time to send has been done already.
 * Called by time-critical IRQ and polling routines!
 *
 * "rawout" is the output silo for the TTY struct supplying data to the port.
 * "dreg" is the i/o address of the UART xmit data register.
 * "xmit_count" is the max number of chars we can write (16 for FIFO parts).
 */
static int
asy_send(rawout, dreg, xmit_count)
register silo_t * rawout;
int dreg, xmit_count;
{
	/*
	 * Transmit next chars in raw output buffer.
	 */
	for (;(rawout->si_ix != rawout->si_ox) && xmit_count; xmit_count--) {
		outb(dreg, rawout->si_buf[rawout->si_ox]);
		/*
		 * Adjust raw output buffer output index.
		 */
		if (++rawout->si_ox >= sizeof(rawout->si_buf))
			rawout->si_ox = 0;
	}
	return xmit_count;
}

/*
 * p1()
 *
 * Interrupt handler for Comtrol-type port groups.
 * Status register has 1 in bit positions for interrupting ports.
 */
static int
p1(g)
int g;
{
	asy_gp_t	*gp = asy_gp + g;
	short		port = gp->stat_port;
	unsigned char	status, index, chan;
	int		safety = LOOP_LIMIT;
	int		ret = 0;

#if 0 /* DEBUG */
static int pxstat[2][8];
	int ci;
	int change_found=0;

	for (ci=0; ci<1; ci++) {
		index = inb(port+ci);
		outb(port+ci, 0);
		if (index != pxstat[g][ci]) {
			if (!change_found) {
				change_found = 1;
				printf("<%d:", g);
			} else
				putchar(' ');
			printf("%x:%x", port+ci, index);
			pxstat[g][ci] = index;
		}
	}
	if (change_found)
		putchar('>');
	for (ci=0; ci<8; ci++)
		asyintr(4+ci);
	putchar('.');
	return 0;
#endif /* DEBUG */

	/*
	 * while any port is active
	 *	call simple interrupt handler for active channel
	 */
	while (status = inb(port)) {
		ret = 1;
		index = 0;
		if (status & 0xf0) {
			status &= 0xf0;
			index +=4;
		} else
			status &= 0x0f;
		if (status & 0xcc) {
			status &= 0xcc;
			index +=2;
		} else
			status &= 0x33;
		if (status & 0xaa)
			index++;
		chan = gp->chan_list[index];
		asyintr(chan);
		if (safety-- == 0) {
			printf("asy: p1 runaway - status %x\n", status);
			break;
		}
	}

	return ret;
}

/*
 * p2()
 *
 * Interrupt handler for Arnet-type port groups.
 * Status register has 0 in bit positions for interrupting ports.
 */
static int
p2(g)
int g;
{
	asy_gp_t	*gp = asy_gp + g;
	short		port = gp->stat_port;
	unsigned char	status, index, chan;
	int		safety = LOOP_LIMIT;
	int		ret = 0;

	/*
	 * while any port is active
	 *	call simple interrupt handler for active channel
	 */
	while (status = ~inb(port)) {
		ret = 1;
		index = 0;
		if (status & 0xf0) {
			status &= 0xf0;
			index +=4;
		} else
			status &= 0x0f;
		if (status & 0xcc) {
			status &= 0xcc;
			index +=2;
		} else
			status &= 0x33;
		if (status & 0xaa)
			index++;
		chan = gp->chan_list[index];
		asyintr(chan);
		if (safety-- == 0) {
			printf("asy: p2 runaway - status %x\n", status);
			break;
		}
	}
	return ret;
}

/*
 * p3()
 *
 * Interrupt handler for GTEK-type port groups.
 */
static int
p3(g)
int g;
{
	asy_gp_t	*gp = asy_gp + g;
	short		port = gp->stat_port;
	unsigned char	index, chan;

	/*
	 * Call simple interrupt handler for active channel.
	 */
	index = inb(port) & 7;
	chan = gp->chan_list[index];
	return asyintr(chan);
}

/*
 * p4()
 *
 * Interrupt handler for DigiBoard-type port groups.
 */
static int
p4(g)
int g;
{
	asy_gp_t	*gp = asy_gp + g;
	short		port = gp->stat_port;
	unsigned char	index, chan;
	int		ret = 0;
	int		safety = LOOP_LIMIT;

	/*
	 * Status register has slot number for active port,
	 * or 0xFF if no port is active.
	 */

	for (;;) {
		index = inb(port);
		if (index == 0xFF)
			break;
		if (safety-- == 0) {
			printf("asy: p4 runaway - status %x\n", index);
			break;
		}
		ret = 1;
		chan = gp->chan_list[index&0xF];
		asyintr(chan);
	}
	return ret;
}

#ifdef TRACER
void
asydump(chan, tag)
int chan;
char *tag;
{
	asy0_t	*a0 = asy0 + chan;
	asy1_t	*a1 = asy1 + chan;
	TTY	*tp = &a1->a_tty;

	printf("ch=%d  %s\n", chan, tag);
	printf("port=%x  irqno=%x  speed=%d  ",
	  a0->a_port, a0->a_irqno, a0->a_speed);
	printf("outs=%x  gp=%d  xcl=%d\n",
	  a0->a_outs, a0->a_asy_gp, a0->a_ixc);
	printf("in_use=%d  lcr=%x  irq=%d  has_irq=%d  ",
	  a1->a_in_use, a1->a_lcr, a1->a_irq, a1->a_has_irq);
	printf("hop=%d  hcl=%d  ", a1->a_hopn, a1->a_hcls);
	printf("opn=%d  ier=%x\n", tp->t_open, inb(a0->a_port+IER));
}
#endif
