/**@<mypas.c>::**/

/*
1: Tue Aug 16 20:50:47 BRT 2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <parser.h>
#include <mypaserrors.h>
#include <mypas.h>

FILE *source, *object;

int
main (int argc, char *argv[], char *envp[])
{
        char* ext = strchr(argv[1], '.');
	char* pasExt = ".pas";
	char* objectName = "object.s";
	
	// Make sure user entered with ".pas" FILE
	if (strcmp(ext, pasExt) != 0) {
		fprintf(stderr, "incompatible file\n");
		return INCMPTY;
	}
		
	source = fopen (argv[1], "r");	
	if (source == NULL) {
		fprintf (stderr, "%s: cannot open %s... exiting\n",
			argv[0], argv[1]);
		return FLNTFND;
	}
	
	object = fopen(objectName, "w+");
        
        mypas();
	
	if (object)
	  fclose(object);
	
	if (error) {
	  remove(objectName);
	  return error;
	}
	
        return 0;
}
