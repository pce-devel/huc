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
#define OP_HIGH_KEYWORD	22
#define OP_LOW_KEYWORD	23
#define OP_PAGE		24
#define OP_BANK		25
#define OP_VRAM		26
#define OP_PAL		27
#define OP_COUNTOF	28
#define OP_SIZEOF	29
#define OP_LINEAR	30
#define OP_OVERLAY	31
#define OP_LOW_SYMBOL	32
#define OP_HIGH_SYMBOL	33
#define OP_LOGICAL_OR	34
#define OP_LOGICAL_AND	35

/* operator priority (bigger number is higher precedence) */
/* precedence is left-to-right if same priority */
const int op_pri[] = {
   0 /* OP_START        */,
   0 /* OP_OPEN         */,
  10 /* OP_ADD          */,
  10 /* OP_SUB          */,
  11 /* OP_MUL          */,
  11 /* OP_DIV          */,
  11 /* OP_MOD          */,
  13 /* OP_NEG          */,
   9 /* OP_SHL          */,
   9 /* OP_SHR          */,
   3 /* OP_OR           */,
   4 /* OP_XOR          */,
   5 /* OP_AND          */,
  13 /* OP_COM          */,
  12 /* OP_NOT          */,
   6 /* OP_EQUAL        */,
   6 /* OP_NOT_EQUAL    */,
   7 /* OP_LOWER        */,
   7 /* OP_LOWER_EQUAL  */,
   7 /* OP_HIGHER       */,
   7 /* OP_HIGHER_EQUAL */,
  13 /* OP_DEFINED      */,
  13 /* OP_HIGH_KEYWORD */,
  13 /* OP_LOW_KEYWORD  */,
  13 /* OP_PAGE         */,
  13 /* OP_BANK         */,
  13 /* OP_VRAM         */,
  13 /* OP_PAL          */,
  13 /* OP_COUNTOF      */,
  13 /* OP_SIZEOF       */,
  13 /* OP_LINEAR       */,
  13 /* OP_TAGOF        */,
   8 /* OP_LOW_SYMBOL   */,
   8 /* OP_HIGH_SYMBOL  */,
   1 /* OP_LOGICAL_OR   */,
   2 /* OP_LOGICAL_AND  */
};

/* second argument */
const int op_2nd[] = {
   0 /* OP_START        */,
   0 /* OP_OPEN         */,
   1 /* OP_ADD          */,
   1 /* OP_SUB          */,
   1 /* OP_MUL          */,
   1 /* OP_DIV          */,
   1 /* OP_MOD          */,
   0 /* OP_NEG          */,
   1 /* OP_SHL          */,
   1 /* OP_SHR          */,
   1 /* OP_OR           */,
   1 /* OP_XOR          */,
   1 /* OP_AND          */,
   0 /* OP_COM          */,
   0 /* OP_NOT          */,
   1 /* OP_EQUAL        */,
   1 /* OP_NOT_EQUAL    */,
   1 /* OP_LOWER        */,
   1 /* OP_LOWER_EQUAL  */,
   1 /* OP_HIGHER       */,
   1 /* OP_HIGHER_EQUAL */,
   0 /* OP_DEFINED      */,
   0 /* OP_HIGH_KEYWORD */,
   0 /* OP_LOW_KEYWORD  */,
   0 /* OP_PAGE         */,
   0 /* OP_BANK         */,
   0 /* OP_VRAM         */,
   0 /* OP_PAL          */,
   0 /* OP_COUNTOF      */,
   0 /* OP_SIZEOF       */,
   0 /* OP_LINEAR       */,
   0 /* OP_OVERLAY      */,
   0 /* OP_LOW_SYMBOL   */,
   0 /* OP_HIGH_SYMBOL  */,
   1 /* OP_LOGICAL_OR   */,
   1 /* OP_LOGICAL_AND  */
};

unsigned int op_stack[64] = {
  OP_START
};                              /* operator stack */
unsigned int val_stack[64];     /* value stack */
int op_idx, val_idx;            /* index in the operator and value stacks */
int need_operator;              /* when set await an operator, else await a value */
char *expr;                     /* pointer to the expression string */
char *expr_stack[16];           /* expression stack */
struct t_symbol *expr_toplabl;  /* pointer to the innermost scope-label */
struct t_symbol *expr_lablptr;  /* pointer to the last-referenced label */
int expr_lablcnt;               /* number of label seen in an expression */
int expr_mprbank;               /* last-defined bank# in an expression */
int expr_overlay;               /* last-defined overlay# in an expression */
int complex_expr;               /* NZ if an expression contains operators */
const char *keyword[11] = {     /* predefined functions */
  "\7DEFINED",
  "\4HIGH",
  "\3LOW",
  "\4PAGE",
  "\4BANK",
  "\4VRAM",
  "\3PAL",
  "\7COUNTOF",
  "\6SIZEOF",
  "\6LINEAR",
  "\7OVERLAY"
};
