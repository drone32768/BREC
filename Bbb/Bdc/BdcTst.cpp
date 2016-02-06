#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <fftw3.h>

#include "../Util/mcf.h"
#include "Bdc.h"


////////////////////////////////////////////////////////////////////////////////
void usage( int exit_code )
{
    printf("This utility uses the X board device interface software");
    printf("-echo Msg     Echo's Msg to output\n");
    printf("-usleep N     sleeps N microseconds\n");
    printf("-open         open's device (required first step)\n");
    printf("-write  N     writes N to host spi port\n");
    exit( exit_code );
}

int
main( int argc, char *argv[] )
{
    int            idx;	  
    Bdc           *bdc;
    int            val;
    char          *end;

    bdc = NULL;

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

        else if( 0==strcmp(argv[idx], "-usleep") ){
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);
            us_sleep( val );
        } 

        else if( 0==strcmp(argv[idx], "-open") ){
            bdc = new Bdc();
            bdc->Open();
        }		 

        else if( 0==strcmp(argv[idx], "-write") ){
            int rval; 
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol( argv[idx+1], &end, 0 );

            rval = bdc->SpiRW16( val );
            printf("BdcTst:w=0x%04hx r=0x%04hx (%hd)\n",val,rval,rval);
        }
    
        // Move to next argument for parsing
        idx++;
    }

}

