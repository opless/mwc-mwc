/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1993 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */

#ifndef __SYS_VT_H__
#define __SYS_VT_H__

/*
 * Virtual console support.
 */

#if	! _KERNEL
# error	You must be compiling the kernel to use this header.
#endif


/*
 * Virtual terminal structure VTDATA
 */

typedef	struct	{
	unsigned	int	off;
	unsigned    /*short*/	seg;
} faddr_t;

typedef struct _VTDATA {
	/*
	 * Elements dealing with the graphic adapter 
	 * - equivalent to the assembler definitions above.
	 */
	void	(*vmm_func)();
	unsigned vmm_port,	vmm_seg,	vmm_off;
	unsigned vmm_rowl,	vmm_col,	vmm_pos,	vmm_attr;
	unsigned char vmm_args[8];
	unsigned vmm_nargs;
	unsigned vmm_brow,	vmm_erow,	vmm_lrow,	vmm_srow;
	unsigned vmm_scol,	vmm_ibrow,	vmm_ierow;
	unsigned vmm_invis;
	unsigned vmm_slow,	vmm_wrap,	vmm_oattr,	vmm_font;
	unsigned vmm_esc,	vmm_visible;
	unsigned vmm_mseg,	vmm_moff,	vmm_vseg,	vmm_voff;

	/*
	 * Elements dealing with both the graphic adaptor & keyboard 
	 * - equivalent to the assembler definitions above.
	 */
	dev_t		vt_ind;
	KBTBL		vt_kb[MAX_KEYS];
	char		*vt_buffer;

	/*
	 * Elements dealing with the keyboard 
	 *
	 * The keyboard mapping table is too large to fit into kernel data space,
	 * so we need to allocate a segment to it.
	 * The function keys tend to be small and tend to change substantially
	 * more often than the mapping table, so we keep them in the kernel data space.
	 */

	int  		vnkb_lastc;		/* state info */
	unsigned	vnkb_shift;		/* Overall shift state */
	unsigned	char **vnkb_funkeyp; 	/* Pointer to array of func. key ptrs */
	FNKEY		*vnkb_fnkeys;		/* Pointer to structure of values */
	unsigned	vnkb_fklength;		/* length of k_fnval field in fnkeys */
	unsigned	vnkb_sh_index;		/* shift/lock state index */
	char		vnkb_fk_loaded;		/* true == function keys resident */
}	VTDATA;

typedef struct _HWentry {
	unsigned char	count, found;
	short	port;
	faddr_t	vidmemory;
	struct	{
		unsigned char	row;
		unsigned char	col;
#define	TEXTBLOCK 4000	/* 25 x 80 x 2 */
	} screen;
	short	start;
} HWentry;

#ifndef	KB_MAJOR
#include <sys/devices.h>		/* KB_MAJOR defined in here */
#endif

#ifndef	VT_MAJOR
#define	VT_MAJOR	KB_MAJOR
#endif

#define	VT_PHYSICAL	0x40
#define	VT_HW		0x30
#define	VT_HW_COLOR	0x00
#define	VT_HW_MONO	0x10

#define	VT_INDEX(dev)	(dev&0x3F)

#define	NO_DEVICE	0xFF	/* to signify that the minor does not exist */

#define	PRINTV	if(vt_verbose)printf
#define	PUTC	if(vt_verbose)putc


/*
 * patchable params for non-standard keyboards
 */

extern	int	KBDATA;			/* Keyboard data */
extern	int	KBCTRL;			/* Keyboard control */
extern	int	KBSTS_CMD;		/* Keyboard status/command */
extern	int	KBFLAG;			/* Keyboard reset flag */
extern	int	KBBOOT;			/* 0: disallow reboot from keyboard */
extern	int	KBTIMEOUT;		/* shouldn't need this much */
extern	int	KBCMDBYTE;		/* no translation */

/*
 * globals
 */
extern	int		vtmax, vtcount, vtactive;
extern	int		vt_verbose, vt_opened, vtloaded;
extern	VTDATA		*vtconsole, **vtdata;
extern	char		kb_table_loaded;
extern	HWentry		*vtHWtable[];

#endif	/* ! defined (__SYS_VT_H__) */
