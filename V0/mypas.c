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
	char* extension_expected = ".pas";
	char* objectFileName;
	char* sourceFileName;
  
	if (argc == 2) {
	      objectFileName = "out.s";
	      sourceFileName = argv[1];
	} else if (argc == 4) {
	      
	      if (strcmp(argv[1], "-o") == 0) {
		    objectFileName = argv[2];
		    sourceFileName = argv[3];
	      } 
	      else if (strcmp(argv[2], "-o") == 0) {
		    objectFileName = argv[3];
		    sourceFileName = argv[2];
	      }
	      else {
		    fprintf(stderr, "arguments provided were not as expected\n");
		    return ARGERR;
	      }
	  
	} else {
	      fprintf(stderr, "arguments provided were not as expected\n");
	      return ARGERR;
	}
	
  	source = fopen (sourceFileName, "r");	
	if (source == NULL) {
		fprintf (stderr, "%s: cannot open %s... exiting\n",
			argv[0], argv[1]);
		return FLNTFND;
	}  
  
        char* sourceExt = strchr(argv[1], '.');
	// Make sure user entered with ".pas" FILE
	if (strcmp(sourceExt, extension_expected) != 0) {
		fprintf(stderr, "incompatible file\n");
		return INCMPTY;
	}
	
	object = fopen(objectFileName, "w+");
        mypas();
	
	if (object)
	  fclose(object);
	
	if (error) {
	  remove(objectFileName);
	  return error;
	}
	
        return 0;
}
