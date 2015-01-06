/*
 * COCOA structure definitions
 *
 * This parser generator is dedicated to Reinaldo Braga.
 * May he live a hundred years
 */
#include <stdio.h>
#include <sys/mdata.h>

#define FATAL 01		/* flag for yyerror */
#define SKIP 02			/* ditto */
#define NLNO 04			/* no line number on error line */
#define WARNING 010
#define TTERM 0			/* "genre" for token */
#define TNTERM 1			/* non terminal */
#define TTYPE 2			/* <type> */
#define MAXT 3			/* number of different genres */
				/* if maxterm is > 127 change LSETSIZE  */
#define LSETSIZE 20		/* chars in ws ::= MAXTERM/8 + 1 */

		/* defaults -- can be changed with run time options */
#define MAXPROD 175		/* maximum number of productions */
#define MAXTERM 150		/* maximum number of different terminals */
#define MAXNTERM 100		/* maximum number of non terminal symbols */
#define MAXSTATE 300		/* max # of states */
#define MAXTYPE 10		/* for the union of YYSTYPE */

		/* compiled in sizes -- can be increased without problem */
#define MAXPRODL 20		/* maximum number of symbols in any prodn */
#define MAXITEM 160		/* maximum number of items in any state */
#define MAXREDS 60		/* maximum number of reductions per state */

		/* keyword codings */
#define START 1	
		/* %token .. %nonassoc must be contiguously coded */
#define TOKEN 2
#define LEFT 3
#define RIGHT 4
#define NONASSOC 5
#define UNION 6
#define PREC 7
#define TYPE 8
#define SEMICOLON 9
#define VBAR 10		/* production separators */
#define LBRAC 11	/* beginning of action */
#define T_IDENT 12
#define C_IDENT 13
#define MARK 14
#define IDENT 15
#define COMMA 16
#define INTEGER 17

	/* precedence associativities */
#define UNASSOC 0
#define LASSOC 1
#define RASSOC 2
#define BASSOC 3		/* "binary" associativity - %nonassoc */

		/* macros */
		/* character manipulation */

		/* run of the mill manifests */
#define UINT -1			/* "unknown" type -- integer */
#define UNKNOWN -1
#define SYMSIZE 32
#define DERIV 01		/* non terminal derives the empty string */
#define CPRES 02		/* temp flag for closure */
#define nterm gtab[TTERM].g_ordno
#define nnonterm gtab[TNTERM].g_ordno
#define ntype gtab[TTYPE].g_ordno
#define maxterm gtab[TTERM].g_maxord
#define maxnterm gtab[TNTERM].g_maxord
#define maxtype gtab[TTYPE].g_maxord
#define maxsym (maxterm+maxnterm+maxtype)
#define NTBASE 010000		/* base number for non terminal allocation */
#define EOFNO 0
#define ERRNO 1
#define bounded(v,l,name) if( v>=l ) yyerror(FATAL, bounderr, name, l)
#define MAXSYM 353



struct sym
{
	char	s_name[SYMSIZE];
	int	s_no;	/* ordinal number of symbol */
	int	s_val;	/* external value, for non terminal only */
	char	s_prc, s_ass; /* precedence, associativity */
	int	s_type;
	char	s_genre; /* "kind" of symbol -- terminal, nonterminal, type */
		/* remaining flags are only used for non-terminals */

	char	s_flags;	/* for closure and lookahead computations */
	int	s_nprods;	/* number of productions having nt as lhs */
	struct prod **s_prods;
	int	s_nstates;	/* sfor nt A, # of states with item A->. ai* */
	int	*s_states;	/* list */
};

struct sitem
{
	int	i_nitems;	/* number of items in set */
	int	*i_items[];
} ;

struct state
{
	int	s_tgo;
	struct	tgo *s_tgos;
	int	s_ntgo;
	struct	ntgo *s_ntgos;
	int	s_nred;
	struct redn *s_reds;
} ;

struct prod
{
	int	p_prodno;	/* index in prdptr */
	char	p_prc, p_ass;	/* precedence, associativity */
	int	p_left;		/* -(ordinal number for lhs) */
	int	p_right[];	/* ordinal numbers for rhs w. -1 end marker */
};
		/* kludgy accessing macro */
#define i2p(leftp) ( (struct prod *) ( (char *)leftp - (int) &0->p_left) )

struct tgo
{
	int	tg_trm;		/* ordinal number of terminal */
	int	tg_st;
};

struct ntgo
{
	int	ng_nt;		/* ordinal number of non terminal */
	int	ng_st;		/* index of state */
	union {				/* MWC DSC */
		struct rel *reln;	/* pointer to relation set */
		struct lset *look;	/* pointer to lookahead set */
	} cheapo;			/* they're not used at once, so... */
};
#define ng_rel cheapo.reln
#define ng_lset cheapo.look

struct redn
{
	struct prod *rd_prod;	/* production pointer */
	struct lset *rd_lset;	/* lookahead set */
};

/* relation between nt transitions */
struct rel
{
	int	r_count;
	int	r_list[];
};

struct trans
{
	struct ntgo *t_trans;
	int	t_level;
};

struct lset
{
	union {
		unsigned char u_bits[LSETSIZE];
		struct	lset *u_next;
	} un;
};
#define l_bits un.u_bits
#define l_next un.u_next

struct resv
{
	char	*r_name;
	int	r_val;
};

struct genre
{
	int	g_ordno;		/* current index for table ptr */
	int	g_maxord;		/* limit value for g_ordno */
	int	g_base;			/* "base" value for s_no */
	struct sym ***g_sptr;	/* pointer to table for type - MWC DSC */
	char	*g_name;
};

typedef union
{
	struct	sym *sptr;
	int	ival;
} YYSTYPE;

extern struct sym **symtab;	/* global symbol table */
extern struct sym **ntrmptr;	/* non terminal pointers into symtab */
extern struct sym **trmptr;	/* " terminal " " */
extern struct sym **typeptr;	/* " type " " */
extern struct state *states;
extern struct sitem **items;
extern struct prod **prdptr;
extern int yyline;
extern nerrors;
extern FILE *defin, *tabout, *actout, *listout, *optout, *fhdr;
extern int tno;
extern char *gramy;	/* input file name */
extern char *ytabc;	/* y.tab.c output file */
extern char *youtput;	/* listing file */
extern char *ytabh;	/* header file for token # definers */
extern char acttmp[], opttmp[];
extern char *parser;
extern	char	bounderr[];
extern struct sitem *nititem;
extern struct prod *nitprod;
extern verbose, yydebug;
extern pstat;
extern int nstates;
extern int nprod;
extern int maxstates;
extern int maxprod;
extern int nrrconf, nsrconf;
extern int ndupgos, ndupacts;
extern struct genre gtab[MAXT];
extern int startsym;
extern int predlev;
extern struct resv restab[];

char 	*calloc();
char	*yalloc();
char	*ptosym();
char	*prsym();
char	*nextarg();
struct	ntgo	*findnt();
struct	lset	*getset();
