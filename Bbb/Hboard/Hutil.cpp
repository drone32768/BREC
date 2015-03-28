#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "Hboard.h"

void usage( int exit_code )
{
    printf("This utility collects 64k samples and produces a histogram\n");
    printf("The output is inter mixed.  A csv file can be obtained by\n");
    printf("greping for CSV lines. Similarly, non zero bins have a NZ\n");
    exit( exit_code );
}

int
main( int argc, char *argv[] )
{
    int            idx;	  
    Hboard        *hbrd;


    idx = 1;
    while( idx < argc ){

        if( 0==strcmp(argv[idx], "-help") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-h") ){
            usage(0);
        }

        // Move to next argument for parsing
        idx++;
    }

    hbrd = new Hboard();
    hbrd->Open();
    hbrd->StartPrus();
    hbrd->SetComplexSampleRate( 5000000 );

    short          *bf;
    unsigned short samples[65536];
    unsigned int   histo[4096];
    unsigned short code;

    // Wait for buffers to wrap at least once
    printf("waiting for fill ...\n");
    sleep(2 );

    printf("collecting samples...\n");
    bf = (short*)samples;
    for( idx=0;idx<32; idx++ ){
       hbrd->Get2kSamples( bf );
       bf += 2048;
    }

    printf("convert samples back to 12 bits...\n");
    for( idx=0;idx<65536;idx++){
       samples[idx] = samples[idx] >> 4; 
    }

    printf("clear histogram...\n");
    for( idx=0;idx<4096;idx++){
       histo[idx] =0;
    }

    printf("creating histogram...\n");
    for( idx=0;idx<65536;idx++){
       code = samples[idx];
       histo[code]++;
    }

    printf("output histogram...\n");
    for( idx=0;idx<4096;idx++){
       if( histo[idx] > 0 ) {	     
         printf("NZ,CSV,%d,%d\n",idx,histo[idx]);	     
       }
       else{
         printf("ZE,CSV,%d,%d\n",idx,histo[idx]);	     
       }
    }
}

