/*
 * C compiler.
 * Final symbol table manager.
 */
#ifdef   vax
#include "INC$LIB:cc2.h"
#else
#include "cc2.h"
#endif

/*
 * Lookup a global (or static external) identifier in the symbol table.
 * Return a pointer to the symbol union.
 * If not there and the 'flag' is true,
 * define the symbol at the current location.
 */
SYM *
glookup(id, def)
char	*id;
{
	register SYM	*sp;
	register int	h;

	sp = hash2[h = symbucket(id)];
	while (sp != NULL) {
		if ((sp->s_flag&S_LABNO)==0 && strcmp(id, sp->s_id)==0) {
			if (def != 0) {
				if ((sp->s_flag&S_DEF) != 0)
					cbotch("global symbol \"%s\" redefined", sp->s_id);
				sp->s_flag |= S_DEF;
			}
			return (sp);
		}
		sp = sp->s_fp;
	}
	if ((sp = (SYM *)malloc(sizeof(SYM)+strlen(id)+1)) == NULL)
		cnomem("glookup");
	sp->s_fp = hash2[h];
	hash2[h] = sp;
	sp->s_flag = 0;
	sp->s_seg = SANY;
	sp->s_value = 0;
	sp->s_data = 0;
	sp->s_type = 0;
	if (def != 0)
		sp->s_flag = S_DEF;
	strcpy(sp->s_id, id);
	return (sp);
}

/*
 * Lookup a local symbol.
 * If the symbol is not there and the 'flag' is true,
 * then define the symbol at the current location.
 * Return a pointer to the symbol union.
 */
SYM *
llookup(labno, def)
{
	register SYM	*sp;
	register int	h;

	sp = hash2[h = labno&SHMASK];
	while (sp != NULL) {
		if ((sp->s_flag&S_LABNO)!=0 && labno==sp->s_labno) {
			if (def != 0) {
				if ((sp->s_flag&S_DEF) != 0)
					cbotch("local symbol \"L%d\" redefined", labno);
				sp->s_flag |= S_DEF;
			}
			return (sp);
		}
		sp = sp->s_fp;
	}
	if ((sp = (SYM *) malloc(sizeof(SYM))) == NULL)
		cnomem("llookup");
	sp->s_fp = hash2[h];
	hash2[h] = sp;
	sp->s_flag = S_LABNO;
	sp->s_seg = SANY;
	sp->s_value = 0;
	if (def != 0)
		sp->s_flag = S_LABNO|S_DEF;
	sp->s_labno = labno;
	return (sp);
}

/*
 * Given an ASCII global or static external symbol name,
 * figure out what hash bucket it goes in.
 */
static
symbucket(p)
register char	*p;
{
	register int	c, h;

	h = 0;
	while ((c = *p++) != 0)
		h += c;
	return (h&SHMASK);
}

#if OVERLAID
/*
 * Run through the hash table,
 * releasing all of the symbol table nodes and
 * setting all of the hash table bases to NULL pointers.
 * This frees up the space used by the
 * optimizer phase if overlaid, and all makes sure
 * that the hash table is clear if this is the
 * second file in a monolithic compilation.
 */
freesym2()
{
	register SYM	*sp1;
	register SYM	*sp2;
	register int	i;

	for (i=0; i<NSHASH; ++i) {
		sp1 = hash2[i];
		while (sp1 != NULL) {
			sp2 = sp1->s_fp;
			free((char *) sp1);
			sp1 = sp2;
		}
		hash2[i] = NULL;
	}
}
#endif
