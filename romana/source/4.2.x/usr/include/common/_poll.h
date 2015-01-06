/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1993 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */

#ifndef	__COMMON__POLL_H__
#define	__COMMON__POLL_H__

/*
 * This internal header file defines the internal data type "poll_t".  This
 * data type is not intended for public use, and would not normally be
 * visible; however, to maintain binary and source compatibility with earlier
 * releases of COHERENT, this type is imported into a structure whose
 * definition was known to driver code in earlier releases.
 *
 * Do not rely on the contents or even existence of this header across
 * releases of the COHERENT operating system.
 */

/*
 * Polling is one of the few places using a circular list makes sense.
 * To maintain the circular-list code, we distinguish the list head from
 * the list node and put the first/next pointers in a special node item
 * used by both.
 */

typedef	struct pollnode	poll_t;

struct pollnode {
	poll_t	      *	pn_next;
	poll_t	      *	pn_prev;
};

#endif	/* ! defined (__COMMON__POLL_H__) */
