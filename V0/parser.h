/**@<parser.h>::**/

extern int lookahead; // @ local

extern int gettoken (FILE *); // @ lexer.c

extern FILE *source; // @ main.c

extern void match (int); // @ local

/* syntax names */
void cmd  (void);
void expr (void);
void rest (void);
void term (void);
void quoc (void);
void fact (void);
int addop (void);
int mulop (void);
void mypas(void);
void body(void);
void imperative(void);
void namelist(void);
void parmdef(void);
void declarative(void);
void vartype(void);
void fnctype(void);
void stmt(void);
void stmtlist(void);
void ifstmt(void);
void whilestmt(void);
void repstmt(void);
