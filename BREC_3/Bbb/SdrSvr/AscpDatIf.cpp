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
#include <math.h>
#include <sys/time.h>

#include "Util/net.h"
#include "AscpDatIf.h"

AscpDatIf::AscpDatIf()
{
    mThreadExit = 0;
    mIpPort     = 0;
    mIpStr      = NULL;
    mRun        = 0;
    mOutputData = (unsigned char*)malloc( 32768 );
    mDev        = NULL;
}

//------------------------------------------------------------------------------
void AscpDatIf::SetDevice( Device *dev )
{
    mDev = dev;
}

//------------------------------------------------------------------------------
void AscpDatIf::RcvEvent( char *evtStr )
{
    char *cmdStr;
    char *argStr;
    char *tokr;

    printf("AscpDatIf: rcv <%s>\n",evtStr);
    cmdStr = strtok_r( evtStr, " ", &tokr );
   
    if( 0==strcmp("ascp.dat.set-port",cmdStr) ){
       argStr = strtok_r(NULL, " ", &tokr);
       mIpPort =  atoi( argStr );
    }

    else if( 0==strcmp("ascp.dat.set-host", cmdStr) ){
       argStr = strtok_r(NULL, " ", &tokr);
       mIpStr = strdup( argStr );
    }

    else if( 0==strcmp("ascp.dat.run", cmdStr) ){
       mRun = 1;
    }

    else if( 0==strcmp("ascp.dat.halt", cmdStr) ){
       mRun = 0;
    }
}

//------------------------------------------------------------------------------
void AscpDatIf::Main()
{
    UdpCln *ucl;

    ucl = UdpClnNew();
    while( !mThreadExit ){

        while( !mRun && !mThreadExit ){
            sleep(1);
        }

        UdpClnSetDst( ucl, mIpStr, mIpPort );
        printf("AscpDatIf:Dat running. Send to %s %d\n",mIpStr,mIpPort);
        mSeqNum = 0;
        while( mRun && !mThreadExit ){
            mDev->PmReset();		 
            SendSamples( ucl ); 
            printf("AscpDatIf:Main:s=%5d, r=%4d KSPS, p=%d, l=%d, f=%f\n",
                  mSeqNum,
		  (int)(mRateEst/1000.0),
		  mDev->PmPauseCount(),
		  mDev->PmLockStatus(),
		  mDev->PmTuneMHz()
		  );
        }

    }
    UdpClnDelete(ucl);
}

//------------------------------------------------------------------------------
// Returns after approximately a second (will be invoked immediately again).
void AscpDatIf::SendSamples( UdpCln *ucl)
{
    struct timeval        tv1,tv2;
    int                   sampleCount = 0;
    int                   dus;
    int                   nb;

    dus = 0;
    gettimeofday( &tv1, NULL );
    while( dus < 3000000 ){
        mDev->GetSamples( (short*)(mOutputData+4), 256 );
        mOutputData[ 0 ] = 0x04;
        mOutputData[ 1 ] = 0x84;
        SetLeUint16( mOutputData + 2, mSeqNum );
        nb = UdpClnSend( ucl, mOutputData, 1028 );    
	if( nb!=1028 ){
            fprintf(stderr,"snd error bytes=%d\n",nb);
	}	  
        mSeqNum++;
        sampleCount+=256;

        gettimeofday( &tv2, NULL );
        dus      = tv_delta_useconds( &tv2, &tv1 ); 
    }

    mRateEst = 1000000.0*(double)sampleCount / (double)dus;
}

