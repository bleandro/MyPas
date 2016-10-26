/**@<symtab.c>::**/

#include <string.h>
#include <symtab.h>
#include <lexer.h>

int symtab[MAX_SYMTAB_ENTRIES][2];
int symtab_nextentry = 0;

char symtab_stream[MAX_SYMTAB_ENTRIES*(MAXID_SIZE+1)];
int symtab_stream_next_descriptor = 0;

int symtab_lookup(char const *name) {
	int i;
	for(i = symtab_nextentry - 1; i > -1; i--) {
		if(strcmp(symtab_stream[symtab[i][0]], name) == 0) {
			break;
		}		
	}
	return i;
}	

int symtab_append(char const *name, int type) {
	if(symtab_nextentry < MAX_SYMTAB_ENTRIES) {		
		int has_symbol = symtab_lookup(name);
		if(has_symbol == -1) {
			strcpy(symtab_stream + symtab_stream_next_descriptor, name);
			/* store the stream position in the symtab array */
			symtab[symtab_nextentry][0] = symtab_stream_next_descriptor;			
			/* preview next entry position */			
			symtab_stream_next_descriptor += strlen(name) + 1;											
			return symtab_nextentry++;	
		}
		return 0;
	} 
	return 0;
}

