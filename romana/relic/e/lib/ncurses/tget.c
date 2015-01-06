#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define	bool	char
#define	TRUE	1
#define	FALSE	0

extern char    buffer[];

/*
 * scan:
 *	scan for character in string, return position of character.
 *	similar to index/strchr, except that if char not found, returns
 *	pointer to null at end of string, instead of a null pointer.
 */

char *
scan (s, c)
register char	*s;
register char	c;
{
    while ((*s) && (*s != c))
	s++;
    return (s);
}


/* findcap:
 * 	returns pointer to just after capname (trailing ':' for flags,
 *	'#' for nums, '=' for strs, '@' for disabled stuff) or to null
 *	 after termcap if not found.
 */

char *
findcap (capname)
char	*capname;
{
    register char	*bptr = buffer;
    register bool	found = FALSE;
    char		cap[3];

    cap[2] = '\0';
    while ((!found) && (*bptr)) {
	bptr = scan (bptr, ':');
	if (*bptr) {
	    cap[0] = *(bptr + 1);
	    cap[1] = *(bptr + 2);
	    if (strcmp (cap,capname) == 0) {
		found = TRUE;
		bptr += 3;	/* skip colon and capname */
	    }
	    else
		bptr++;		/* skip colon */
	}
    }
    return (bptr);
}


/*
 * tgetname:
 *	store name of termcap entry in name
 */

tgetname (name)
char	*name;
{
    char	*bptr;

    bptr = buffer;

    while ((*bptr) && (*bptr != ':'))
	*(name++) = *(bptr++);
    *(name) = '\0';
}


/*
 * tgetflag:
 *	return 1 if capname present, 0 otherwise, -1 if '@'ed.
 */

int
tgetflag (capname)
char	*capname;
{
    char	*c;

    c = findcap (capname);
    if (*c == '\0')
	return (0);
    else if (*c == '@')
	return (-1);
    else
	return (1);
}


/*
 * tgetnum:
 *	return value of capname, -1 if not present, -2 if '@'ed.
 */

int
tgetnum (capname)
char	*capname;
{
    char	*c;
    int		val;

    c = findcap (capname);
    if (*c == '@')
	return (-2);
    else if (*c != '\0') {
	c++;	/* skip '#' */
	val = 0;
	while (isdigit (*c)) {
	    val = (val * 10) + (*c - '0');
	    c++;
	}
	return (val);
    }
    else
	return (-1);
}


/*
 * tgetstr:
 *	store binary value of capname into value, store null if
 *	not present, store "@" if '@'ed.
 */

tgetstr (capname, value)
char		*capname;
register char	*value;
{
    register char	*c;

    c = findcap (capname);
    if (*c == '@')
	strcpy (value, "@");
    else if (*c != '\0') {
	c++;	/* skip '=' */
	while ((*c) && (*c != ':')) {
	    if (*c == '^') {
		c++;
		if (islower (*c)) 
		    *(value++) = toupper(*(c++)) - '@';	/* ascii dependent */
		else
		    *(value++) = *(c++) - '@';	/* ascii dependent */
	    }
	    else if (*c == '\\') {
		c++;
		switch (*c) {
		    case 'E':
			*(value++) = '\033';
			c++;
			break;
		    case 'n':
			*(value++) = '\n';
			c++;
			break;
		    case 'r':
			*(value++) = '\r';
			c++;
			break;
		    case 't':
			*(value++) = '\t';
			c++;
			break;
		    case 'b':
			*(value++) = '\b';
			c++;
			break;
		    case 'f':
			*(value++) = '\f';
			c++;
			break;
		    default:
			if (isdigit (*c)) {	/* octal value */
			    *value = '\0';
			    while (isdigit (*c)) {
				*value = ((*value) * 8) + ((*c) - '0');
				c++;
			    }
			    value++;
			}
			else
			    *(value++) = *(c++);
			break;
		}
	    }
	    else
		*(value++) = *(c++);
	}
	*value = '\0';
    }
    else
	*value = '\0';
}
