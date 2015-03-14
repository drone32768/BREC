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
#include <math.h>
#include <sys/time.h>

#include "../Util/net.h"
#include "AdcDatIf.h"

AdcDatIf::AdcDatIf()
{
    mAdcSamplesToCapture = 32768;

    mThreadExit = 0;
    mIpPort     = 0;
    mIpStr      = NULL;
    mRun        = 0;
    mPktData    = (unsigned char*)malloc( 4096 );
    mAdcData    = (short*)malloc( mAdcSamplesToCapture*sizeof(short) );
}

//------------------------------------------------------------------------------
void AdcDatIf::RcvEvent( char *evtStr )
{
    char *cmdStr;
    char *argStr;
    char *tokr;

    printf("AdcDatIf : rcv <%s>\n",evtStr);
    cmdStr = strtok_r( evtStr, " ", &tokr );
   
    if( 0==strcmp("adc.dat.set-port",cmdStr) ){
       argStr = strtok_r(NULL, " ", &tokr);
       mIpPort =  atoi( argStr );
    }

    else if( 0==strcmp("adc.dat.set-host", cmdStr) ){
       argStr = strtok_r(NULL, " ", &tokr);
       mIpStr = strdup( argStr );
    }

    else if( 0==strcmp("adc.dat.run", cmdStr) ){
        mRun = 1;
    }

    else if( 0==strcmp("adc.dat.halt", cmdStr) ){
        mRun = 0;
    }
}

//------------------------------------------------------------------------------
void AdcDatIf::ConfigureHw()
{
    Dp()->Adc()->SetGain( 0 );
    // Dp()->Adc()->SetComplexSampleRate( 500000 );
    // Dp()->Adc()->SetComplexSampleRate( 625000 );
    // Dp()->Adc()->SetComplexSampleRate( 833333 );
    // Dp()->Adc()->SetComplexSampleRate( 1250000 );
    // Dp()->Adc()->SetComplexSampleRate( 2500000 );
    Dp()->Adc()->SetComplexSampleRate( 5000000 );
}

//------------------------------------------------------------------------------
void AdcDatIf::Main()
{
    UdpCln *ucl;

    ucl = UdpClnNew();
    while( !mThreadExit ){

        while( !mRun && !mThreadExit ){
            sleep(1);
        }

        UdpClnSetDst( ucl, mIpStr, mIpPort );
        printf("Adc Dat running. Send to %s %d\n",mIpStr,mIpPort);
        ConfigureHw();
        mSeqNum = 0;
        while( mRun && !mThreadExit ){
            mPauseCount=0;
            SendSamples( ucl ); 
            printf("AdcDatIf:Main:s=%5d, r=%4d KSPS, p=%d\n",
                  mSeqNum,
		  (int)(mRateEst/1000.0),
		  mPauseCount );
        }

    }
    UdpClnDelete(ucl);
}

//------------------------------------------------------------------------------
void AdcDatIf::AdcDataCapture()
{
    short *dst;

    // NOTE: we are just grabbing a snapshot so do not advance
    // to most current position, just pick up from where we last left off
    // Dp()->Adc()->FlushSamples();

    mAdcSamplesAvail = 0;
    dst              = mAdcData;
    while( mAdcSamplesAvail < mAdcSamplesToCapture ){
        mPauseCount += Dp()->Adc()->Get2kSamples( dst );
        mAdcSamplesAvail+=2048;
        dst+=2048;
    }
}

//------------------------------------------------------------------------------
int AdcDatIf::AdcDataSend( UdpCln *ucl )
{
    short          *src;
    unsigned short *shortBf;
    int             samplesSent;
    int             nb;

    unsigned int    realSampleRate;
    unsigned short  rsps[2];
    realSampleRate = 2 * Dp()->Adc()->GetComplexSampleRate();
    rsps[1]        = (realSampleRate>>16)&0xffff;
    rsps[0]        = (realSampleRate    )&0xffff;

    shortBf     = (unsigned short*)(mPktData);
    src         = mAdcData;
    samplesSent = 0;
    while( mAdcSamplesAvail > 0 ){

        // Copy 512 samples (1024 bytes) from adc capture buffer to packet
        memcpy( mPktData+12, src, 1024 );
  
        // Update position in capture buffer
        src             +=512;
        mAdcSamplesAvail-=512;

        // Construct the frame header in the packet
        shortBf[0] = 0xa5a5;   // Format indicator ( 6word hdr, lendian )
        shortBf[1] = 512;      // Number of real samples following header
        shortBf[2] = rsps[1];  // Real sample rate ms short
        shortBf[3] = rsps[0];  // Real sample rate ls short
        shortBf[4] = mSeqNum;
        shortBf[5] = 0;

        // Send the packet
        nb = UdpClnSend( ucl, mPktData, 1036 );    
        if( nb!=1036 ){
                fprintf(stderr,"snd error bytes=%d\n",nb);
        }	  
    
	// This optionally give time for sent packet to make it on wire
	// us_sleep( 80 );

        // Maintain continuous sequence number
        mSeqNum++;

        // Update the samples transmitted
        samplesSent+=512;

    }

    // Return the number of samples sent
    return(samplesSent);
}

//------------------------------------------------------------------------------
// Returns after approximately a second (will be invoked immediately again).
void AdcDatIf::SendSamples( UdpCln *ucl)
{
    struct timeval        tv1,tv2;
    int                   sampleCount = 0;
    int                   dus;

    // Establish starting point time
    dus = 0;
    gettimeofday( &tv1, NULL );

    // Capture and Send data until time has expired
    while( dus < 3000000 ){

        // Capture an adc data set
        AdcDataCapture();

        // Send the entire data set
        sampleCount+=AdcDataSend( ucl );

        // Update the timing and throughput data
        gettimeofday( &tv2, NULL );
        dus      = tv_delta_useconds( &tv2, &tv1 ); 

	// This optionally slows down the send rate
	// us_sleep( 100000 );
    }

    // Update the transmission rate estimate
    mRateEst = 1000000.0*(double)sampleCount / (double)dus;

    // printf("us = %d samples=%d sps=%d\n",
    //  		     dus, sampleCount, (int)mRateEst);
}

