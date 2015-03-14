#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"

#include "../Iboard/Iboard.h"
#include "../Mboard/Mboard.h"
#include "../Adf4351/Adf4351.h"

#include "Lboard.h"

void usage( int exit_code )
{
    printf(
"This utility sweeps the frequency and level of a signal while measuring\n"
"the detector power. Results are displayed on stdout in csv format.  The\n"
"configuration is setup to have a synthesizer on an I boare port 1 while\n"
"the detector is on port 0.  Results are captuered by using a spectrum\n"
"analyzer with maxhold on to capture a level by frequency table.  This\n"
"is then correlated with the detector output readings to produce calibration\n"
"results. Results can be filterd by selecting lines ^CSV\n");
    printf("-pwr  <A>   Output pwr setting [0..3]\n");
    printf("-freq <Hz>  Start freq Hz\n");
    exit( exit_code );
}

int
GetAvePwrCode( Lboard *lbrd, int count )
{
    unsigned int sum;
    int          idx;

    sum = 0;
    for(idx=0;idx<count;idx++){
       sum += lbrd->GetPwrCode( );
    }

    return( (int)( (float)sum/(float)count )  );
}

int
GetMaxPwrCode( Lboard *lbrd, int nsecs )
{
    struct timeval tv1,tv2;
    unsigned int max,val;

    max = 0;
    gettimeofday( &tv1, NULL );
    while( 1 ){
       val = lbrd->GetPwrCode( );
       if( val>max ) max = val;
       gettimeofday(&tv2,NULL);
       if( tv2.tv_sec - tv1.tv_sec > nsecs ) return(max);
    }
}

int
main( int argc, char *argv[] )
{
    int            Lport, Mport;
    Gpio6PinGroup *g6pg = NULL;
    Iboard        *ibrd = NULL;
    Lboard        *lbrd = NULL;
    Mboard        *mbrd = NULL;
    Adf4351       *syn  = NULL;

    long long      startHz;
    int            outputPwr;

    int            loopCount;
    float          freqHz;
    float          attenDb;
    int            code;
    int            idx;
    char          *end;


    // Configure which ports boards are at
    Lport     = 0;
    Mport     = 1;

    // Configure frequency and power of scan start
    startHz   = 315000000;
    outputPwr = 0;

    // Configure default number of passes
    loopCount = 1;

    // Parse command line arguments
    idx = 1;
    while( idx < argc ){

        if( 0==strcmp(argv[idx], "-help") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-h") ){
            usage(0);
        }

	else if( 0==strcmp(argv[idx], "-freq") ){

            if( (idx+1) >= argc ){ usage(-1); }

            startHz = (long long)atof_suffix( argv[idx+1] );

	    idx+=2;
	    continue;
        }

        else if( 0==strcmp(argv[idx], "-pwr") ){
	    if( (idx+1) >= argc ){ usage(-1); }
	    outputPwr = strtol(argv[idx+1],&end,0);

	    idx+=2;
	    continue;
        }

        // Move to next argument for parsing
        idx++;
    }


    // Open the I board so we can access other boards
    ibrd = new Iboard();
    ibrd->Open();

    // Open L board
    g6pg  = ibrd->AllocPort( Lport );
    ibrd->EnablePort( Lport, 1 );
    lbrd = new Lboard();
    lbrd->Open( g6pg );
    lbrd->SetLog(0);

    // Open M board
    g6pg  = ibrd->AllocPort( Mport );
    ibrd->EnablePort( Mport, 1 );
    mbrd = new Mboard();
    mbrd->Open( g6pg );
    syn = mbrd->GetAdf4351( 0 );
    syn->SetLog( 0x0 );
    syn->SetAuxEnable( 0 );
    syn->SetMainPower( outputPwr );
    syn->SetMtld( 1 );

    printf("CSV, freq(Hz), atten(dB), det code\n");
    while( loopCount>0 ){
	loopCount--;
        freqHz = startHz - 3200000;

        for( attenDb=0.0; attenDb<=31.5; attenDb+=0.5 ){
            lbrd->SetAttenDb( attenDb );
            us_sleep( 5000 );

	    syn->SetFrequency( (long long)freqHz );
            us_sleep( 5000 );

	    // Read stale value
            code = lbrd->GetPwrCode( );

            // Monitor the power for specified seconds
	    code = GetMaxPwrCode(lbrd,2);

	    printf("CSV, %f, %f, %d\n",freqHz,attenDb,code);
	    freqHz += 100000;
        }	     
    }

}

