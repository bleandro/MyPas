/**@<parser.h>::**/

extern int lookahead; // @ local

extern int gettoken (FILE *); // @ lexer.c

extern FILE *source; // @ main.c

extern void match (int); // @ local

extern char lexeme[];

/* syntax names */
void cmd  (void);
int expr (int);
int superexpr (int);
int addop (void);
int mulop (void);
void mypas(void);
void body(void);
void imperative(void);
char** namelist(void);
void parmdef(void);
void declarative(void);
int vartype(void);
void fnctype(void);
void stmt(void);
void stmtlist(void);
void ifstmt(void);
void whilestmt(void);
void repstmt(void);
int is_ASGN_compatible(int, int);
int is_operand_compatible(int, int, int);
