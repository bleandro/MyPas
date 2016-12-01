#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <pseudoassembly.h>
#include <macros.h>
#include <tokens.h>
#include <keywords.h>
#include <lexer.h>
#include <symtab.h>
#include <mypas.h>
#include <mypaserrors.h>
#include <parser.h>

/*************************** LL(1) grammar definition ******************************/

int error;
/** mypas -> body'.' */
void mypas(void){
	error = 0; // Initial value for error
	lookahead = gettoken (source);
	body();
	match('.');
}

/** body -> declarative imperative */
void body(void){
	declarative();
	imperative();
}

/** declarative -> [ VAR namelist ':' vartype ';' ||
			{ namelist ':' vartype ';' } ] */
void declarative(void){
	if(lookahead == VAR) {
		match(VAR);
		do {
			namelist();
			match(':');
			vartype();
			match(';');
		} while(lookahead == ID);
	}
}

/** imperative -> BEGIN stmtlist END */
void imperative(void){
	match(BEGIN);
	stmtlist();
	match(END);
}

/** namelist -> ID {, ID} */
void namelist(void) {
	match(ID);
	if(lookahead == ',') {
		match(',');
		goto _namelistbegin;
	}
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
		case DOUBLE:
			match(DOUBLE);
			return DOUBLE;
		default:
			match(BOOLEAN);
			return BOOLEAN;
	}
}

/*stmt -> imperative
	| IF smpexpr THEN { stmt } [ ELSE stmt ]
*	| WHILE smpexpr DO stmt
*	| REPEAT stmtlist UNTIL smpexpr
*	| smpexpr
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

    /* here after we expect FIRST(smpexpr) */
    case ID:
    case DEC:
    case FLT:
    case TRUE:
    case FALSE:
    case NOT:
    case '-':
    case '(':
      smpexpr(0);
      break;

    default:
        /*<epsilon>*/
        ;
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

/** IF smpexpr THEN { stmt } [ ELSE stmt ] */
void ifstmt(void){
	match(IF);
	expr(BOOLEAN);
	match(THEN);
	stmt();

	if(lookahead == ELSE){
		match(ELSE);
		stmt();
	}
}

/** WHILE smpexpr DO stmt */
void whilestmt(void){
	match(WHILE);
	expr(BOOLEAN);
	match(DO);
	stmt();
}

/** REPEAT stmtlist UNTIL smpexpr */
void repstmt(void){
	match(REPEAT);
	stmtlist();
	match(UNTIL);
	expr(BOOLEAN);
}

int isrelop(void)
{	
	switch(lookahead) {
		case '>':
			match('>');
			if (lookahead == '='){
				match('=');
				return GEQ;
			}
			return '>';
		case '<':
			match('<');
			if (lookahead == '>') {
				match('>');
				return NEQ;
			}
			if (lookahead == '=') {
				match('=');
				return LEQ;
			}
			return '<';
		case '=':
			match('=');
			return '=';
	}

	return 0;
}

/* syntax: expr -> smpexpr [ relop smpexpr ] */
int expr()
{
	smpexpr();
	if (isrelop()) {
		smpexpr();
	}
}

/*
 * smpexpr -> term rest
 *
 * rest -> addop term rest | <>
 *
 * term -> fact quoc
 *
 * quoc -> mulop fact quoc | <>
 *
 * fact -> vrbl | cons | ( smpexpr )
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
int smpexpr ()
{
	if(lookahead == '-') {
		match('-');

	}
	else if(lookahead == NOT) {
		match(NOT);

	}

   T_Entry:
   F_Entry:
	switch (lookahead) {
	case ID:

		match(ID);
		if(lookahead == ASGN) {		// ASGN = ":="
			match(ASGN);
			expr();
		}

		break;
	case DEC:
		match(DEC);
		break;
	case FLT:
		match(FLT);
		break;
	case TRUE: case FALSE:
		match(lookahead);
		break;
	default:
		match ('(');
		expr();
		match (')');
	}

	if (mulop())
	    goto F_Entry;
	
	if (addop())
	    goto T_Entry;
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

		 error = SYNTAX_ERR;
	 }
 }
