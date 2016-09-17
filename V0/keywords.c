/**@<keywords.c>::**/

#include <keywords.h>
#include <string.h>

char *keywords[] = {
		"begin", "if", "then", "else", "while", "do", "repeat", "until", "var", "procedure", "function", "end"
};

int iskeyword(char const *identifier){

	int i;
	for(i = BEGIN; i <= END; i++){
		if(strcmp(keywords[i - BEGIN], identifier) == 0)
			return i;
	}
	return 0;
}
