#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tokens.h>
#include <parser.h>
#include <keywords.h>
#include <lexer.h>
#include <ctype.h>
#include <symtab.h>

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
	char** symvec = calloc(MAX_ARG_NUM, sizeof(char**));
	int i = 0;
	symvec[i] = malloc(sizeof(lexeme) + 1);
	strcpy(symvec[i], lexeme);
	i++;
	match(ID);
	while(lookahead == ',') {
		match(',');
		match(ID);
	}
	return symvec;
}

/** parmdef -> [ ( [VAR] namelist ':' vartype { ';' [VAR] namelist ':' vartype } ) ] */
void parmdef(void) {
	if(lookahead == '('){

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
			

		/*default: fprintf(stderr, "parser: token mismatch error. found # %d (%s) ", lookahead, lexeme);
		         fprintf(stderr, "whereas expected vartype");
						 exit(SYNTAX_ERR);*/
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

		/*default: fprintf(stderr, "parser: token mismatch error. found # %d (%s) ", lookahead, lexeme);
		         fprintf(stderr, "whereas expected function type");
						 exit(SYNTAX_ERR);*/
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
void expr (void)
{
 	if(lookahead == '-'){
		match('-');
	}
	else if(lookahead == 'NOT') {
	  match(NOT);
	}
	term(); 
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
void term (void)
{
  fact();
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
void fact (void)
{
	switch (lookahead) {
	case ID:
		/* symbol must be declared */
		
		match (ID); break;
	case DEC:
		match (DEC); break;
	default:
		match ('('); expr(); match (')');
	}
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
	}
	return 0;
}

/** mulop -> '*' | '/' */
int mulop (void)
{
	switch(lookahead){
	  case '*':
	    match('*'); 
	    return '*';
	  case '/':
	    match('/'); 
	    return '/';
	  case AND:
	    match(AND);
	    return AND;
	  case OR:
	    match(OR);
	    return(OR);
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
