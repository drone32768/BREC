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

class Lgen {
private:
    Gpio6PinGroup *mg6pg;
    Iboard        *mIbrd;
    Lboard        *mLbrd;
    Mboard        *mMbrd;
    Adf4351       *mSyn;
    int            mNest;
#   define         LGEN_LOG_SETOUTPUT 0x00000010
    unsigned int   mLog;

public:
    Lgen();
    int    Open();
    double GetPwrEst( int nEst );
    int    SetOutput( double targetHz, double targetDbm, 
                      double *actHz,   double *actDbm );
    
};

Lgen::Lgen()
{
    mg6pg = NULL;
    mIbrd = NULL;
    mLbrd = NULL;
    mMbrd = NULL;
    mSyn  = NULL;
    mNest = 10;
    // mLog  = 0xffffffff;
    mLog  = 0;
}

int
Lgen::Open()
{
    // Configure which ports boards are at
    int            Lport, Mport;
    Lport     = 0;
    Mport     = 1;

    // Open the I board so we can access other boards
    mIbrd = new Iboard();
    mIbrd->Open();

    // Open L board
    mg6pg  = mIbrd->AllocPort( Lport );
    mIbrd->EnablePort( Lport, 1 );
    mLbrd = new Lboard();
    mLbrd->Open( mg6pg );
    mLbrd->SetLog(0);

    // Maximize attenuation before opening synthesizer
    mLbrd->SetAttenDb( mLbrd->GetMaxAttenDb() );

    // Open M board
    mg6pg  = mIbrd->AllocPort( Mport );
    mIbrd->EnablePort( Mport, 1 );
    mMbrd = new Mboard();
    mMbrd->Open( mg6pg );
    mSyn = mMbrd->GetAdf4351( 0 );
    mSyn->SetLog( 0x0 );
    mSyn->SetAuxEnable( 0 );
    mSyn->SetMtld( 1 );

    return(0);
}


double 
Lgen::GetPwrEst( int nEst )
{
    int    cnt;
    double sum;

    mLbrd->GetPwrDbm();
    sum = 0;
    for(cnt=0;cnt<nEst;cnt++){
       sum+=mLbrd->GetPwrDbm( );
    }
    return( sum/nEst );
}

int
Lgen::SetOutput( double targetHz, double targetDbm, 
                 double *actHz,   double *actDbm )
{
    int    mSynPwr;
    double attenDb;
    double dbm,delta;

    if( mLog&LGEN_LOG_SETOUTPUT ){
        printf("TargetHz=%f, TargetDbm=%f\n",targetHz,targetDbm);
    }

    mSynPwr  = 0;
    attenDb  = mLbrd->GetMaxAttenDb();
    mLbrd->SetAttenDb( attenDb );
    mSyn->SetMainPower( mSynPwr );
    *actHz = (double)( mSyn->SetFrequency( (long long)targetHz ) );

    // TODO - error on fail to lock 
    
    mSynPwr = 0;
    delta   = 100;
    while( mSynPwr < 4 ){
       attenDb  = mLbrd->GetMaxAttenDb();
       mLbrd->SetAttenDb( attenDb );
       mSyn->SetMainPower( mSynPwr );

       // Iterate on atten until we are within atten setting or too many
       // passes or until our attenuation goes negative
       int pass;
       pass = 0;
       dbm  = -100;
       while( ( delta*delta ) > (0.25*0.25)  && 
              (attenDb>=0)                   && 
              (attenDb<=mLbrd->GetMaxAttenDb())  &&
              pass<10  ){
          mLbrd->SetAttenDb( attenDb );
          us_sleep( 5000 );
          dbm   = GetPwrEst(mNest);
          delta = dbm - targetDbm;
          if( mLog&LGEN_LOG_SETOUTPUT ){
              printf("LOG: tgt=%f, dbm=%f, sp=%d, atn=%f, del=%f\n",
   		        targetDbm, dbm, mSynPwr, attenDb, delta);
          }
          attenDb += ( delta / 2 );
          if( attenDb < 0 ) attenDb = 0;
          pass++;
       }
       *actDbm = dbm;

       // Output power ok - just return with estimate
       if( (delta*delta)<(0.25*0.25) ){
           return(0);
       }

       // Output still too low, make another pass with higher power
       mSynPwr++;
       if( mLog&LGEN_LOG_SETOUTPUT ){
          printf("LOG: Increasing syn power to %d\n",mSynPwr);
       }
    }

    return(-1);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void usage( int exit_code )
{
    printf(
"This utility uses an M board and L board to output a specific \n"
"frequency at a specified level.  Sweeps in both level and frequency\n"
"can be done with the dwell time and step size configurable.\n"
"NOTE: The M board is assumed to be at I board port 1 and the L board\n"
"is assumed to be a port 0\n"
    );
    printf("-csv            Enable CSV output\n");
    printf("-startHz   <V>  Set sweep start to <V> Hertz\n");
    printf("-stopHz    <V>  Set sweep stop to <V>  Hertz\n");
    printf("-deltaDbm  <V>  Set sweep step to <V>  Hertz\n");
    printf("-startDbm  <V>  Set sweep start to <V> Dbm\n");
    printf("-stopDbm   <V>  Set sweep stop to <V>  Dbm\n");
    printf("-deltaDbm  <V>  Set sweep step to <V>  Dbm\n");
    printf("-dwell     <V>  Set sweep dwell time <V> microseconds\n");
    printf("Examples:\n");
    printf("    Set a single output to 100MHz at -10dBm                   \n");
    printf("        -startHz 100M -startDbm -10                           \n");
    printf("    Sweep 100MHz to 200MHz with default frequency step        \n");
    printf("    at -5dBm using 0.5Sec dwells                              \n");
    printf("        -startHz 100M -stopHz 200M -startDbm -5 -dwell 500000 \n");
    printf("    Sweep 0dBm to -10dBm using steps of 1 dBm                 \n");
    printf("    at 200MHz using 0.5Sec dwells                             \n");
    printf("        -startDbm 0 -stopDbm -10 -startHz 200M -dwell 500000  \n");
    exit( exit_code );
}

int
main( int argc, char *argv[] )
{
    int            idx;

    double         targetHz,actHz;
    double         targetDbm,actDbm;

    double         startHz,stopHz,deltaHz;
    double         startDbm,stopDbm,deltaDbm;

    int            err;
    long           dwellUs;
    int            csvFlag;

    // Configure level and frequency
    targetHz    = 100e6;
    startHz     = 100e6;
    stopHz      = 100e6;
    deltaHz     = 0.5e6;

    targetDbm   = -10.0;
    startDbm    = -10.0;
    stopDbm     = -10.0;
    deltaDbm    = -1.0;

    dwellUs     = 0; // 500000;
    csvFlag     = 0;

    // Parse command line arguments
    idx = 1;
    while( idx < argc ){

        if( 0==strcmp(argv[idx], "-help") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-h") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-csv") ){
            csvFlag = 1;
        }

        else if( 0==strcmp(argv[idx], "-startHz") ){
            if( (idx+1) >= argc ){ usage(-1); }
	    startHz = atof_suffix( argv[idx+1] );
            stopHz  = startHz;
	    idx+=2; continue;
        }

        else if( 0==strcmp(argv[idx], "-stopHz") ){
            if( (idx+1) >= argc ){ usage(-1); }
	    stopHz = atof_suffix( argv[idx+1] );
	    idx+=2; continue;
        }

        else if( 0==strcmp(argv[idx], "-deltaHz") ){
            if( (idx+1) >= argc ){ usage(-1); }
	    deltaHz = atof_suffix( argv[idx+1] );
	    idx+=2; continue;
        }

        else if( 0==strcmp(argv[idx], "-startDbm") ){
            if( (idx+1) >= argc ){ usage(-1); }
	    startDbm = atof_suffix( argv[idx+1] );
            stopDbm  = startDbm;
	    idx+=2; continue;
        }

        else if( 0==strcmp(argv[idx], "-stopDbm") ){
            if( (idx+1) >= argc ){ usage(-1); }
	    stopDbm = atof_suffix( argv[idx+1] );
	    idx+=2; continue;
        }

        else if( 0==strcmp(argv[idx], "-deltaDbm") ){
            if( (idx+1) >= argc ){ usage(-1); }
	    deltaDbm = atof_suffix( argv[idx+1] );
	    idx+=2; continue;
        }

        else if( 0==strcmp(argv[idx], "-dwell") ){
            if( (idx+1) >= argc ){ usage(-1); }
	    dwellUs = atof_suffix( argv[idx+1] );
        }

        // Move to next argument for parsing
        idx++;
    }

    Lgen lgen;
    lgen.Open();

    targetHz = startHz;
    while( targetHz <= stopHz ){

        targetDbm = startDbm;
        while( targetDbm >= stopDbm ){
            err = lgen.SetOutput( targetHz, targetDbm, &actHz, &actDbm );
            if( dwellUs > 0 ) us_sleep( dwellUs );
            if( csvFlag ){
               printf("CSV,%f,%f,%d\n",actHz, actDbm, err);
            }
            targetDbm += deltaDbm;
        }
        targetHz += deltaHz;
    }

    return(0);
}

