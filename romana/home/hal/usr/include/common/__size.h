/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1993 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */

#ifndef	__COMMON___SIZE_H__
#define	__COMMON___SIZE_H__

/*
 * This internal header file defines the internal data type "__size_t".
 * This type is equivalent to size_t, but with an internal name to avoid
 * exporting the name into the user's namespace.
 *
 * Under normal circumstances, applications should not need to know what
 * fundamental type an "__size_t" corresponds to.  For efficiency, however,
 * it is sometimes handy to know *at the preprocessor* level, and there is
 * no way to derive this information portably.  So, we also supply that datum
 * here.  Caveat utilitor.
 */

#include <common/feature.h>

#if	__BORLANDC__ || __COHERENT__

typedef	unsigned	__size_t;
#define	___SIZE_T	_UINT

#elif	__GNUDOS__

typedef	unsigned long	__size_t;
#define	___SIZE_T	_ULONG

#else

#error	size_t not known for this system

#endif

#endif	/* ! defined (__COMMON___SIZE_H__) */
