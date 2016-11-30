/**@<mypas.c>::**/

/*
1: Tue Aug 16 20:50:47 BRT 2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <parser.h>
#include <mypaserrors.h>
#include <mypas.h>

FILE *source, *object;

extern int lookahead; // @ parser.c

int
main (int argc, char *argv[], char *envp[])
{
	object = fopen("object.s", "w+");
        if (argc == 1) {
                source = stdin;
        } else {
                source = fopen (argv[1], "r");
                if (source == NULL) {
                        fprintf (stderr, "%s: cannot open %s... exiting\n",
                                argv[0], argv[1]);
                        return FLNTFND;
                }
        }
        mypas();
	fclose(object);
	
	if (error)
	  remove("object.s");
        return 0;
}
