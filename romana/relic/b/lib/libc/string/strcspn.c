/*
 * strcspn.c
 * ANSI 4.11.5.3.
 * Search string for characters not in string.
 */

#include <string.h>

size_t *strcspn(s1, s2) char *s1; char *s2;
{
	register char *cp1, *cp2;
	register char c1, c2;

	for (cp1 = s1; c1 = *cp1; cp1++)
		for (cp2 = s2; c2 = *cp2; cp2++)
			if (c1 == c2)
				return((size_t)(cp1 - s1));
	return((size_t)(cp1 - s1));
}
