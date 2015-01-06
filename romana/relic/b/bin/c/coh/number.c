/*	number.c
 *	11/28/83
 *	Usage:  number [ -tn ] [ file ... ]
 *	Reads file ... or standard input,
 *	prints line numbered listing on standard output.
 *	Option -tn expands tabs to every n spaces (default: 8).
 */

#define	VERSION	"V1.4"		/* version number */
#ifndef	UDI
#define	VCHAR	'V'
#else
#define	VCHAR	'v'		/* upper case mapped to lower under UDI */
#endif

#include <stdio.h>

int tab = 8;			/* tab stop spacing */
int line = 0;			/* line number */
int pos = 0;			/* line position */
int last = '\n';		/* last character read */

main (argc, argv)
int argc;
register char **argv;
{
	register int c;

	/* process flags */
	while (*++argv != NULL && **argv == '-') {
		switch (*++*argv) {
			case 't':	tab = atoi (++*argv);
					break;
			case VCHAR:	fprintf (stderr, "number:  %s\n", VERSION);
					break;
			default:	fprintf (stderr, "Usage:  number [ -tn ] [ file ... ]\n");
					exit (1);
					break;
		}
	}

	/* process files */
	do {
		if (*argv != NULL) {
			if (freopen (*argv++, "r", stdin) == NULL ) {
				fprintf (stderr, "number:  cannot open %s\n", *--argv);
				exit (1);
			}
		}
		while ((c = getchar ()) != EOF) put1 (c);
	} while (*argv != NULL);
	exit (0);
}

/* print character c, expanding tabs and numbering after newlines */
put1 (c)
register c;
{
	if (last == '\n') printf ("%4d:   ", ++line);
	last = c;
	switch (c) {
		case '\t':	if (tab != 0) { /* expand tab */
					do putchar (' ');
					while ( ++pos % tab != 0);
					return;
				}
				break;
		case '\n':	pos = 0;
				break;
		case '\b':	--pos;
				break;
		default:	++pos;
				break;
	}
	putchar (c);
}

/* end of number.c */
