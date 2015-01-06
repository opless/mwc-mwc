/*
 * The routines in this file
 * read in expression trees from the intermediate
 * file. They are used by both the modify phase and the
 * code generation phase.
 */
#ifdef   vax
#include "INC$LIB:cc1.h"
#else
#include "cc1.h"
#endif

/*
 * Read in a tree.
 * The tree is built up in the
 * tree buffer.
 * Return a pointer to the top
 * of the tree.
 */
TREE *
treeget()
{

	newtree(sizeof(TREE));
	return (treeget1());
}

/*
 * This recursive routine is
 * the real workhorse of the tree
 * reader.
 * It build the node and calls
 * itself to grab the subtrees of
 * the node.
 */
TREE *
treeget1()
{
	register TREE *tp;
	register op;

	if ((op=iget()) == NIL)
		return (NULL);
	tp = alocnode();
	tp->t_op = op;
	tp->t_type = bget();
	tp->t_size = 0;
	if (tp->t_type == BLK)
		tp->t_size = iget();
	switch (op) {

	case ICON:
		tp->t_ival = iget();
		break;

	case LCON:
		tp->t_lval = lget();
		break;

	case DCON:
		dget(tp->t_dval);
		break;

	case AID:
	case PID:
		tp->t_offs = zget();
		break;

	case LID:
		tp->t_offs = zget();
		tp->t_seg = bget();
		tp->t_label  = iget();
		break;

	case GID:
		tp->t_offs = zget();
		tp->t_seg = bget();
		sget(id, NCSYMB);
		tp->t_sp = gidpool(id);
		break;

	case REG:
		tp->t_reg = iget();
		break;

	case FIELD:
		tp->t_width = bget();
		tp->t_base  = bget();
		tp->t_lp = treeget1();
		break;

	default:
		tp->t_lp = treeget1();
		tp->t_rp = treeget1();
	}
	return (tp);
}

/*
 * Walk a tree, calling the supplied
 * function at each node. The order of the
 * walk is left, right, root.
 */
walk(tp, f)
TREE	*tp;
int	(*f)();
{
	walk1(tp, NULL, f);
}

walk1(tp, ptp, f)
register TREE	*tp;
TREE		*ptp;
int		(*f)();
{
	register TREE	*stp;

	if (!isleaf(tp->t_op)) {
		walk1(tp->t_lp, tp, f);
		if (tp->t_op!=FIELD && (stp=tp->t_rp)!=NULL)
			walk1(stp, tp, f);
	}
	(*f)(tp, ptp);
}
