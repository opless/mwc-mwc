#include "mprec.h"


/*
 *	Mcmp compares the mint's pointed to by "a" and "b".  It returns an
 *	int which is :
 *		greater than zero iff "a" > "b"
 *		less than zero ii "a" < "b"
 *		equal to zero iff "a" == "b".
 */

mcmp(a, b)
register mint *a, *b;
{
	register char *ap, *bp;
	register unsigned count;
	int sign;

	ap = a->val + a->len;
	bp = b->val + b->len;

	/* first see if signs are different */
	sign = *(ap - 1) == NEFL;
	if (sign != (*(bp - 1) == NEFL))
		return (sign ? -1 : 1);

	/* next see if lengths different */
	if (a->len != b->len)
		return (sign ? b->len - a->len : a->len - b->len);

	/* finally see if any of the bytes are different */
	count = a->len;
	while (count-- != 0)
		if (*--ap != *--bp)
			return (*ap - *bp);
	return (0);
}
