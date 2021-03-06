
#line 25 "gram.y"
typedef union {
	opcode	opcode;
	rvalue	*lvalue;
	char	*svalue;
	dicent	*dvalue;
	int	ivalue;
	code	*location;
} YYSTYPE;
#define NUMBER 256
#define STRING 257
#define IDENTIFIER 258
#define ADDAB 259
#define AUTO 260
#define BREAK 261
#define CONTINUE 262
#define DECR 307
#define DEFINE 264
#define DIVAB 265
#define DO 266
#define DOT 267
#define ELSE 268
#define EQP 269
#define ERROR 270
#define EXPAB 271
#define FOR 272
#define GEP 273
#define GTP 274
#define IBASE 275
#define IF 276
#define INCR 306
#define LENGTH_ 278
#define LEP 279
#define LTP 280
#define MULAB 281
#define NEP 282
#define OBASE 283
#define QUIT 284
#define REMAB 285
#define RETURN_ 286
#define SCALE_ 287
#define SQRT_ 288
#define SUBAB 289
#define WHILE 290
#define UMINUS 308
#ifdef YYTNAMES
extern struct yytname
{
	char	*tn_name;
	int	tn_val;
} yytnames[];
#endif
extern	YYSTYPE	yylval;
