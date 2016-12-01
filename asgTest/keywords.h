/**@<keywords.h>::**/

enum {
  BEGIN = 0x10001, IF, THEN, ELSE, WHILE, DO, REPEAT, UNTIL, VAR,
  BOOLEAN, INTEGER, REAL, DOUBLE, DIV, MOD, AND, OR, NOT, TRUE, FALSE, END
};

extern char *keywords[];
extern int iskeyword(char const *identifier);
