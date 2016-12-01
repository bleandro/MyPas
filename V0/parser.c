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
	
	if(symtab_lookup(lexeme) < 0) {
	    symvec[i] = malloc(sizeof(lexeme) + 1);
	    strcpy(symvec[i], lexeme);
	    i++;
	} else {
	    error = IDDLCRD;
	    fprintf(stderr, "identifier already declared: %s\n", lexeme);
	}

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
		case DOUBLE:
			match(DOUBLE);
			return DOUBLE;
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
	int _endif, _else;
	match(IF);
	expr(BOOLEAN);
	_endif = _else = gofalse(labelcounter++);
	match(THEN);
	stmt();

	if(lookahead == ELSE){
		match(ELSE);
		_endif = jump(labelcounter++);
		mklabel(_else);
		stmt();
	}
	mklabel(_endif);
}

/** WHILE smpexpr DO stmt */
void whilestmt(void){
	int _whilehead = labelcounter++, _whiletail;
	match(WHILE);
	mklabel(_whilehead = labelcounter++);
	expr(BOOLEAN);
	gofalse(_whiletail = labelcounter++);
	match(DO);
	stmt();
	jump(_whilehead);
	mklabel(_whiletail);
}

/** REPEAT stmtlist UNTIL smpexpr */
void repstmt(void){
	int _repeat;
	match(REPEAT);
	_repeat = mklabel(labelcounter++);
	stmtlist();
	match(UNTIL);
	expr(BOOLEAN);
	gofalse(_repeat);
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

void assemble_relop(int relop) {
	compare();
  
	int _true, _false;
  	switch(relop) {
		case '>':
			_true = jump_greater(labelcounter++);
			break;
		case GEQ:
			_true = jump_greaterequal(labelcounter++);
			break;
		case '<':
			_true = jump_less(labelcounter++);
			break;
		case LEQ:
			_true = jump_lessequal(labelcounter++);
			break;
		case '=':
			_true = jump_equal(labelcounter++);
			break;
		case NEQ:
			_true = jump_notequal(labelcounter++);
			break;
	}
	move_false();
	_false = jump(labelcounter++);
	mklabel(_true);
	move_true();
	mklabel(_false);
}

/* syntax: expr -> smpexpr [ relop smpexpr ] */
int expr(int inherited_type)
{
	int reloperator;
	int t1 = 0, t2 = 0;
	t1 = smpexpr(0);
	if (reloperator = isrelop()) {
		t2 = smpexpr(t1);
		
		assemble_relop(reloperator);
	}
	
	if (t1 == t2 && t1 == BOOLEAN || t1 > BOOLEAN && t2 > BOOLEAN) {
	    return BOOLEAN;
	}
	else {
	    if ((inherited_type == BOOLEAN && t1 > BOOLEAN) || (t1 == BOOLEAN && inherited_type > BOOLEAN)) {
		error = TYMSMTCH;
		fprintf(stderr, "type mismatch: fatal error\n");
	    }
	    else {
	      return max(t1, inherited_type);
	    }
	}
	
	return -1;
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

void assemble_operation(int type, int operand){
    switch(operand){
      //Add operation
      case '+':
	switch(type){
	  case INTEGER:
	    addint();
	    break;
	    
	  case REAL:
	    addflt();
	    break;
	    
	  case DOUBLE:
	    adddbl();
	    break;
	    
	  case BOOLEAN:
	    addlog();
	    break;
	}
        break;
      
      //Sub operation
      case '-':
	switch(type){
	  case INTEGER:
	    subint();
	    break;
	    
	  case REAL:
	    subflt();
	    break;
	    
	  case DOUBLE:
	    subdbl();
	    break;
	}
      break;
      
      //Mul operation
      case '*':
	switch(type){
	  case INTEGER:
	    mulint();
	    break;
	    
	  case REAL:
	    mulflt();
	    break;
	    
	  case DOUBLE:
	    muldbl();
	    break;
	  
	  case BOOLEAN:
	    mullog();
	    break;
	}
      break;
      
      //Div operation
      case '/':
	switch(type){
	  case INTEGER:
	    divint();
	    break;
	    
	  case REAL:
	    divflt();
	    break;
	    
	  case DOUBLE:
	    divdbl();
	    break;
	}
      break;
      
      //Negate operation
      case NOT:
	switch(type){
	  case INTEGER:
	    negint();
	    break;
	    
	  case REAL:
	    negflt();
	    break;
	    
	  case DOUBLE:
	    negdbl();
	    break;
	  
	  case BOOLEAN:
	    neglog();
	    break;
	}
      break;
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
int smpexpr (int inherited_type)
{
	int acctype = inherited_type, syntype, varlocality, lvalue_seen = 0, ltype, rtype, muloperator = 0, addoperator = 0, notoperator = 0;

	if(lookahead == '-') {
		match('-');
		notoperator = NOT;
		if(acctype == BOOLEAN) {
			error = INCMPTY;
			fprintf(stderr, "incompatible types: fatal error.\n");
		}
		else if(acctype == 0) {
			acctype = INTEGER;
		}

	}
	else if(lookahead == NOT) {
		match(NOT);
		notoperator = NOT;
		if(acctype > BOOLEAN) {
			error = INCMPTY;
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
			error = IDNTFND;
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
			rtype = expr(ltype);

			if(is_ASGN_compatible(ltype, rtype)) {
				acctype = max(acctype, rtype);
			}
			else {
				error = INCMPTY;
				fprintf(stderr, "incompatible types in assignment\n");
			}
		}
		else if (varlocality > -1) {
		  	switch(acctype){
			  case INTEGER: case REAL:
				  rmovel(symtab_stream + symtab[varlocality][0]);
			  break;
			  case DOUBLE:
				  rmoveq(symtab_stream + symtab[varlocality][0]);
			  break;
			  default:
			  break;
			}
		}

		break;
	case DEC:
		{
		      int lexval = atoi(lexeme);  //alpha to Integer
		      char *integerIEEE = malloc(sizeof(lexeme)+1);
		      sprintf(integerIEEE, "$%i", lexval);
		      rmovel(integerIEEE);
		}
		match(DEC);
		syntype = INTEGER;
		if (acctype > BOOLEAN || acctype == 0) {
		    acctype = max(acctype, syntype);
		} else {
		    error = INCMPTY;
		    fprintf(stderr, "incompatible types: fatal error.\n");
		}
	break;
	case FLT:
		{
		      float lexval = atof(lexeme);  //alpha to float
		      char *fltIEEE = malloc(sizeof(lexeme)+1);
		      sprintf(fltIEEE, "$%i", *((int *)&lexval));
		      rmovel(fltIEEE);
		}
		match(FLT);
		syntype = REAL;
		if (acctype > BOOLEAN || acctype == 0) {
		    acctype = max(acctype, syntype);
		} else {
		    error = INCMPTY;
		    fprintf(stderr, "incompatible types: fatal error.\n");
		}
		break;
		
	case TRUE: case FALSE:
		match(lookahead);
		syntype = BOOLEAN;
		if (acctype == BOOLEAN || acctype == 0) {
		    acctype = BOOLEAN;
		} else {
		    error = INCMPTY;
		    fprintf(stderr, "incompatible types: fatal error.\n");
		}
		break;
	default:
		match ('(');
		syntype = expr(0);
		if((acctype == 0) || (is_ASGN_compatible(acctype, syntype))) {
		    acctype = max(acctype, syntype);
		}
		else {
		    acctype = -1;
		}
		match (')');
	}

	if (notoperator) {
	  assemble_operation(max(acctype, syntype), notoperator);
	  notoperator = 0;
	}
	
	// Test if there's a MUL Operation to execute
	if (muloperator){
		// Test if operand are compatible with types and execute the operation if it is
		if (is_operand_compatible(acctype, syntype, muloperator))
		  assemble_operation(max(acctype, syntype), muloperator); 
		else {
		  error = OPNTAPPLY;
		  fprintf(stderr, "operand not applicable\n");
		}
	}

	if (muloperator = mulop())
		goto F_Entry;
	
	// Test if there's an ADD Operation to execute
	if (addoperator){
		// Test if operand are compatible with types and execute the operation if it is
		if (is_operand_compatible(acctype, syntype, addoperator))
		  assemble_operation(max(acctype, syntype), addoperator); 
		else {
		  error = OPNTAPPLY;
		  fprintf(stderr, "operand not applicable\n");
		}
	}

	if (addoperator = addop())
		goto T_Entry;

	/* expression ends down here */

	if (lvalue_seen && varlocality > -1) {
		switch(ltype){
			case INTEGER: case REAL:
				lmovel(symtab_stream + symtab[varlocality][0]);
			break;
			case DOUBLE:
				lmoveq(symtab_stream + symtab[varlocality][0]);
			break;
			default:
			break;
		}
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

		 error = SYNTAX_ERR;
	 }
 }
