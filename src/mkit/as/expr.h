/* value types */
#define T_DECIMAL	0
#define T_HEXA		1
#define T_BINARY	2
#define T_CHAR		3
#define T_SYMBOL	4

/* operators */
#define OP_START	0
#define OP_OPEN		1
#define OP_ADD		2
#define OP_SUB		3
#define OP_MUL		4
#define OP_DIV		5
#define OP_MOD		6
#define OP_NEG		7
#define OP_SHL		8
#define OP_SHR		9
#define OP_OR		10
#define OP_XOR		11
#define OP_AND		12
#define OP_COM		13
#define OP_NOT		14
#define OP_EQUAL	15
#define OP_NOT_EQUAL	16
#define OP_LOWER	17
#define OP_LOWER_EQUAL	18
#define OP_HIGHER	19
#define OP_HIGHER_EQUAL 20
#define OP_DEFINED	21
#define OP_HIGH		22
#define OP_LOW		23
#define OP_PAGE		24
#define OP_BANK		25
#define OP_VRAM		26
#define OP_PAL		27
#define OP_SIZEOF	28
#define OP_dotLO	29
#define OP_dotHI	30

/* operator priority */
const int op_pri[] = {
	 0 /* START */,  0 /* OPEN  */,
	 8 /* ADD   */,  8 /* SUB   */,  9 /* MUL   */,  9 /* DIV   */,
	 9 /* MOD   */, 11 /* NEG   */,  7 /* SHL   */,  7 /* SHR   */,
	 2 /* OR    */,  3 /* XOR   */,  4 /* AND   */, 11 /* COM   */,
	10 /* NOT   */,  5 /* =     */,  5 /* <>    */,  6 /* <     */,
	 6 /* <=    */,  6 /* >     */,  6 /* >=    */,
	11 /* DEFIN.*/, 11 /* HIGH  */, 11 /* LOW   */, 11 /* PAGE  */,
	11 /* BANK  */, 11 /* VRAM  */, 11 /* PAL   */, 11 /* SIZEOF*/,
	 1 /* dotLO */,  1 /* dotHI */
};

unsigned int op_stack[64] = {
	OP_START
};				/* operator stack */
unsigned int val_stack[64];	/* value stack */
int op_idx, val_idx;		/* index in the operator and value stacks */
int need_operator;		/* when set await an operator, else await a value */
char *expr;			/* pointer to the expression string */
char *expr_stack[16];		/* expression stack */
struct t_symbol *expr_lablptr;	/* pointer to the lastest label */
int expr_lablcnt;		/* number of label seen in an expression */
int simple_expr;		/* NZ if the expression has no operators */
const char *keyword[8] = {	/* predefined functions */
	"\7DEFINED",
	"\4HIGH", "\3LOW",
	"\4PAGE", "\4BANK",
	"\4VRAM", "\3PAL",
	"\6SIZEOF"
};

