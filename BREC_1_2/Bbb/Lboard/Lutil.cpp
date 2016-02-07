#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "../Iboard/Iboard.h"
#include "Lboard.h"

void usage( int exit_code )
{
    printf("-select <N>    select <N>=port[0..2] for subsequent operations\n");
    printf("-enable <EN>   set enable on select port to <EN>=[0..1]\n");
    printf("-usleep <M>    sleeps <M> micro seconds\n");
    printf("-atten  <A>    set atten to <A> dB, <A>=[0.0..31.5]\n");
    printf("-monitor <Hz>  display loop on power meter output\n");
    printf("                 <Hz> hint of signal, 0.0 for default\n");
    printf("-slope         output csv scan of pwr meter by atten\n");
    printf("\n");
    printf("NOTE: You must select a port as the first operation\n");
    printf("e.g. -select 0 -enable 1 -atten 10\n");
    printf("will select port 0, enable it, and set the mosi to 1\n");
    exit( exit_code );
}

int
main( int argc, char *argv[] )
{
    Iboard        *ibrd;
    int            idx;

    Gpio6PinGroup *g6pg = NULL;
    Lboard        *lbrd = NULL;

    int            portN;

    int            val;
    char           *end;

    printf("Lutil: Enter Main\n");
    portN = 0;
    g6pg  = NULL;

    ibrd = new Iboard();
    ibrd->Open();

    idx = 1;
    while( idx < argc ){

        if( 0==strcmp(argv[idx], "-help") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-h") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-select") ){
            if( (idx+1) >= argc ){ usage(-1); }

            portN = strtol(argv[idx+1],&end,0);
            g6pg  = ibrd->AllocPort( portN );

            idx+=2;
            continue;
        }

        else if( 0==strcmp(argv[idx], "-enable") ){
            if( !g6pg ) { usage(-1); }
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);

            printf("enable port=%d enable=%d\n",portN,val);
            ibrd->EnablePort( portN, val );

            lbrd = new Lboard();
            lbrd->Open( g6pg );

            idx+=2;
            continue;
        }

        else if( 0==strcmp(argv[idx], "-usleep") ){
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);
            us_sleep( val );
        }

        else if( 0==strcmp(argv[idx], "-atten") ){
	    float attenDb;

            if( !g6pg || !lbrd ) { usage(-1); }
            if( (idx+1) >= argc ){ usage(-1); }

            attenDb = atof(argv[idx+1]);

	    printf("Lutil: atten is %f\n",attenDb);

            lbrd->SetAttenDb( attenDb );

            idx+=2;
            continue;
        }

        else if( 0==strcmp(argv[idx], "-monitor") ){
	    float pwrDbm;
	    float hintHz;
	    int   code;

            if( !g6pg || !lbrd ) { usage(-1); }
            if( (idx+1) >= argc ){ usage(-1); }

            hintHz = atof_suffix(argv[idx+1]);

            while( 1 ){
                pwrDbm = lbrd->GetPwrDbm( hintHz );
                code  = lbrd->GetPwrCode( );
                printf("pwr = %f dB, code=%d\n", pwrDbm,code);
		sleep(1);
            }

            idx+=2;
            continue;
        }

        else if( 0==strcmp(argv[idx], "-slope") ){
	    float code[1024], attenDb;

            if( !g6pg || !lbrd ) { usage(-1); }

	    lbrd->SetLog(0);
	    for( attenDb=0.0; attenDb<=31.5; attenDb+=0.5 ){

                lbrd->SetAttenDb( attenDb );

                code[0] = lbrd->GetPwrCode( );
	        for( idx=0;idx<10;idx++){	     
                    us_sleep( 5000 );
                    code[ idx ] = lbrd->GetPwrCode( );
		}    

                printf("%f, ", attenDb );
	        for( idx=0;idx<10;idx++){	     
                    printf("%f,", code[idx]);
		}    
                printf("\n");
            }

            idx+=2;
            continue;
        }

        // Move to next argument for parsing
        idx++;
    }

    printf("Lutil: Enter Exit\n");
}

