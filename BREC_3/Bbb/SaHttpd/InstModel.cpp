//
// This source code is available under the "Simplified BSD license".
//
// Copyright (c) 2016, J. Kleiner
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, 
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright 
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the original author nor the names of its contributors 
//    may be used to endorse or promote products derived from this software 
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include "InstModel.h"
#include "Device.h"

////////////////////////////////////////////////////////////////////////////////
/*
                                  Tune
                                 mCurHz 
                                   ^ 
                                   |
               ----.---------------+----------------.-- 
              /    .               |                .  \
             /     .               |                .   \
            /      .               |                .    \
           /       .               |                .     \
          /        .               |                .      \
         /         .               |                .       \
        /          .               |                .        \
  --O-----|----|---|---|---|--|--|---|---|---|--|---|---|---|----O-------
    |              .                                .            |
    |              .           mHzPerStep           .            |
    |              .          mInBinsPerStep        .            |
    |               <--|---|--|--|---|---|---|--|-->             |
    |              .                                .            |
    |              .                                .            |
    |              .          mOutBinsPerStep       .            |
    |              x------x--------x--------x-------x            |
    |                     |        |                             |
    |                     |        |                             |
    |                     |<------>|                             |
    |                    mOutHzPerBin                            |
    |                                                            |
    |                                                            |
    |                      mSamplesPerStep AND                   |
    |                         (fft size)   AND                   |
    |<---------------------------------------------------------->|
*/


/**
 * This structure collects all of the critical step parameters together
 */
struct StepParams
{
   int    mChId;           // channel to use
   int    mSamplesPerStep; // samples collected at each step (and fft size)

   int    mInBinsPerStep;  // fft bins available for measure at each step
   double mHzPerStep;      // measured hz at each step

   int    mTotalSteps;     // number of steps per scan 
   double mHzStart;        // first step center frequency
   int    mOutBinsPerStep; // output bins collected at each step
   double mOutHzPerBin;    // Hz per output bin

   int    mCurStep;        // current step in total steps
   double mCurHz;          // current frequency center under measure

};
// TODO - revisit global instance
static StepParams gSp;

////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor for hardware model.  It initializes all of the internal
 * values.  The object must be started after it is created.
 */
InstModel::InstModel()
{
    int  idx;

    mLog        = 0x0; // 0xffffffff;

    mRun        = 0;
    mCenterHz   = 750e6;
    mSpanHz     = 1e6;
    mNewCenterHz= 750e6;
    mNewSpanHz  = 1500e6;

    mParamChange= 0;
    mCfgFname   = strdup( "local.cfg" );

    mXyMaxLen   = 8192;
    mXyCurLen   = 256;
    mXvec       = (double*)malloc( mXyMaxLen*sizeof(double) );
    mYvec       = (double*)malloc( mXyMaxLen*sizeof(double) );

    mMaxSamples = 32768;
    mSampleBf   = (short*)malloc( mMaxSamples*sizeof(short) );

    mXmin       = 1e6;
    mXmax       = 1e6;
    for( idx=0; idx<mXyCurLen; idx++){
       mXvec[ idx ] = mXmin + ((double)idx/2048.0)*(mXmax-mXmin);
       mYvec[ idx ] = -40 + idx%10; 
    }

    mStepMarker = 0;
}

////////////////////////////////////////////////////////////////////////////////
int
InstModel::SetCfg( const char *fname )
{
    free(mCfgFname);
    mCfgFname = strdup( fname ); 
    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
int  InstModel::ReadCfg()
{
#   define CFG_LINEBF_BYTES 512
    char     lineBf[CFG_LINEBF_BYTES];
    FILE    *fp;
    char    *name,*value;

    printf("InstModel::ReadCfg:Reading configuration file\n");

    fp = fopen(mCfgFname,"r");
    if( !fp ) return(-1);
   
    while( fgets(lineBf,(CFG_LINEBF_BYTES-1),fp) ){
       if( '#'==lineBf[0] ) continue;
       name = strtok(lineBf," \t");
       value= strtok(NULL," \t");
       if( !name || !value ) continue;

       if( 0==strcmp(name,"mCenterHz") ){
           mNewCenterHz = atof( value );
       }
       else if( 0==strcmp(name,"mSpanHz") ){
           mNewSpanHz = atof( value );
       }
    }

    fclose(fp);
    printf("InstModel::ReadCfg:Configuration file read complete\n");

    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
int
InstModel::WriteCfg()
{
    FILE *fp;
    printf("InstModel:WriteCfg:Enter\n");

    fp=fopen(mCfgFname,"w");
    if( !fp ) return(-1);

    fprintf(fp,"mCenterHz %f\n",mCenterHz);
    fprintf(fp,"mSpanHz   %f\n",mSpanHz);

    fclose(fp);

    printf("InstModel:WriteCfg:Exit\n");
    return(0);
}

////////////////////////////////////////////////////////////////////////////////
/**
 * This internal method dumps the internal model state to stdout
 */
void InstModel::ShowState()
{
    printf("InstModel::ShowState:mXyCurLen= %d\n",mXyCurLen);
}

////////////////////////////////////////////////////////////////////////////////
/**
 * This internal method initializes the hardware. It must be invoked
 * prior to conducting any operations on the model.
 */
int InstModel::HwInit()
{
    GetDev()->Open();
    return(0);
}

////////////////////////////////////////////////////////////////////////////////
/**
 * This is the internal method used to place the hardware in 
 * a quiescent state.
 */
void InstModel::HwStop()
{
}

////////////////////////////////////////////////////////////////////////////////
/**
 * This is a required interface of the McF class. No McF events are processed
 * in this application.
 */
void InstModel::RcvEvent( char *evtStr )
{
   printf("InstModel::RcvEvent rcv <%s>\n",evtStr);

   if( 0==strcmp("step-marker-on",evtStr) ){
       mStepMarker = 1;
   }
   else if( 0==strcmp("step-marker-off",evtStr) ){
       mStepMarker = 0;
   }
   else{
      // Nothing to do for unrecognized/unapplicable command
   }
}

////////////////////////////////////////////////////////////////////////////////
/**
 * This is the primary external interface to the model.  It is given
 * a single name/value pair (text strings). The model state is updated
 * based on the provided values.
 */
int
InstModel::SetState( char *name, char *value )
{
    int   err;

    printf("InstModel::SetState name=\'%s\',value=\'%s\'\n",name,value);

    // TODO - there really should be a lock around this method as well as
    //        the param change indicator elsewhere

    if( 0==strcmp(name,"run") ){
          if( 0==strcmp(value,"ON") ){
             mRun = 1;
          }
          else{
             mRun = 0; 
          }
    }

    else if( 0==strcmp(name,"nPts") ){
          int nPts;
          nPts = mXyCurLen;
          nPts = nPts * 2;
          if( nPts>4096 ) nPts = 256;
          mXyCurLen = nPts;
    }

    else if( 0==strcmp(name,"centerHz") ){
          mNewCenterHz = atof( value );
    }

    else if( 0==strcmp(name,"spanHz") ){
          mNewSpanHz = atof( value );
    }

    else if( 0==strcmp(name,"swreset") && 0==strcmp(value,"ON") ){
          exit(0);
    }

    else if( 0==strcmp(name,"savecfg") && 0==strcmp(value,"ON") ){

	  err = system("/bin/mount -o remount,rw /");
          if( -1 == err ){
              fprintf(stderr,"system command error [mount rw]\n");
          }

          WriteCfg();

	  err = system("/bin/mount -o remount,ro /");
          if( -1 == err ){
              fprintf(stderr,"system command error [mount ro]\n");
          }
    }

    else if( 0==strcmp(name,"hwreset") && 0==strcmp(value,"ON") ){
	  err = system("/sbin/shutdown -r now");    
          if( -1 == err ){
              fprintf(stderr,"system command error [shutdown -r]\n");
          }
    }

    else if( 0==strcmp(name,"shutdown") && 0==strcmp(value,"ON") ){
	  err = system("/sbin/shutdown -h now");    
          if( -1 == err ){
              fprintf(stderr,"system command error [shutdown -h]\n");
          }
    }

    mParamChange = 1;
    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
/**
 * This is one of the primary external interfaces to the model.  It places
 * the model state in a provided string.  The state is formated
 * as a JSON object.
 */
int
InstModel::GetState( char *resultsStr, int resultsLen )
{
    int    nBytes;
    char  *pos;
    int    len;
    time_t now;
    char   timeStr[128];

    time( &now );
    ctime_r( &now, timeStr );
    timeStr[ strlen(timeStr) - 1 ] = 0; // remove new line

    // Setup the output position and residual length
    pos = resultsStr;
    len = resultsLen;

    // Output the 
    nBytes = snprintf(pos, len, 
                "{ "
                    "\"savecfg\"  : \"OFF\","
                    "\"swreset\"  : \"OFF\","
                    "\"hwreset\"  : \"OFF\","
                    "\"shutdown\" : \"OFF\","
                    "\"run\"      : \"%s\","
                    "\"time\"     : \"%s\","
                    "\"nPts\"     : \"%d\","
                    "\"chnl\"     : \"%d\","
                    "\"centerHz\" : %d,"
                    "\"spanHz\"   : %d "
                    ,
                    mRun?"ON":"OFF",            // run
                    timeStr,                    // time
                    mXyCurLen,                  // nPts
                    gSp.mChId,                  // chnl id (debug)
                    (int)mCenterHz,             // centerHz
                    (int)mSpanHz                // spanHz
    );
    pos += nBytes;
    len -= nBytes;

    nBytes = snprintf(pos, len, "}"); // End of main obj
    pos += nBytes;
    len -= nBytes;

    return(0);
}

////////////////////////////////////////////////////////////////////////////////
/**
 * This is one of the primary external interfaces to the model.  It places
 * the model data in a provided string.  The data is formated
 * as a JSON object.
 */
int
InstModel::GetData( char *resultsStr, int resultsLen )
{
    int    nBytes;
    char  *pos;
    int    len;
    int    idx;
    double fval;
    int    npc;

    // NOTE: this index is status to maintain history
    // between outputs.  It could be moved to a class member var
    static int npi = 0; 

    // Since the number of points retrieved per GET request may not
    // be an integer multiple of the total number of points we need
    // to setup for that here
#   define MAX_NPTS_PER_GET 512
    if( (mXyCurLen - npi) >= MAX_NPTS_PER_GET ){
        npc = MAX_NPTS_PER_GET;
    }
    else{
        npc = mXyCurLen-npi;
    }

    // printf("\nnpi=%d npc=%d, mXyCurLen=%d\n",npi,npc,mXyCurLen);

    // Setup the output position and residual length
    pos = resultsStr;
    len = resultsLen;

    // Output the 
    nBytes = snprintf(pos, len, 
                "{ "
                    "\"tpc\" : %d,"      
                    "\"npc\" : %d,"      
                    "\"npi\" : %d,"    
                    ,
                    mXyCurLen,                  // total point count
                    npc,                        // new point count
                    npi                         // new point index
    );
    pos += nBytes;
    len -= nBytes;

    mXmin = mXvec[0];
    mXmax = mXvec[mXyCurLen-1]; 
    nBytes = snprintf(pos, len, "\"lim\":[%f,%f],",
                                    mXmin,mXmax);
    pos += nBytes;
    len -= nBytes;

    nBytes = snprintf(pos, len, "\"npx\":[");
    pos += nBytes;
    len -= nBytes;

    for(idx=0;idx<npc;idx++){
        fval = mXvec[npi+idx];
        if(!isnormal(fval)) fval=0.0;
        nBytes = snprintf(pos, len, "%f%c",fval,
                                           (idx==(npc-1))?' ':',');
        pos += nBytes;
        len -= nBytes;
    }

    nBytes = snprintf(pos, len, "],"); // End of npx
    pos += nBytes;
    len -= nBytes;

    nBytes = snprintf(pos, len, "\"npy\":[");
    pos += nBytes;
    len -= nBytes;

    for(idx=0;idx<npc;idx++){
        fval = mYvec[npi+idx];
        if(!isnormal(fval)) fval=0.0;
        nBytes = snprintf(pos, len, "%g%c",fval,
                                           (idx==(npc-1))?' ':',');
        pos += nBytes;
        len -= nBytes;
    }

    // Show points as transfered
    if( 0 ){
        for(idx=0;idx<npc;idx++){
           printf("%f %g\n",mXvec[npi+idx],mYvec[npi+idx]);
        }
    }

    nBytes = snprintf(pos, len, "]"); // End of npy
    pos += nBytes;
    len -= nBytes;

    nBytes = snprintf(pos, len, "}"); // End of main obj
    pos += nBytes;
    len -= nBytes;

    // Setup the index for next round of outputs
    npi = (npi+npc);
    if( npi>=mXyCurLen ){
       npi = 0;
    }

    return(0);
}

////////////////////////////////////////////////////////////////////////////////
/**
 * This is a required interface of the McF class.  It is the main entry
 * point of the thread associated with the object.  A dedicated thread
 * will invoke this entry point when the object is started.
 *
 * NOTE: Due to this being a seperate and independent thread:
 *  a) Parameters are copied from current state and applied
 *  b) Hardware mutex locking is used under run operations
 */
void  InstModel::Main()
{
 
    // Read and restore saved configuration
    ReadCfg();

    // Initialize the hw
    HwInit();

    // Main processing loop
    while( !mThreadExit ){
       if( mRun ){
          ScanSvc(); 
       } 
       else{
          us_sleep( 500000 );
       }
    } // End of main processing loop 
}

int
StepCfg( 
    int       cfg, 
    int     & chId, 
    int     & stepSamples,
    int     & stepInBins,
    double  & stepHz
)
{
    switch( cfg ){
        case 0:
            chId        = 5;
            stepSamples = 8192;
            stepInBins  = 0.8*stepSamples;
            stepHz      = 0.2e6 * stepInBins / stepSamples;
            return( 1 );
        case 1:
            chId        = 4;
            stepSamples = 4096;
            stepInBins  = 0.8*stepSamples;
            stepHz      = 4e6   * stepInBins / stepSamples;
            return( 1 );
        case 2:
            chId        = 3;
            stepSamples = 2048;
            stepInBins  = 0.4*stepSamples;
            stepHz      = 40e6  * stepInBins / stepSamples;
            return( 0 );
        default:
            return( 0 );
    }
}


////////////////////////////////////////////////////////////////////////////////
void
InstModel::ScanReset()
{
    int    stepLimit = 100;  // tunable
    int    tgtPts    = 1024; // tunable

    int    idx;
    int    cont;

    // Clamp to positive start
    if( (mCenterHz - (mSpanHz/2)) < 0 ){
        mSpanHz = mCenterHz*2;
    }

#define HzToMHz(f) ( (f)/1e6 )

    printf(
"----------------------------- ScanReset ------------------------------------\n"
    );
    printf("S:mCenterHz      =%15f MHz , mSpanHz     =%15f MHz\n",
             HzToMHz(mCenterHz), HzToMHz(mSpanHz) );

    // Mark the scan as reset
    mScanReset = 0;

    // Loop over possible step configurations find the "best" one
    cont      = 1;
    idx       = 0;
    gSp.mTotalSteps = 2*stepLimit;
    while( cont && (gSp.mTotalSteps>stepLimit) ){

        cont= StepCfg( idx,
                       gSp.mChId,
                       gSp.mSamplesPerStep,
                       gSp.mInBinsPerStep,
                       gSp.mHzPerStep );

        gSp.mTotalSteps = 1 + (mSpanHz / gSp.mHzPerStep);
        idx++;

    } // end of loop over step configurations

    // At this point the following parameters have been set
    printf("S:mChId          =%15d chl , mTotalSteps =%15d cnt\n",
            gSp.mChId,gSp.mTotalSteps);
    printf("S:mSamplesPerStep=%15d cnt , mHzPerStep  =%15f MHz\n",
            gSp.mSamplesPerStep, HzToMHz(gSp.mHzPerStep) );
    printf("S:mInBinsPerStep =%15d cnt,\n",
            gSp.mInBinsPerStep);


    // Calculate the final number of output bins and output bins per step
    if( gSp.mTotalSteps > 1 ){
        gSp.mOutBinsPerStep = tgtPts / gSp.mTotalSteps;
    }
    else{
        // gSp.mOutBinsPerStep = tgtPts;
        gSp.mOutBinsPerStep = gSp.mInBinsPerStep;
    }
    gSp.mOutHzPerBin    = gSp.mHzPerStep / gSp.mOutBinsPerStep;
    mXyCurLen           = gSp.mOutBinsPerStep * gSp.mTotalSteps; 

    printf("S:mOutBinsPerStep=%15d cnt , mOutHzPerBin=%15f  Hz\n",
             gSp.mOutBinsPerStep,
             gSp.mOutHzPerBin
    );

    // First step frequency is lower limit plus half a step size
    gSp.mHzStart = mCenterHz - (mSpanHz/2) + gSp.mHzPerStep/2;

    printf("S:mHzStart       =%15f cnt , mXyCurLen   =%15d MHz\n",
             HzToMHz(gSp.mHzStart),
             mXyCurLen
    );

    // Setup the stepping limits
    gSp.mCurStep = 0;
    gSp.mCurHz   = gSp.mHzStart;

    // Construct the frequency bin locations
    double fcHz,fbHz;
    int    didx,sidx,bidx;

    fcHz = gSp.mHzStart;
    didx = 0;
    sidx = 0;
    do{
       fbHz = fcHz -  (gSp.mOutBinsPerStep/2)*gSp.mOutHzPerBin;
       for(bidx=0;bidx<gSp.mOutBinsPerStep;bidx++){
           mXvec[didx] = fbHz;
           mYvec[didx] = -160.0;
           didx        = didx + 1;
           fbHz        = fbHz + gSp.mOutHzPerBin;
       }
       fcHz = fcHz + gSp.mHzPerStep;
       sidx = sidx + 1;
    }
    while( sidx < gSp.mTotalSteps );
    printf("S:X[%d]           =%15f MHz , X[%4d]     =%15f MHz\n",
                      0,(mXvec[0]/1e6),
                      didx-1,(mXvec[didx-1] /1e6)
    );

    GetDev()->SetChannel( gSp.mChId );
    GetDev()->SetTuneHz( gSp.mCurHz );
}

////////////////////////////////////////////////////////////////////////////////
void
InstModel::ScanSvc()
{
    ScanStep();
}

////////////////////////////////////////////////////////////////////////////////
void
InstModel::ShowXy()
{
    int idx;
    for( idx=0; idx<mXyCurLen; idx++){
        printf("%d %f %f\n",idx,HzToMHz(mXvec[idx]),mYvec[idx]);
    }
}

////////////////////////////////////////////////////////////////////////////////
void
InstModel::ScanStep()
{
    int    didx;
    double nextHz;
    int    nextStep;

    // Figure out if we need to reset
    if( (mNewCenterHz!=mCenterHz) || 
        (mNewSpanHz  !=mSpanHz  ) ){

        mCenterHz = mNewCenterHz;
        mSpanHz   = mNewSpanHz;

        ScanReset();
    }

    // Collect the required number of samples
    didx = 0;
    while( didx< (2*gSp.mSamplesPerStep) ){
        // Since these are really words we get 2x since a sample is complex
        GetDev()->Get2kSamples( mSampleBf + didx );
        didx += 2048;
    }

    // Tune to the next step
    if( gSp.mCurStep >= (gSp.mTotalSteps-1) ){
        nextHz   = gSp.mHzStart;
        nextStep = 0;
    }
    else{
        nextHz   = gSp.mCurHz + gSp.mHzPerStep;
        nextStep = gSp.mCurStep + 1;
    }

    if( gSp.mCurHz!=nextHz ){
       GetDev()->SetTuneHz( nextHz );
    }
    GetDev()->FlushSamples();

    // Get the estimate   
    mPse.GetEstimate( 
              mSampleBf,           // Samples
              gSp.mSamplesPerStep, // Number of complex input samples 
              gSp.mInBinsPerStep,  // Number of bins to evaluate
              gSp.mOutBinsPerStep, // Number of output bins
              mYvec + (gSp.mCurStep*gSp.mOutBinsPerStep)
    );

    // Place visual marker in each start bin of step
    if( mStepMarker ){
        int idx;
        idx =  (gSp.mCurStep*gSp.mOutBinsPerStep);
        mYvec[ idx ] = -160;
    }
              
    // Make the next step the current step
    // NOTE: last thing we do
    gSp.mCurHz   = nextHz;
    gSp.mCurStep = nextStep;
}
