/*
 * The code in this file was automatically generated. Do not hand-modify!
 * Generated at Fri Jul 16 16:28:28 1993

 */

#define _KERNEL		1
#define _DDI_DKI	1

#include <kernel/confinfo.h>

/* entry points for "echo" driver */

extern int echodevflag;
DECLARE_STREAMS (echo)


/* entry points for "dump" driver */

extern int dumpdevflag;
DECLARE_MODULE (dump)


/* entry points for "loop" driver */

extern int loopdevflag;
DECLARE_STREAMS (loop)
DECLARE_INIT (loop)


/* entry points for "timeout" facility */

DECLARE_INIT (timeout_)


/* entry points for "streams" facility */



init_t inittab [] = {
	INIT (loop),
	INIT (timeout_)
};

unsigned int ninit = sizeof (inittab) / sizeof (* inittab);

start_t starttab [1];

unsigned int nstart= 0;

exit_t exittab [1];

unsigned int nexit= 0;

halt_t halttab [1];

unsigned int nhalt= 0;

cdevsw_t cdevsw [] = {
	STREAMS_ENTRY (loop),
	STREAMS_ENTRY (echo)
};

unsigned int ncdevsw = sizeof (cdevsw) / sizeof (* cdevsw);

bdevsw_t bdevsw [1];

unsigned int nbdevsw = 0;

modsw_t modsw [] = {
	MODSW_ENTRY (dump)
};

unsigned int nmodsw = sizeof (modsw) /  sizeof (* modsw);

major_t _maxmajor = 34;

major_t _major [] = {
	NODEV, NODEV, NODEV, NODEV, NODEV, NODEV, NODEV, NODEV, 
	NODEV, NODEV, NODEV, NODEV, NODEV, NODEV, NODEV, NODEV, 
	NODEV, NODEV, NODEV, NODEV, NODEV, NODEV, NODEV, NODEV, 
	NODEV, NODEV, NODEV, NODEV, NODEV, NODEV, NODEV, NODEV, 
	0, 1, NODEV
};

minor_t _minor [] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0
};

intmask_t _masktab [] = {
	0x0UL, 0x0UL, 0x0UL, 0x0UL, 
	0x0UL, 0x0UL, 0x0UL, 0x0UL, 
	0xFFFFFFFFUL
};

intr_t inttab [1];
unsigned int nintr = 0;

