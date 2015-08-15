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

        else if( 0==strcmp(argv[idx], "-echo") ){
            if( (idx+1) >= argc ){ usage(-1); }
            printf("%s\n",argv[idx+1]);
        } 

        else if( 0==strcmp(argv[idx], "-open") ){
            xbrd = new Xboard();
            xbrd->Open();
            xbrd->StartPrus();
            xbrd->SetComplexSampleRate( 5000000 );
        }		 

        else if( 0==strcmp(argv[idx], "-usleep") ){
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);
            us_sleep( val );
        } 

        else if( 0==strcmp(argv[idx], "-write") ){
            int rval; 
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol( argv[idx+1], &end, 0 );

            rval = xbrd->XspiWrite( val );
            printf("Xutil:w=0x%04hx r=0x%04hx (%hd)\n",val,rval,rval);
        }
    
        else if( 0==strcmp(argv[idx], "-samp") ){
            short bf[4096];
            int   idx;

            xbrd->Get2kSamples( bf );
            for(idx=0;idx<32;idx++){
                printf("%03d, 0x%04hx %hd\n",idx,bf[idx],bf[idx]);
            }
        }

        else if( 0==strcmp(argv[idx], "-flush") ){
            xbrd->Flush();
        }

        else if( 0==strcmp(argv[idx], "-pru") ){
            xbrd->ShowPrus( "Xutil: pru state" );
        }

        // Move to next argument for parsing
        idx++;
    }

}

