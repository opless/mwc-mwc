/*
 * db/i386/mdbdefs.h
 * A debugger.
 * i386-specific header.
 * i386/mdb.h contains definitions used by machine-independent code.
 * i386/mdbdefs.h defines externals referenced by machine-independent code.
 * i386/i386db.h contains definitions used only by machine-dependent code.
 */

/*
 * Functions defined by machine-dependent routines.
 * i386/i386db.h defines additional routines
 * which are referenced only by machine-dependent code.
 */
/* i386db1.c */
extern	int	bpt_setr	__((char *cmd));
extern	int	bpt_test	__((ADDR_T pc));
extern	void	display_reg	__((int flag));
extern	int	find_seg	__((ADDR_T addr));
extern	char	*get_format	__((int t1, int t2));
extern	ADDR_T	get_fp		__((void));
extern	ADDR_T	get_pc		__((void));
extern	int	get_regs	__((int flag));
extern	int	get_sig		__((void));
extern	void	init_mch	__((void));
extern	int	is_call		__((void));
extern	int	is_reg		__((SYM *sp));
extern	int	is_syscall	__((int *flagp));
extern	void	print_const	__((char *buf, ADDR_T addr));
extern	int	rest_syscall	__((int *flagp));
extern	void	setcoffseg	__((void));
extern	void	setdump		__((char *name));
extern	void	setkmem		__((char *name));
extern	void	setloutseg	__((void));
extern	void	set_pc		__((ADDR_T ip));
extern	void	set_ra_bpt	__((void));
extern	void	set_sig		__((int sig));
extern	void	traceback	__((int levels));
/* i386db2.c */
extern	char	*disassemble	__((char *dest, int s));
/* i386db4.s */
extern	double	_get_fp_reg	__((struct _fpreg *fpregp));

/*
 * Variables defined by machine-dependent code.
 * i386/i386db.h defines additional globals
 * which are referenced only by machine-dependent code.
 */
extern	BIN	bin;

/* end of db/i386/mdbdefs.h */
