/*
 * The routine in this file handles the insertion of LEAF nodes.
 */
#ifdef   vax
#include "INC$LIB:cc1.h"
#else
#include "cc1.h"
#endif

/*
 * This function inserts LEAF tree nodes at the places
 * where the value of a subexpression must be computed.
 * It must be done in a pass by itself because the
 * fancy folder rearranges expressions.
 */
TREE *
modleaf(tp, ac)
register TREE *tp;
{
	register ldown, rdown;
	TREE *tp1, *tp2, *tp3;
	int c, ldentry, op, psf;
	FLAG flag;

	if ((c=ac)==MINIT || c==MRETURN || c==MSWITCH)
		c = MRVALUE;
	op = tp->t_op;
	if ((op==EQ || op==NE) && isnval(tp->t_rp, 0)) {
		tp1 = tp->t_lp;
		while (tp1->t_op == LEAF)
			tp1 = tp1->t_lp;
		tp->t_lp = modleaf(tp1, MFLOW);
		return (tp);
	}
	if (!isleaf(op)) {
		ldentry = ldtab[op-MIOBASE];
		if ((ldown=getli(ldentry)) == MPASSED)
			ldown = c;
		else if (ldown == MHARD)
			ldown = getldown(tp, c);
		if ((rdown=getri(ldentry)) == MPASSED) {
			rdown = c;
			if ((op==QUEST || op==COMMA)
			 && (rdown==MLADDR || rdown==MRADDR))
				rdown = MRVALUE;
		} else if (rdown == MHARD)
			rdown = getrdown(tp, c);
		tp1 = tp->t_lp;
		while (tp1->t_op == LEAF)
			tp1 = tp1->t_lp;
		tp->t_lp = modleaf(tp1, ldown);
		if (op!=FIELD && (tp1=tp->t_rp)!=NULL) {
			while (tp1->t_op == LEAF)
				tp1 = tp1->t_lp;
			tp->t_rp = modleaf(tp1, rdown);
		}
	}
	if (ac!=MINIT && tp->t_op!=LEAF) {
		flag = tp->t_flag;
		if ((flag&T_LEAF) != 0) {
			if ((flag&T_NLEAF) == 0) {
				tp2 = findoffs(tp);
				psf = 0;	/* Pointer seen flag */
				tp1 = tp;
				for (;;) {
					tp3 = tp1->t_rp;
					if (tp3 != NULL
					 && ispoint(tp3->t_type)) 
						psf = 1;
					tp3 = tp1->t_lp;
					if (tp3 == tp2)
						break;
					tp1 = tp3;
				}
				flag = tp2->t_flag;
				if ((flag&T_LEAF)!=0 && tp2->t_op!=LEAF) {
					tp2 = leafnode(tp2);
					amd(tp2);
					tp1->t_lp = tp2;
				}
				if ((tp2->t_op==LEAF || tp2->t_op==CONVERT
				||  tp2->t_op==CAST)
				&&  psf == 0)
					setofstype(tp2);
			}
			if (ac!=MLADDR && ac!=MRADDR && tp->t_op!=LEAF) {
				tp = leafnode(tp);
				if (ac == MFNARG)
					fixtoptype(tp);
				amd(tp);
			}
		}
	}
	return (tp);
}
