/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1993 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */

#ifndef	__COMMON___TYPES_H__
#define	__COMMON___TYPES_H__

/*
 * This internal header defines internal versions of the DDI/DKI data types
 * 'uchar_t', 'ushort_t', 'uint_t', and 'ulong_t'.
 *
 * Several headers define types that we want to be compatible with the DDI/DKI
 * definitions, but we do not want them always to pull in the entire
 * <sys/types.h> header file.
 */

typedef	unsigned char	__uchar_t;
typedef	unsigned short	__ushort_t;
typedef	unsigned int	__uint_t;
typedef	unsigned long	__ulong_t;

#endif	/* ! defined (__COMMON___TYPES_H__) */

