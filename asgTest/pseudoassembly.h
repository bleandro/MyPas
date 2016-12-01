/**@<pseudoassembly.h>::**/

//unified label counter
extern int labelcounter;

//control pseudo-instructions
int gofalse(int label);
int jump(int label);
int jump_lessequal(int label);
int jump_less(int label);
int jump_greaterequal(int label);
int jump_greater(int label);
int jump_equal(int label);
int jump_notequal(int label);
int compare();
int move_true();
int move_false();
int mklabel(int label);
int lmovel(char const *variable);
int lmoveq(char const *variable);
int rmovel(char const *variable);
int rmoveq(char const *variable);

//ULA pseudo-instruction
//unary
int neglog(void);
int negint(void);
int negflt(void);
int negdbl(void);
//binary addition and inverse
int addlog(void);
int addint(void);
int addflt(void);
int adddbl(void);

int subint(void);
int subflt(void);
int subdbl(void);
//binary multiplication and inverse
int mullog(void);
int mulint(void);
int mulflt(void);
int muldbl(void);

int divint(void);
int divflt(void);
int divdbl(void);
