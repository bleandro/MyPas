/**@<keywords.c>::**/

#include <keywords.h>
#include <string.h>

#include <stdio.h>

char *keywords[] = {
		"begin", "if", "then", "else", "while", "do", "repeat", "until", "var", "procedure", "function",
		"boolean", "integer", "real", "double", "div", "mod", "and", "or", "not", "true", "false", "end"
};

int iskeyword(char const *identifier){
	int i;

	for(i = BEGIN; i <= END; i++){
		if(strcmp(keywords[i - BEGIN], identifier) == 0)
			return i;
	}
	return 0;
}
