/*
 * replace(s1, pat, s3, all, matcher)
 * find pat in s1 and replace it with s3
 * return the result in a new malloced string.
 * or NULL if s1 not in pat. Set all to 1 to
 * replace all occurances 0 to replace the first.
 */
#include "local_misc.h"

char *
replace(s1, pat, s3, all, matcher)
char *s1, *pat, *s3;
int all;
char * (*matcher)();
{
	int i;
	char *w1, *w2, *w3, *w;

	if (NULL == (w2 = (*matcher)(s1, pat, &w3)))
		return (NULL);
	w = w1 = alloc(strlen(s1) + 1 + (i=strlen(s3)) + (int)ptrdiff(w2, w3));
	for (; s1 != w2;)
		*w1++ = *s1++;
	strcpy(w1, s3);
	if (!all || (NULL == (w2 = replace(w3, pat, s3, all, matcher)))) {
		strcpy(w1 + i, w3);
		return (w);
	}
	strcpy((w3 = alloc(strlen(w2) + 1 + (i = strlen(w)))), w);
	free(w);
	strcpy(w3 + i, w2);
	free(w2);
	return (w3);
}
#ifdef TEST
extern char *strstr();

/*
 * function to find by string
 */
char *
strfind(s1, s2, fin)
char *s1, *s2, **fin;
{
	char *w1;

	if (NULL == (w1 = strstr(s1, s2)))
		return (NULL);
	*fin = w1 + strlen(s2);
	return (w1);
}

main()
{
	if (yn("Match by pattern "))
		for (;;) {
			char s1[80], pat[80], s3[80], *s;
			int all;

			ask(s1, "S1 ");
			if (!strcmp(s1, "quit"))
				exit(0);
			ask(pat, "pat");
			ask(s3, "S3 ");
			all = yn("all");
			if (NULL == (s = replace(s1, pat, s3, all, match)))
				printf("Not found\n");
			else {
				printf("%s\n", s);
				free(s);
			}
		}
	else
		for (;;) {
			char s1[80], s2[80], s3[80], *s;
			int all;

			ask(s1, "S1");
			if (!strcmp(s1, "quit"))
				exit(0);
			ask(s2, "S2");
			ask(s3, "S3");
			all = yn("all");
			if (NULL == (s = replace(s1, s2, s3, all, strfind)))
				printf("Not found\n");
			else {
				printf("%s\n", s);
				free(s);
			}
		}
}
#endif
