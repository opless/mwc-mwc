/*
 * Copyright (c) 1981 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Modifications copyright (c) 1989 by INETCO Systems, Ltd.
 */

#define uchar unsigned char

#ifndef COHERENT
#ifndef lint
static uchar sccsid[] = "@(#)cr_tty.c	5.4 (Berkeley) 6/30/88";
#endif /* not lint */
#endif /* not COHERENT */

/*
 * Terminal initialization routines.
 *
 */

# include	"curses.ext"

static uchar	*_PC;

uchar *tgoto();

uchar		_tspace[2048];		/* Space for capability strings */

static uchar	*aoftspace;		/* Address of _tspace for relocation */

static int	destcol, destline;

/*
 *	This routine does terminal type initialization routines, and
 * calculation of flags at entry.  It is almost entirely stolen from
 * Bill Joy's ex version 2.6.
 */
extern short	ospeed;

gettmode() {

	if (gtty(_tty_ch, &_tty) < 0)
		return;
	savetty();
	if (stty(_tty_ch, &_tty) < 0)
		_tty.sg_flags = _res_flg;
	ospeed = _tty.sg_ospeed;
	_res_flg = _tty.sg_flags;
	UPPERCASE = (_tty.sg_flags & LCASE) != 0;
	GT = ((_tty.sg_flags & XTABS) == 0);
	NONL = ((_tty.sg_flags & CRMOD) == 0);
	_tty.sg_flags &= ~XTABS;
	stty(_tty_ch, &_tty);
# ifdef DEBUG
	fprintf(outf, "GETTMODE: UPPERCASE = %s\n", UPPERCASE ? "TRUE":"FALSE");
	fprintf(outf, "GETTMODE: GT = %s\n", GT ? "TRUE" : "FALSE");
	fprintf(outf, "GETTMODE: NONL = %s\n", NONL ? "TRUE" : "FALSE");
	fprintf(outf, "GETTMODE: ospeed = %d\n", ospeed);
# endif
}

setterm(type)
reg uchar *type;
{

	reg int		unknown;
	static uchar	genbuf[1024];
# ifdef TIOCGWINSZ
	struct winsize win;
# endif

# ifdef DEBUG
	fprintf(outf, "SETTERM(\"%s\")\n", type);
	fprintf(outf, "SETTERM: LINES = %d, COLS = %d\n", LINES, COLS);
# endif
	if (type[0] == '\0')
		type = "xx";
	unknown = FALSE;
	if (tgetent(genbuf, type) != 1) {
		unknown++;
		strcpy(genbuf, "xx|dumb:");
	}
# ifdef DEBUG
	fprintf(outf, "SETTERM: tty = %s\n", type);
# endif
# ifdef TIOCGWINSZ
	if (ioctl(_tty_ch, TIOCGWINSZ, &win) >= 0) {
		if (LINES == 0)
			LINES = win.ws_row;
		if (COLS == 0)
			COLS = win.ws_col;
	}
# endif

	if (LINES == 0)
		LINES = tgetnum("li");
	if (LINES <= 5)
		LINES = 24;

	if (COLS == 0)
		COLS = tgetnum("co");
	if (COLS <= 4)
		COLS = 80;

# ifdef DEBUG
	fprintf(outf, "SETTERM: LINES = %d, COLS = %d\n", LINES, COLS);
# endif
	aoftspace = _tspace;

	/*
	 * Extract boolean capabilities.
	 */
	{
		register tflgent_t * fp;

		for ( fp = tflgmap; fp->bp != NULL; fp++ ) {
			*(fp->bp) = tgetflag(fp->id);
#ifdef DEBUG
			fprintf( outf, "%2.2s = %s", fp->id,
				*(fp->bp) ? "TRUE" : "FALSE" );
#endif
		}
	}

	/*
	 * Extract string capabilities.
	 */
	{
		register tstrent_t *sp;
		extern uchar *tgetstr();

		for ( sp = tstrmap; sp->cpp != NULL; sp++ ) {
			*(sp->cpp) = tgetstr(sp->id, &aoftspace);
#ifdef DEBUG
			{
				fprintf(outf, "%2.2s = %s", sp->id,
				*(sp->cpp) == NULL ? "NULL\n" : "\"");
				if (*(sp->cpp) != NULL) {
					uchar * cp;
					for (cp = *(sp->cpp); *cp; cp++)
						fprintf(outf,"%s",unctrl(*cp));
					fprintf(outf, "\"\n");
				}
			}
#endif
		}
	}

	/*
	 * Standout not erased by writing over it - disable standout mode.
	 */
	if (XS)
		SO = SE = NULL;

	/*
	 * Entering/exiting standout mode consumes display memory.
	 * Disable standout mode.
	 */
	if ( (magic_cookie_glitch = tgetnum( "sg" )) > 0 )
		SO = NULL;

	/*
	 * Entering/exiting underline mode consumes display memory.
	 * Disable underline mode.
	 */
	if (tgetnum("ug") > 0)
		US = NULL;

	/*
	 * Standout mode not provided, but underline mode is.
	 * Map standout mode into underline mode.
	 */
	if (!SO && US) {
		SO = US;
		SE = UE;
	}

	/*
	 * Handle funny termcap capabilities
	 */
	if (CS && SC && RC) AL=DL="";
	if (AL_PARM && AL==NULL) AL="";
	if (DL_PARM && DL==NULL) DL="";
	if (IC && IM==NULL) IM="";
	if (IC && EI==NULL) EI="";
	if (!GT) BT=NULL;	/* If we can't tab, we can't backtab either */

	if (tgoto(CM, destcol, destline)[0] == 'O')
		CA = FALSE, CM = 0;
	else
		CA = TRUE;

	PC = _PC ? _PC[0] : FALSE;
	aoftspace = _tspace;
	strncpy(ttytype, longname(genbuf, type), sizeof(ttytype) - 1);
	ttytype[sizeof(ttytype) - 1] = '\0';
	if (unknown)
		return ERR;
	return OK;
}

/*
 * return a capability from termcap
 */
uchar *
getcap(name)
uchar *name;
{
	uchar *tgetstr();

	return tgetstr(name, &aoftspace);
}
