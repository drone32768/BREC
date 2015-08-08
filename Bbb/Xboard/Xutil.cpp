#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "Xboard.h"

void usage( int exit_code )
{
    printf("This utility uses the X board device interface software");
    exit( exit_code );
}

int
main( int argc, char *argv[] )
{
    int            idx;	  
    Xboard        *xbrd;
    int            val;
    char          *end;

    xbrd = NULL;

    idx = 1;
    while( idx < argc ){

        if( 0==strcmp(argv[idx], "-help") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-h") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-open") ){
            xbrd = new Xboard();
            xbrd->Open();
            xbrd->StartPrus();
            xbrd->SetComplexSampleRate( 5000000 );
        }		 

        else if( 0==strcmp(argv[idx], "-write") ){
            int rval; 
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol( argv[idx+1], &end, 0 );

            printf("Xutil:write  : 0x%08x\n",val);
            rval = xbrd->XspiWrite( val );
            printf("Xutil:read   : 0x%08x\n",rval);
        }

        // Move to next argument for parsing
        idx++;
    }

}

