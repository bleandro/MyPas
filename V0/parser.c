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

    /* here after we expect FIRST(expr) */
    case ID: 
    case DEC:
    case FLT:
    case TRUE:
    case FALSE:
    case NOT:
    case '-':
    case '(': 
      expr(0);
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

/** IF expr THEN { stmt } [ ELSE stmt ] */
int labelcounter = 1;
void ifstmt(void){
	int _endif, _else;
	match(IF); 
	superexpr(BOOLEAN);
	fprintf(object, "\tjz .L%d\n", _endif = _else = labelcounter++);
	match(THEN);
	stmt();

	if(lookahead == ELSE){
		match(ELSE);
		fprintf(object, "\tjmp .L%d\n",	_endif = labelcounter++);
		fprintf(object, ".L%d:\n", _else);
		stmt();
	}
	fprintf(object, ".L%d:\n", _endif);
}

/** WHILE expr DO stmt */
void whilestmt(void){
	int _while, _end;
	match(WHILE); 
	superexpr(BOOLEAN); 
	fprintf(object, ".L%d:\n", _while = labelcounter++);
	fprintf(object, "\tjz .L%d\n",	_end = labelcounter++);
	match(DO);
	stmt();
	fprintf(object, "\tjmp .L%d\n",	_while);
	fprintf(object, ".L%d:\n", _end);
}

/** REPEAT stmtlist UNTIL expr */
void repstmt(void){
	int _repeat;
	match(REPEAT);
	fprintf(object, ".L%d:\n", _repeat = labelcounter++);
	stmtlist();
	match(UNTIL); 	
	superexpr(BOOLEAN);
	fprintf(object, "\tjz .L%d\n",	_repeat);
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

/* syntax: superexpr -> expr [ relop expr ] */
int superexpr(int inherited_type)
{
	int reloperand = 0;
	int t1 = 0, t2 = 0;
	t1 = expr(inherited_type);
	if (reloperand = isrelop()) {
		t2 = expr(t1);
		
		if (!(is_operand_compatible(t1, t2, reloperand)))
		  fprintf(stderr, "operand not applicable\n");
		
		return BOOLEAN;
	}
	
	if (t1 < 0)
	  fprintf(stderr, "type mismatch: fatal error\n");

	return max(t1, t2);
}

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
*/
int is_operand_compatible(int ltype, int rtype, int operand){
      switch(operand){
	case '+':case '-':
	case '*':case '/':
	  if ((ltype > BOOLEAN) && (rtype > BOOLEAN)) return 1;
	  break;
	  
	case OR: case AND:
	  if ((ltype == BOOLEAN) && (rtype == BOOLEAN)) return 1;
	  break;
	  
	case DIV: case MOD:
	  if ((ltype == INTEGER) && (rtype == INTEGER)) return 1;
	  
	case '>': case '<':
	case '=': case NEQ:
	case GEQ: case LEQ:
	  if (ltype == rtype) return 1;	
	  break;
      }
      
      return 0;
}

/* 
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
int is_ASGN_compatible(int ltype, int rtype)
{
	switch(ltype){
		case BOOLEAN:
		case INTEGER:
			if (ltype == rtype) return ltype;
			break;
		case REAL:
			switch(rtype) {
				case INTEGER: case REAL:
					return ltype;
			}
			break;
		case DOUBLE:
			switch(rtype) {
				case INTEGER: case REAL: case DOUBLE:
					return ltype;
			}
	}

	return 0;
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
int expr (int inherited_type)
{
	int acctype = inherited_type, syntype, varlocality, lvalue_seen = 0, ltype, rtype, muloperand = 0, addoperand = 0;
 	
	if(lookahead == '-') {
		match('-');
		
		if(acctype == BOOLEAN) {
			fprintf(stderr, "incompatible types: fatal error.\n");
		} 
		else if(acctype == 0) {
			acctype = INTEGER;
		}
		
	}
	else if(lookahead == NOT) {
		match(NOT);
		if(acctype > BOOLEAN) {
			fprintf(stderr, "incompatible types: fatal error.\n");
		}
		acctype = BOOLEAN;
		
	}

   T_Entry:
   F_Entry:
	switch (lookahead) {
	case ID:
		/* symbol must be declared */		
		varlocality = symtab_lookup(lexeme);
		if (varlocality < 0) {
			fprintf(stderr, "identifier not found: %s\n", lexeme);
			syntype = -1;
		}
		else {
			syntype = symtab[varlocality][1];
		}
		
		if (acctype == 0) acctype = syntype;

		match(ID); 
		if(lookahead == ASGN) {		// ASGN = ":="
			/* located variable is LVALUE */
			lvalue_seen = 1;
			ltype = syntype;
			match(ASGN);
			rtype = superexpr(ltype);

			if(is_ASGN_compatible(ltype, rtype)) {
				acctype = max(acctype, rtype);
			}
			else {
				fprintf(stderr, "incompatible types in assignment\n");
			}
		} 
		else if (varlocality > -1) {
			fprintf(object, "\tpushl %%eax\n\tmovl %%eax, %s\n", 
				symtab_stream + symtab[varlocality][0]);
		}

		break;
	case DEC:
		match(DEC);
		syntype = INTEGER;
		if (acctype > BOOLEAN || acctype == 0) {
		    acctype = max(acctype, syntype);
		} else {
		    fprintf(stderr, "incompatible types: fatal error.\n");
		}
		break;
        case FLT:
		match(FLT);
		syntype = REAL;
		if (acctype > BOOLEAN || acctype == 0) {
		    acctype = max(acctype, syntype);
		} else {
		    fprintf(stderr, "incompatible types: fatal error.\n");
		}
		break;
	case TRUE: case FALSE:
		match(lookahead);
		syntype = BOOLEAN;
		if (acctype == BOOLEAN || acctype == 0) {
		    acctype = BOOLEAN;
		} else {
		    fprintf(stderr, "incompatible types: fatal error.\n");
		}
		break;
	default:
		match ('('); 
		syntype = superexpr(0); 		
		if((acctype == 0) || (is_ASGN_compatible(acctype, syntype))) {
		    acctype = max(acctype, syntype);
		}
		else {
		    acctype = -1;
		}
		match (')');
	}

	if ((muloperand) > 0 || (addoperand > 0)){
		if (!(is_operand_compatible(acctype, syntype, max(addoperand, muloperand))))
		  fprintf(stderr, "operand not applicable\n");
	}
	
	if (muloperand = mulop())
		goto F_Entry;
	
	if (addoperand = addop())
		goto T_Entry;

	/* expression ends down here */
	
	if (lvalue_seen && varlocality > -1) {
		fprintf(object, "\tmovl %s, %%eax\n",
			symtab_stream + symtab[varlocality][0]);
	}
	
	return acctype;
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
