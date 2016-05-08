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

#include "InstModel.h"
#include "Device.h"

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
    mCenterHz   = 92e6;
    mSpanHz     = 10e6;
    mParamChange= 0;
    mCfgFname   = strdup( "local.cfg" );

    mXyMaxLen   = 8192;
    mXyCurLen   = 256;
    mXvec       = (double*)malloc( mXyMaxLen*sizeof(double) );
    mYvec       = (double*)malloc( mXyMaxLen*sizeof(double) );

    mXmin       = 1e6;
    mXmax       = 1e6;
    for( idx=0; idx<mXyCurLen; idx++){
       mXvec[ idx ] = mXmin + ((double)idx/2048.0)*(mXmax-mXmin);
       mYvec[ idx ] = -40 + idx%10; 
    }

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
    printf("InstModel::ReadCfg:Reading configuration file\n");
    printf("InstModel::ReadCfg:Configuration file read complete\n");

    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
int
InstModel::WriteCfg()
{
    printf("InstModel:WriteCfg:Enter\n");
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
          mCenterHz = atof( value );
    }

    else if( 0==strcmp(name,"spanHz") ){
          mSpanHz = atof( value );
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
                    "\"centerHz\" : %d,"
                    "\"spanHz\"   : %d "
                    ,
                    mRun?"ON":"OFF",            // run
                    timeStr,                    // time
                    mXyCurLen,                  // nPts
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

    int npc;
    static int npi = 0; // FIXME

#   define MAX_NPTS_PER_GET 512
    npc = MAX_NPTS_PER_GET;
    npi = (npi+npc)%mXyCurLen;

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
    nBytes = snprintf(pos, len, "\"lim\":[%g,%g],",
                                    mXmin,mXmax);
    pos += nBytes;
    len -= nBytes;

    nBytes = snprintf(pos, len, "\"npx\":[");
    pos += nBytes;
    len -= nBytes;

    for(idx=0;idx<npc;idx++){
        nBytes = snprintf(pos, len, "%g%c",mXvec[npi+idx],
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
        nBytes = snprintf(pos, len, "%g%c",mYvec[npi+idx],
                                           (idx==(npc-1))?' ':',');
        pos += nBytes;
        len -= nBytes;
    }

    nBytes = snprintf(pos, len, "]"); // End of npy
    pos += nBytes;
    len -= nBytes;

    nBytes = snprintf(pos, len, "}"); // End of main obj
    pos += nBytes;
    len -= nBytes;

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

////////////////////////////////////////////////////////////////////////////////
void InstModel::ScanReset()
{
}

////////////////////////////////////////////////////////////////////////////////
void InstModel::ScanStep()
{
    // GetDev()->SetRfInputFreq( mNextFreqHz );
    // Flush any samples
}

////////////////////////////////////////////////////////////////////////////////
void InstModel::ScanSvc()
{
    if( mScanReset ){
        ScanReset();
    }

    // Collect the samples for current step
    // idx = 0;
    // while( idx<mSampleCnt ){
    //      GetDev()->Get2kSamples( mSamples+idx );
    //    idx+=2048;
    // }

    // Step the scan
    // ScanStep();


    // Get the current spectral estimate
    // mSiEstimator( mSamples, mSampleCnt, m2Data );

    // Update the current results
}


