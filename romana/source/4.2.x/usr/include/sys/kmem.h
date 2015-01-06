/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1993 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */

#ifndef	__SYS_KMEM_H__
#define	__SYS_KMEM_H__

/*
 * This header deals with kernel memory-management functions defined by the
 * DDI/DKI.
 */

#include <common/ccompat.h>
#include <common/_size.h>
#include <common/_null.h>


/*
 * Flag values for kmem_alloc () and kmem_zalloc (), also used by various
 * lock-allocation functions in <sys/ksynch.h>
 */

enum {
	KM_SLEEP,
	KM_NOSLEEP
};


__EXTERN_C_BEGIN__

__VOID__      *	kmem_alloc	__PROTO ((size_t _size, int _flag));
void		kmem_free	__PROTO ((__VOID__ * _addr, size_t _size));
__VOID__      *	kmem_zalloc	__PROTO ((size_t _size, int _flag));

__EXTERN_C_END__

#endif	/* ! defined (__SYS_KMEM_H__) */
