#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <macros.h>
#include <tokens.h>
#include <parser.h>
#include <keywords.h>
#include <lexer.h>
#include <ctype.h>
#include <symtab.h>
#include <mypas.h>

/*************************** LL(1) grammar definition ******************************/

/** mypas -> body'.' */
void mypas(void){
	body();
	match('.');
}

/** body -> declarative imperative */
void body(void){
	declarative();
	imperative();
}

/** declarative -> [ VAR namelist ':' vartype ';' || 
			{ namelist ':' vartype ';' } ]
                   { sbpmod sbpname parmdef [ ':' fnctype ] ';' body ';' } */
void declarative(void){
	int sbpmod;
	
	/* vardef -> VAR namelist ':' vartype ';' || vardef.symtab <- forall symbol in namelist.name do
								      		symtab_append(symbol,vartype.type) 
								      end do
	*/
	if(lookahead == VAR) {
		match(VAR);
		do {
			int type, i;
			char** namev = namelist();
			match(':');
			type = vartype();
			for(i = 0; namev[i]; i++) 
				symtab_append(namev[i], type);
			match(';');
		} while(lookahead == ID);
	}

	while(lookahead == PROCEDURE || lookahead == FUNCTION) {
		sbpmod = lookahead;
		match(lookahead);
		match(ID);
		parmdef();

		if(sbpmod == FUNCTION && lookahead == ':'){
			match(':');
			fnctype();
		}

		match(';');
		body();
		match(';');
	}
}

/** imperative -> BEGIN stmtlist END */
void imperative(void){
	match(BEGIN);
	stmtlist();
	match(END);
}

/** namelist -> ID {, ID} */
#define MAX_ARG_NUM 1024
char** namelist(void) {
	/**/ char** symvec = calloc(MAX_ARG_NUM, sizeof(char**)); /**/
	/**/ int i = 0; /**/

	_namelistbegin:
	/**/ symvec[i] = malloc(sizeof(lexeme) + 1);
	strcpy(symvec[i], lexeme);
	i++; /**/

	match(ID);
	if(lookahead == ',') {

		match(',');
		goto _namelistbegin;
	}
	return symvec;
}

/** parmdef -> [ ( [VAR] namelist ':' vartype { ';' [VAR] namelist ':' vartype } ) ] */
void parmdef(void) {
	if (lookahead == '(') {

		match('(');
		P_Entry:
		if(lookahead == VAR) match(VAR);
		namelist();
		match(':');
		vartype();

		if(lookahead == ';'){
			match(';');
			goto P_Entry;
		}
		match(')');

	}
}

int vartype(void) {
	switch(lookahead) {
		case INTEGER:
			match(INTEGER);
			return INTEGER;
			break;
		case REAL:
			match(REAL);
			return REAL;
			break;
		default: 
			match(BOOLEAN);
			return BOOLEAN;
	}
}

void fnctype(void){
	switch(lookahead){
		case INTEGER:
			match(INTEGER);
			break;
		case REAL:
			match(REAL);
			break;
		default: match(BOOLEAN);
	}
}

/*stmt -> imperative
	| IF expr THEN { stmt } [ ELSE stmt ]
*	| WHILE expr DO stmt
*	| REPEAT stmtlist UNTIL expr
*	| expr
*	| <empty>
*/
void stmt(void){
  switch (lookahead) {
    case BEGIN:
      imperative();
      break;
    case IF:
      ifstmt();
      break;
    case WHILE:
      whilestmt();
      break;
    case REPEAT:
      repstmt();
      break;
    case ID: case DEC: case '(': /* here after we expect FIRST(expr) */
      expr();
      break;
  }
}

/** stmtlist -> stmt { ';' stmt } */
void stmtlist(void){
	stmt();
	while(lookahead == ';'){
		match(';');
		stmt();
	}
}

/** IF expr THEN { stmt } [ ELSE stmt ] */
void ifstmt(void){
	match(IF); expr(); match(THEN);
		stmt();

	if(lookahead == ELSE){
		match(ELSE);
		stmt();
	}
}

/** WHILE expr DO stmt */
void whilestmt(void){
	match(WHILE); expr(); match(DO);
		stmt();
}

/** REPEAT stmtlist UNTIL expr */
void repstmt(void){
	match(REPEAT);
		stmtlist();
	match(UNTIL); expr();
}

/*
 * expr -> term rest
 *
 * rest -> addop term rest | <>
 *
 * term -> fact quoc
 *
 * quoc -> mulop fact quoc | <>
 *
 * fact -> vrbl | cons | ( expr )
 *
 * vrbl -> ID
 *
 * cons -> DEC
 *
 * addop -> '+' | '-'
 *
 * mulop -> '*' | '/'
 *
 * ID = [A-Za-z][A-Za-z0-9]*
 *
 * DEC = [1-9][0-9]* | 0
 *
 **********************************************************************************/

/* expr -> term rest */
/*
 * | OP     | BOOLEAN     | NUMERIC   | 
 * |--------|-------------|-----------|
 * | NOT    |    X        |    NA     | 
 * | OR     |    X        |    NA     |
 * | AND    |    X        |    NA     |
 * | CHS    |    NA       |    X      |
 * | '+''-' |    NA       |    X      |
 * | '*''/' |    NA       |    X      |
 * | DIV    |    NA       | INTEGER   |
 * | MOD    |    NA       | INTEGER   |
 * | RELOP  | BOOL X BOOL | NUM X NUM |

 * | EXPRESS | INTEGER  | REAL    | DOUBLE |
 * |---------|----------|---------|--------|
 * | INTEGER | INTEGER  | REAL    | DOUBLE |
 * | REAL    | REAL     | REAL    | DOUBLE |
 * | DOUBLE  | DOUBLE   | DOUBLE  | DOUBLE |
 *
 * | LVALUE  | BOOLEAN | INTEGER |  REAL  | DOUBLE |
 * |---------|---------|---------|--------|--------|
 * | BOOLEAN | BOOLEAN |   NA    |   NA   |   NA   |
 * | INTEGER |    NA   | INTEGER |   NA   |   NA   |
 * | REAL    |    NA   |   REAL  |  REAL  |   NA   |
 * | DOUBLE  |    NA   | DOUBLE  | DOUBLE | DOUBLE |
 *
 */
int expr (int inherited_type)
{
	/*[[*/ int acctype = inherited_type; /*]]*/
 	if(lookahead == '-') {
		match('-');
		/*[[*/
		if(acctype == BOOLEAN) {
			fprintf(stderr, "incompatible types: fatal error.\n");
		} 
		else if(acctype == 0) {
			acctype = INTEGER;
		}
		/*]]*/
	}
	else if(lookahead == NOT) {
		match(NOT);
		/*[[*/
		if(acctype > BOOLEAN) {
			fprintf(stderr, "incompatible types: fatal error.\n");
		}
		acctype = BOOLEAN;
		/*]]*/
	}
	term(acctype); 
	rest();
}

/** rest -> addop term rest | <> */
void rest (void)
{
	if (addop()) {
	  term(); 
	  rest();
	}
}

/** term -> fact quoc */
void term (int inherited_type)
{
  int acctype = inherited_type;
  fact(acctype);
  quoc();
}

/** quoc -> mulop fact quoc | <> */
void quoc (void)
{
	if (mulop()) {
		fact(); quoc();
	}
}

/** fact -> vrbl | cons | ( expr ) */
void fact (int syntype)
{
	/*[[*/ int acctype = syntype, varlocality, lvalue = 0; /*]]*/
	switch (lookahead) {
	case ID:
		/* symbol must be declared */		

		/*[[*/
		varlocality = symtab_lookup(lexeme);
		if (varlocality < 0)
			fprintf(stderr, "%s not declared\n", lexeme);
		/*]]*/
		

		match (ID); 
		/*[[*/
		if(lookahead == ASGN) {
			lvalue = 1;
			match(ASGN);
			expr();
		} 
		else if (varlocality > -1) {
			fprintf(object, "\tpushl %%eax\n\tmovl %%eax, %s\n", 
				symtab_stream + symtab[varlocality][0]);
		}
		/*]]*/
		break;
	case DEC:
		match (DEC); break;
	default:
		match ('('); 
		/*[[*/ syntype = expr(); /*]]*/		
		/*[[*/ if(is_compatible(syntype, acctype)) {
			acctype = max(acctype, syntype);
		}
		else {
			fprintf(stderr, "parethesized type incompatible with accumulated type: fatal error.");
		} /*]]*/
		 match (')');
	}

	/* expression ends down here */
	/*[[*/
	if (lvalue && varlocality > -1) {
		fprintf(object, "\tmovl %s, %%eax\n",
			symtab_stream + symtab[varlocality][0]);
	}
	/*]]*/
}

/** vrbl -> ID
 *
 * cons -> DEC
 *
 * ID = [A-Za-z][A-Za-z0-9]*
 *
 * DEC = [1-9][0-9]* | 0
 *
 * addop -> '+' | '-' */
int addop (void)
{
	switch(lookahead){
	case '+':
	  match('+'); return '+';
	case '-':
	  match('-'); return '-';
	case OR:
	  match(OR); return OR;
	}
	return 0;
}

/** mulop -> '*' | '/' */
int mulop (void)
{
	switch(lookahead){
	  case '*':
	    match('*'); return '*';
	  case '/':
	    match('/'); return '/';
	  case AND:
	    match(AND); return AND;
	}
	return 0;
}
/***************************** lexer-to-parser interface **************************/

int lookahead; // @ local

void match (int expected)
 {
	 if (expected == lookahead) {
		 lookahead = gettoken (source);
	 } else {
		 fprintf(stderr, "parser: token mismatch error. found # %d (%s) ", lookahead, lexeme);

		 fprintf(stderr, "whereas expected # %d\n", expected);

		 exit(SYNTAX_ERR);
	 }
 }
