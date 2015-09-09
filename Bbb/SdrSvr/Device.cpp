//
//
// This source code is available under the "Simplified BSD license".
//
// Copyright (c) 2013, J. Kleiner
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
///
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <ifaddrs.h>
#include <sys/time.h>
#include <math.h>

#include "Device.h"

#   define IF0 1575420000
#   define IF1 10625000

////////////////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////////////////
Device::Device()
{
    mThreadExit       = 0;
    mComplex          = 1;
    mDisplayCount     = 0;

    mSwapIQ           = 0;
    mPmTuneMHz        = 0;
    mXboard           = 0;
    mHboard           = 0;
    mAboard           = 0;

    if( FindCapeByName("brecA") ){
        mAboard = 1;
        mAdc = new Aboard();
        mAdc->Open();
        mAdc->StartPrus();

        Bboard *bcBoard;
        bcBoard = new Bboard();
        bcBoard->Open();
        mLo0 = bcBoard->GetAdf4351( 0 );
        mLo1 = bcBoard->GetAdf4351( 1 );
    }

    if( FindCapeByName("brecH") ){
        mHboard = 1;

        mAdc = new Hboard();
        mAdc->Open();

	// NOTE: this is just the default.
	// actual value will be set by sdr control sw
        mAdc->SetComplexSampleRate(1250000);

        Iboard        *iBoard;
        Gpio6PinGroup *g6pg;
        Mboard        *mBoard;

        iBoard = new Iboard();
        iBoard->Open();

        g6pg = iBoard->AllocPort( 2 );
        iBoard->EnablePort( 2, 1 );

        mBoard = new Mboard();
        mBoard->Open( g6pg );
        mLo0 = mBoard->GetAdf4351( 2 );

        g6pg = iBoard->AllocPort( 1 );
        iBoard->EnablePort( 1, 1 );

        mBoard = new Mboard();
        mBoard->Open( g6pg );
        mLo1 = mBoard->GetAdf4351( 0 );
    }

    if( FindCapeByName("brecX") ){
        mXboard = 1;
        mAdc = new Xboard();
        mAdc->Open();
        mAdc->StartPrus();
        mAdc->SetComplexSampleRate( 5000000 );


        // Tuning will reset this
        // 2048 * 640k / 5000k = 262.144
        ((Xboard*)mAdc)->SetLoFreq( 262 );

        // Set the source to be IQ signed 16 bit
        ((Xboard*)mAdc)->SetSource ( 7 );

        Iboard        *iBoard;
        Gpio6PinGroup *g6pg;
        Mboard        *mBoard;

        iBoard = new Iboard();
        iBoard->Open();

        g6pg = iBoard->AllocPort( 0 );
        iBoard->EnablePort( 0, 1 );

        mBoard = new Mboard();
        mBoard->Open( g6pg );
        mLo0 = mBoard->GetAdf4351( 0 );

        g6pg = iBoard->AllocPort( 1 );
        iBoard->EnablePort( 1, 1 );

        mBoard = new Mboard();
        mBoard->Open( g6pg );
        mLo1 = mBoard->GetAdf4351( 1 );
    }

    mLo0->SetFrequency( IF0 + 93100000 );
    mLo0->SetAuxEnable( 0 );
    mLo0->SetMainPower( 2 );
    mLo0->SetCpCurrent( 15 );

    mLo1->SetFrequency( IF0 + IF1 );
    mLo1->SetAuxEnable( 0 );
    mLo1->SetMainPower( 2 );
    mLo1->SetCpCurrent( 15 );

    mNLO = 1;
}

//------------------------------------------------------------------------------
int Device::TunerSet( long long freqHz )
{
    printf("Device::TunerSet( %f Hz )\n",(double)freqHz);

    // X board w/o mixer is a special case.  Integrated nco
    if( mXboard ){
        int pinc;
        int hzMod;
        // TODO integrate this better with device model

        // Fout = ( pinc / 2^16 )*Fsamp

        hzMod= freqHz % 5000000;
        // pinc = 2048 * hzMod / 5000000;
        pinc = (double)2048 * (double)hzMod / (double)5000000;
        printf("XboardSdr : hzMod = %d, pinc=%d\n",hzMod,pinc);
        ((Xboard*)mAdc)->SetLoFreq( pinc );

        return( 0 );
    }

    switch( mNLO ){

       case 1:{
          freqHz   = freqHz + IF1;
          mLo0->SetFrequency( freqHz );
          mPmTuneMHz = (double)freqHz / 1e6;
          printf("*** TunerSet *** LO 0 set to %f Hz\n",(double)freqHz);
	  break;
       }

       case 2:
       default:{
          freqHz   = freqHz + IF1;
          mLo0->SetFrequency( freqHz );
          mPmTuneMHz = (double)freqHz / 1e6;
          printf("LO 0 set to %f Hz\n",(double)freqHz);
	  break;
       }

    }// End of switch over mNLO

    return(0);
}

//------------------------------------------------------------------------------
void Device::RcvEvent( char *evtStr )
{
    char *cmdStr;
    char *argStr;
    char *tokr;

    printf("Device   : rcv <%s>\n",evtStr);

    cmdStr = strtok_r( evtStr, " ", &tokr );
   
    if( 0==strcmp("dev-complex",cmdStr) ){
       mComplex = 1;
    }

    if( 0==strcmp("dev-real",cmdStr) ){
       mComplex = 0;
    }

    if( 0==strcmp("sample-rate",cmdStr) ){
       int sampleRate;	      

       argStr     = strtok_r(NULL, " ", &tokr);
       sampleRate =  atoi( argStr );
       printf("sample rate = %d\n",sampleRate);

       mAdc->SetComplexSampleRate( sampleRate );
    }

    if( 0==strcmp("d2",cmdStr) ){
       argStr = strtok_r(NULL, " ", &tokr);
       mDisplayCount =  atoi( argStr );
    }

    if( 0==strcmp("tune-hz",cmdStr) ){
       long long freq; 	     
      
       argStr = strtok_r(NULL, " ", &tokr);
       freq   = strtoll( argStr, NULL, 10 );

       TunerSet( freq );
    }    

}

//------------------------------------------------------------------------------
void Device::Main()
{
    while( !mThreadExit ){
        if( !mLo0->GetLock() ) mPmLock = 0;
	us_sleep( 100000 );
    }
}

//------------------------------------------------------------------------------
void Device::PmReset()
{
    mPmPause = 0;
    mPmLock  = 1;
}

//------------------------------------------------------------------------------
int Device::PmPauseCount()
{
    return( mPmPause );
}

//------------------------------------------------------------------------------
int Device::PmLockStatus()
{
    return( mPmLock );
}

//------------------------------------------------------------------------------
double Device::PmTuneMHz()
{
    return( mPmTuneMHz );
}

//------------------------------------------------------------------------------
void Device::GetSamples_A( short *sampPtr, int nComplexSamples )
{
    static unsigned int runningSampleCount;
    int                 complexSampleCount;

    // TODO - move these into class
    static short        *eo;
    static short        bf[2048];
    static int          bfSamps=0;


    // Process samples
    complexSampleCount = 0;
    while( complexSampleCount < nComplexSamples ){

        // Get 2k samples to internal buffer
        if( bfSamps<=0 ){
            mPmPause += mAdc->Get2kSamples( bf );
            eo      = bf;
            bfSamps = 2048;
        }

        // Complex sample output
        if( mComplex ){

/*
Smpl  Sin Fsr/4  Dncvt    Filter                                     Dec
==============================================================================
S0       0         0      A0*0 ....
S1       1        S1      A0*S1    + A1*0 ....
S2       0         0      A0*0     + A1*S1    +A2*0 ...               
S3      -1       -S3      A0*(-S3) + A1*0     +A2*S1     + A3*0   
------------------------------------------------------------------------------
S4       0         0      A0*0     + A1*(-S3) + A2*0     + A3*S1     O    Q1
S5       1        S5      A0*A5    + A1*0     + A2*(-S3) + A3*0      X
S6       0         0      A0*0     + A1*(-S5) + A3*0     + A3*(-S3)  O    Q2
S7      -1       -S7      A0*(-S7) + A1*0     + A2*S5    + A3*0      X

Smpl  Cos Fsr/4  Dncvt    Filter                                     Dec
==============================================================================
S0       1        S0      A0*S0 ...
S1       0         0      A0*0     + A1*S0 ....
S2      -1       -S2      A0*(-S2) + A1*0     +A2*S0 ...               
S3       0         0      A0*0     + A1*(-S2) +A2*0      + A3*S0   
------------------------------------------------------------------------------
S4       1        S4      A0*S4    + A1*0     + A2*(-S2) + A3*0      E    I1
S5       0         0      A0*0     + A1*S4    + A2*0     + A3*(-S2)  X
S6      -1       -S6      A0*(-S6) + A1*0     + A3*S0    + A3*0      E    I2
S7       0         0      A0*0     + A1*(-S6) + A2*0     + A3*S0     X

*/
            // filter first real on odds
            if( runningSampleCount&1 ) eo[0] = -eo[0];
            sampPtr[0] = miF3.Filter(1,eo[0]);

            // filter second real on evens
            if( runningSampleCount&1 ) eo[1] = -eo[1];
            sampPtr[1] = mqF3.Filter(0,eo[1]);

	    if(mSwapIQ) {
  	        short tmp = sampPtr[0];
	        sampPtr[0] = sampPtr[1];
	        sampPtr[1] = tmp;
	    }
        }

        // Real sample output
        else{
            sampPtr[0] = eo[0];
            sampPtr[1] = eo[1];
        }

        if( mDisplayCount > 0 ){
            printf("%d,%d\n", sampPtr[0], sampPtr[1] );
            mDisplayCount--;
        }

        // update sample count and buffer location
        runningSampleCount++;
        sampPtr+=2;
        complexSampleCount++;

        // Update internal buffer
        eo     +=2;
        bfSamps-=2;
    }
}

//------------------------------------------------------------------------------
void Device::GetSamples_H( short *sampPtr, int nComplexSamples )
{
    static unsigned int runningSampleCount;
    int                 complexSampleCount;

    // TODO - move these into class
    static short        *eo;
    static short        bf[2048];
    static int          bfSamps=0;

    // Process samples
    complexSampleCount = 0;
    while( complexSampleCount < nComplexSamples ){

        // Get 2k samples to internal buffer
        if( bfSamps<=0 ){
            mPmPause += mAdc->Get2kSamples( bf );
            eo      = bf;
            bfSamps = 2048;
        }

        // Complex sample output
        if( mComplex ){

            // sampPtr[0] = eo[0]>>4;
            // sampPtr[1] = eo[1]>>4;
	    if( runningSampleCount&1 ){ 
                sampPtr[0] = -eo[0];
                sampPtr[1] = -eo[1];
	    }
	    else{
                sampPtr[0] = eo[0];
                sampPtr[1] = eo[1];
            }		     
        }

        // Real sample output
        else{
            sampPtr[0] = eo[0];
            sampPtr[1] = eo[1];
        }

        if( mDisplayCount > 0 ){
            printf("%d,%d\n", sampPtr[0], sampPtr[1] );
            mDisplayCount--;
        }

        // update sample count and buffer location
        runningSampleCount++;
        sampPtr+=2;
        complexSampleCount++;

        // Update internal buffer
        eo     +=2;
        bfSamps-=2;
    }
}

//------------------------------------------------------------------------------
void Device::GetSamples_X( short *sampPtr, int nComplexSamples )
{
    static unsigned int runningSampleCount;
    int                 complexSampleCount;

    // TODO - move these into class
    static short        *eo;
    static short        bf[2048];
    static int          bfSamps=0;

    // Process samples
    complexSampleCount = 0;
    while( complexSampleCount < nComplexSamples ){

        // Get 2k samples to internal buffer
        if( bfSamps<=0 ){
            mPmPause += mAdc->Get2kSamples( bf );
            eo      = bf;
            bfSamps = 2048;
        }
        sampPtr[0] = eo[0];
        sampPtr[1] = eo[1];

        // update sample count and buffer location
        runningSampleCount++;
        sampPtr+=2;
        complexSampleCount++;

        // Update internal buffer
        eo     +=2;
        bfSamps-=2;
    }
}

//------------------------------------------------------------------------------
//
// This method is expected to produce the specified number of complex 16 bit 
// samples at the location provided.  
// 
// It is periodically called and may pend waiting for samples.
//
// This is the standard device interface.  This simulation/test version
// rate limits to the specified sample rate and relies on GenSamples() to 
// actually produce the samples.
//
void Device::GetSamples( short *sampPtr, int nComplexSamples )
{
   // TODO better approach to multiple board sets.
   if( mXboard ) return( GetSamples_X(sampPtr,nComplexSamples) );
   if( mHboard ) return( GetSamples_H(sampPtr,nComplexSamples) );
   if( mAboard ) return( GetSamples_A(sampPtr,nComplexSamples) );
}

