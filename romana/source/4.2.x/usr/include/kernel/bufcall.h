/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1993 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */

#ifndef	__KERNEL_BUFCALL_H__
#define	__KERNEL_BUFCALL_H__

/*
 * This header file contains a number of definitions that aim at working around
 * the enormous problems generated by the existence of callback functions
 * as implemented by bufcall(), i.e., without argument checking.  See the
 * definition of the str_event structure in <kernel/strmlib.h> for a more
 * complete discussion of the possible lossage caused by this.
 */

#include <common/ccompat.h>
#include <common/xdebug.h>
#include <sys/stream.h>


/*
 * The most appealing solution to this problem is to define a solution that
 * works with inline functions to get full argument type checking, and then
 * to define an intermediate level for C that simply deals with ensuring that
 * the arguments are passed through the regular bufcall () mechanism without
 * lossage.
 *
 * Because the only genuinely safe way of doing this is via the use of stdargs
 * (which require that the function be declared that way, in ISO C it is
 * distinctly unsafe to cast function pointers such that the argument
 * definitions are different).  Either an underlying implementation uses this
 * form of function call or there must be a forwarding function written that
 * does some form of similar hack to perform what is effectively an unsafe
 * cast.
 *
 * To ease the job of declaring potential fully-type-safe versions of the
 * forwarding function, we provide a template-style macro that users
 * can try and use.
 */

/*
 * If you are using this header with an AT&T STREAMS implementation, either
 * you can directly call bufcall () (if the AT&T header is not in fact
 * prototyped) or you can must bufcall () indirectly through a special
 * function safe_bufcall (). If you are using the MWC-supplied STREAMS, then
 * bufcall () has been implemented and prototyped such that it does not
 * perform any type-checking of the callback or callback-argument arguments.
 *
 * If you are compiling with the AT&T implementation and have determined that
 * it is safe to do so, either define __STDARG_BUFCALL__ below or define it
 * in the compiler command line.
 */

/* #define __STDARG_BUFCALL__ */

/*
 * safe_bufcall () does a bufcall () but effectively overrides any declared
 * prototype for bufcall (). In certain cases the underlying bufcall ()
 * works this way.
 */

#ifdef	__STDARG_BUFCALL__

# define	safe_bufcall(s,p,f,a)		bufcall (s, p, f, a)
# define	safe_esbbcall(p,f,a)		esbbcall (p, f, a)

#else

__EXTERN_C_BEGIN__

toid_t		safe_bufcall	__PROTO ((uint_t _size, int _pri, ...));
toid_t 		safe_esbbcall	__PROTO ((int _pri, ...));

__EXTERN_C_END__

#endif


/*
 * Clients should follow the general form here for declaring wrappers for
 * callbacks that are private to clients (in general, the following types
 * of callbacks are the only ones that would be visible at anything greater
 * than file scope).
 */


#if	__USE_INLINE__ && __USE_PROTO__

# define	__INLINE_BUFCALL(name,type)\
    __LOCAL__ __INLINE__ toid_t __CONCAT (bufcall_, name) \
		(uint_t _size, int _pri, void (* _func) (type), type _arg) { \
      return safe_bufcall (_size, _pri, _func, _arg); \
    }
# define	__INLINE_ESBBCALL(name,type)\
    __LOCAL__ __INLINE__ toid_t __CONCAT (esbbcall_ ,name) \
		(int _pri, void (* _func) (type), type _arg) { \
      return safe_esbbcall (_pri, _func, arg); \
    }

__LOCAL__ __INLINE__
  toid_t bufcall_void (uint_t _size, int _pri, void (* _func) (void)) {
	return safe_bufcall (_size, _pri, _func, 0);
}

__INLINE_BUFCALL (int, int);
__INLINE_BUFCALL (long, long);
__INLINE_BUFCALL (queue, queue_t *);

__LOCAL__ __INLINE__
  toid_t esbbcall_void (int _pri, void (* _func) (void)) {
	return safe_esbbcall (_pri, _func, 0);
}

__INLINE_ESBBCALL (int, int);
__INLINE_ESBBCALL (long, long);
__INLINE_ESBBCALL (queue, queue_t *);

#else	/* if ! __USE_INLINE__ || ! __PROTO__ */

# define 	bufcall_void(s,p,f)		safe_bufcall (s, p, f, 0)
# define	bufcall_int(s,p,f,a)		safe_bufcall (s, p, f,\
							      (int) (a))
# define	bufcall_long(s,p,f,a)		safe_bufcall (s, p, f,\
							      (long) (a))
# define	bufcall_queue(s,p,f,a)		safe_bufcall (s, p, f,\
							      (queue_t *) (a))

# define 	esbbcall_void(p,f)		safe_esbbcall (p, f, 0)
# define	esbbcall_int(p,f,a)		safe_esbbcall (p, f, (int) (a))
# define	esbbcall_long(p,f,a)		safe_esbbcall (p, f, (long) (a))
# define	esbbcall_queue(p,f,a)		safe_esbbcall (p, f, (queue_t *) (a))

#endif	/* ! __USE_INLINE__ || ! __PROTO__ */

#endif	/* ! defined (__KERNEL_BUFCALL_H__) */
