#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Util/mcf.h"
#include "Util/gpioutil.h"

#include "Iboard/Iboard.h"
#include "Bdc/Bdc.h"

#include "Adf4351/Adf4351.h"
#include "Mboard.h"

void usage( int exit_code )
{
    printf("-select <N>  select <N>=port[0..2] for subsequent operations\n");
    printf("-enable <EN> set enable on select port to <EN>=[0..1]\n");
    printf("-usleep <M>  sleeps <M> micro seconds\n");
    printf("-pwr    <V>  set syn output pwr setting to <V> [0..3]\n");
    printf("-freq   <V>  set syn freqyency to <V> Hertz\n");
    printf("             NOTE: This causes programming\n");
    printf("-start  <V>  set sweep start freqyency to <V> Hertz\n");
    printf("-stop   <V>  set sweep stop freqyency to <V> Hertz\n");
    printf("-delta  <V>  set sweep step freqyency to <V> Hertz\n");
    printf("-dwell  <V>  set sweep dwell time <V> microseconds\n");
    printf("-sweep       start sweep with set parameters\n");
    printf("             NOTE: This causes loop/processing\n");
    printf("\n");
    printf("NOTE: You must select a port as the first operation\n");
    printf("e.g. -select 0 -enable 1 -freq 50000000\n");
    printf("will select port 0, enable it, and set the mosi to 1\n");
    exit( exit_code );
}

int
main( int argc, char *argv[] )
{
    int           idx;
    int            val;
    char          *end;

    int            portN;
    int            outputPwr;
    double         startHz;
    double         stopHz;
    double         deltaHz;
    double         dwellUs;

    printf("Mutil: Enter Main\n");
    portN     = 0;
    outputPwr = 3;
    startHz   = 100000000;
    stopHz    = 110000000;
    deltaHz   = 1000000;
    dwellUs   = 500000;

    portN     = 1;

    Mboard  *mbrd;
    Adf4351 *syn;

    mbrd = new Mboard();

    if( FindCapeByName( "brecFpru" ) || FindCapeByName( "brecFjtag" ) ){
       Bdc     *bdc;

       bdc = new Bdc();
       bdc->Open();

       mbrd->Attach( (void*)bdc, (void*)portN );
    }
    else{
       Iboard        *ibrd;

       ibrd = new Iboard();
       ibrd->Open();

       mbrd->Attach( (void*)ibrd, (void*)portN );
    }

    syn = mbrd->GetAdf4351( 0 );

    idx = 1;
    while( idx < argc ){

        if( 0==strcmp(argv[idx], "-help") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-h") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-usleep") ){
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);
            us_sleep( val );
        }

        else if( 0==strcmp(argv[idx], "-pwr") ){
            if( (idx+1) >= argc ){ usage(-1); }
            outputPwr = strtol(argv[idx+1],&end,0);
        }

        else if( 0==strcmp(argv[idx], "-freq") ){
	    long long lval;
	    double    freqHz;

            if( (idx+1) >= argc ){ usage(-1); }

	    freqHz = atof_suffix( argv[idx+1] );
	    lval   = (long long)freqHz;

	    printf("f is %llu\n",lval);

            syn->SetLog( 0xffffffff );
            syn->SetFrequency( lval );
            syn->SetAuxEnable( 0 );
            syn->SetMainPower( outputPwr );
            syn->Show();

            us_sleep( 100000 );
            printf("Lock status=%d\n",syn->GetLock());
           
            idx+=2;
            continue;
        }

        else if( 0==strcmp(argv[idx], "-start") ){
            if( (idx+1) >= argc ){ usage(-1); }
	    startHz = atof_suffix( argv[idx+1] );
        }

        else if( 0==strcmp(argv[idx], "-stop") ){
            if( (idx+1) >= argc ){ usage(-1); }
	    stopHz = atof_suffix( argv[idx+1] );
        }

        else if( 0==strcmp(argv[idx], "-delta") ){
            if( (idx+1) >= argc ){ usage(-1); }
	    deltaHz = atof_suffix( argv[idx+1] );
        }

        else if( 0==strcmp(argv[idx], "-dwell") ){
            if( (idx+1) >= argc ){ usage(-1); }
	    dwellUs = atof_suffix( argv[idx+1] );
        }

        else if( 0==strcmp(argv[idx], "-sweep") ){

	    long long freqHz;


	    if( startHz > stopHz ){
               fprintf(stderr, "start > stop\n");
               usage(-1);
            }

            syn->SetLog( 0xffffffff );
            syn->SetAuxEnable( 0 );
            syn->SetMainPower( outputPwr );
            syn->SetMtld(1);
            syn->SetLog( 0x0 );
            syn->Show();

	    printf(" startHz=%f\n stopHz=%f\n deltaHz=%f\n dwellUs=%f\n",
			    startHz,stopHz, deltaHz, dwellUs);

	    while( 1 ){
                freqHz =  (long long)startHz;
		while( freqHz < stopHz ){
                   syn->SetFrequency( freqHz );
		   freqHz += deltaHz;
		   if( dwellUs > 0 ) us_sleep( dwellUs );
		   printf("f = %lld\n",freqHz);
	        }		 
            }		       

            continue;
        }		 

        idx++;
    }

    printf("Mutil: Enter Exit\n");
}

