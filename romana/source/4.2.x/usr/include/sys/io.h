/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1993 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */

#ifndef	 __SYS_IO_H__
#define	 __SYS_IO_H__

#include <common/feature.h>
#include <kernel/__buf.h>
#include <common/__paddr.h>
#include <common/__caddr.h>
#include <common/__size.h>
#include <common/__off.h>
#include <common/_io.h>

/*
 * Complete the definition of the "IO" type by declaring the structure used to
 * store parameters for I/O.
 */

struct __coherent_io {
	int		io_seg;		/* Space */
	unsigned	io_ioc;		/* Count */
	__off_t		io_seek;	/* Seek posiion */
	union {
		__caddr_t	vbase;		/* Virtual base */
		__paddr_t	pbase;		/* Physical base */
	} io;
	short		io_flag;	/* Flags: 0, IONDLY */
};


/*
 * Types of space I/O operation is being performed from.
 */

#define IOSYS	0			/* System */
#define IOUSR	1			/* User */
#define IOPHY	2			/* Physical */


/*
 * No delay if results are not immediately available. IONDLY is the funky
 * internal name for O_NDELAY and IONONBLOCK is the version for O_NONBLOCK.
 */

#define	IONDLY		8
#define	IONONBLOCK	16


#if	_KERNEL

__EXTERN_C_BEGIN__

void		ioread		__PROTO ((IO * _iop, char * _v, __size_t _n));
void		ioclear		__PROTO ((IO * _iop, __size_t _size));
void		iowrite		__PROTO ((IO * _iop, char * _v, __size_t _n));
int		iogetc		__PROTO ((IO * _iop));
int		ioputc		__PROTO ((unsigned char _c, IO * _iop));
void		ioreq		__PROTO ((__buf_t * _bp, IO * _iop,
					  o_dev_t _dev, int _req, int _f));

void		dmareq		__PROTO ((__buf_t * _bp, IO * _iop,
					  o_dev_t _dev, int _req));

__EXTERN_C_END__

#endif	/* ! _KERNEL */

#endif	/* ! defined (__SYS_IO_H__) */
