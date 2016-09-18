/**@<mypas.c>::**/

/*
1: Tue Aug 16 20:50:47 BRT 2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <mypas.h>
#include <interpreter.h>
#include <parser.h>

FILE *source;

extern int lookahead; // @ parser.c

int
main (int argc, char *argv[], char *envp[])
{
        if (argc == 1) {
                source = stdin;
        } else {
                source = fopen (argv[1], "r");
                if (source == NULL) {
                        fprintf (stderr, "%s: cannot open %s... exiting\n",
                                argv[0], argv[1]);
                        exit (-1);
                }
        }

        lookahead = gettoken (source);

        mypas();
        printf("\n");

        return 0;
}
