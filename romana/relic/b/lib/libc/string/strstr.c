/*
 * strstr.c
 * ANSI 4.11.5.7.
 * Search string for string.
 */

#include <string.h>

char *strstr(s1, s2) char *s1; char *s2;
{
	register char *cp1, *cp2;
	register char c2;

	if (*s2 == '\0')
		return(s1);
	for ( ; *s1; s1++) {
		cp1 = s1;
		cp2 = s2;
		while ((c2 = *cp2++) && *cp1++ == c2)
			;
		if (c2 == '\0')
			return(s1);		/* match */
		else if (*--cp1 == '\0')
			return(NULL);		/* end of s1, failed */
	}
	return (NULL);				/* failed */
}
