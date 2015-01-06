/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1993 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */

#ifndef	__COMMON__NULL_H__
#define	__COMMON__NULL_H__

/*
 * This internal header factors out the definition of NULL from all other
 * aspects of the STREAMS and device systems.  To create a reasonable
 * facsimile of the STREAMS and DDI/DDK programming environments, many of
 * the header files defined by those documents in the System V environment
 * have been defined.  From the POSIX and ISO C standards documents, we learn
 * that <sys/types.h> is the most likely candidate, but that definitions also
 * occur in <stdlib.h>, <stdio.h>, and many other places.
 */

/*
 * For Borland C++, the use of memory models requires that we define NULL
 * appropriately.  There is a special Borland header for this, however.
 */

#ifndef	NULL

# if	__BORLANDC__

#  include <_null.h>

# else

#  define  NULL	0

# endif

#endif

#endif	/* ! defined (__COMMON__NULL_H__) */
