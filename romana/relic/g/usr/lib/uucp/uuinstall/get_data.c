/* get_data.c: Given a window, coordinates and a required flag, this
 *		function will move the cursor to the given coordinates,
 *		and accept keyboard input up to a limit also passed to this
 *		function. The data will be echoed to the screen as it is
 *		entered and the function will return a pointer to the
 *		string enetered when done.
 */

#include "uuinstall.h"

char * get_data(win, row, col, len, rflag, sflag)
WINDOW *win;
int row, col, len, rflag, sflag;

/* rflag is used to tell us if a field MUST be filled in. If set, when the
 * user tries to press return as the first char, it will be rejected.
 *
 * sflag has a couple of meanings. In some entries, no spaces are permitted,
 * but we assume that the user really wants an underscore. sflag is 1 if
 * this is the case.
 * 
 * In some cases, a space is represented by a \s. If sflag is 2, we will
 * convert a space to a \s.
 */

{

	int x;
	char a[80];
	char b;

	wmove(win, row, col);		/* position our cursor */
	wrefresh(win);
	
	x = 0;
	while (x < len){
		b = wgetch(win);

		/* convert space to underscore if necessary */
		if((sflag == 1) && (b == ' '))
			b = '_';

		if((sflag ==2) && (b == ' ')){
			mvwaddstr(win,row,col + x,"\\s");
			a[x] = '\\';
			a[x+1] = 's';
			x+=2;
			wrefresh(win);
			continue;
		}
			
		/* test for no data entered in field */
		if( (rflag && (x == 0)) && ((b == 10) || (b == 13)))
			continue;

		/* if return was pressed, terminate loop */
		if((b == 10) || (b == 13))
			break;

		/* handle backspace */
		if( (b == 8) && (x > 0)){
			--x;
			wmove(win,row,col+x);
			wstandout(win);
			waddch(win,' ');
			wmove(win,row,col+x);
			wstandend(win);
			wrefresh(win);
			a[x] = '\0';
			continue;
		}

		wmove(win,row,col + x);
		waddch(win,b);
		a[x] = b;
		++x;
		wrefresh(win);

	}
	a[x] = '\0';
	wstandout(win);
	mvwaddstr(win,row,col,a);
	wstandend(win);
	wrefresh(win);

	return(a);
}
