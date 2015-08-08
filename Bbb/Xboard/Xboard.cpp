//
//
// This source code is available under the "Simplified BSD license".
//
// Copyright (c) 2015, J. Kleiner
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
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include "../Util/gpioutil.h"
#include "Xboard.h"

//------------------------------------------------------------------------------
#define XSPI_FIFO_EN     0x8000
#define XSPI_FIFO_RST    0x4000

#define XSPI_LED0        0x0100
#define XSPI_LED1        0x0200
#define XSPI_LED2        0x0400
#define XSPI_LED3        0x0800

#define XSPI_SEL_P1_CNTR 0x0000
#define XSPI_SEL_P1_FIFO 0x0010

#define XSPI_READ        0x0008
#define XSPI_WRITE       0x0004
#define XSPI_PORT1       0x0001
#define XSPI_PORT0       0x0000

#define XSPI_WP0   (XSPI_WRITE | XSPI_PORT0)
#define XSPI_RP1   (XSPI_READ  | XSPI_PORT1)

#define XSPI_STOP      (               XSPI_SEL_P1_FIFO | XSPI_LED0 | XSPI_WP0 )
#define XSPI_FLUSH     (XSPI_FIFO_RST| XSPI_SEL_P1_FIFO | XSPI_LED0 | XSPI_WP0 )
#define XSPI_START     (XSPI_FIFO_EN | XSPI_SEL_P1_FIFO | XSPI_LED1 | XSPI_WP0 )
#define XSPI_READ_SAMPLE   (  XSPI_RP1 )

//------------------------------------------------------------------------------
Xboard::Xboard()
{
   mCSPS = 0;
}

//------------------------------------------------------------------------------
int
Xboard::SetGain( int gn )
{
   if( gn!=0 ){
       fprintf(stderr,"Xboard:SetGain invoked w/o 0 - err \n");
   }
   return( 0 );
}

//------------------------------------------------------------------------------
int
Xboard::Open()
{
    printf("Xboard:Open\n");

    mIbrd = new Iboard();
    mIbrd->Open();
 
    mG6pg = mIbrd->AllocPort( 0 );
    mIbrd->EnablePort( 0, 1 );

    mUsHold = 1;
    mXspiDbg= 0;
    mG6pg->GetSs1()->Set( 1 ); us_sleep( mUsHold );
    mG6pg->GetSs1()->Set( 0 ); us_sleep( mUsHold );

    XspiWrite( XSPI_STOP );

    return(0);
}

//------------------------------------------------------------------------------
int
Xboard::Flush()
{
    return(0);
}

//------------------------------------------------------------------------------
int 
Xboard::FlushSamples()
{
    return(0);
}

//------------------------------------------------------------------------------
// NOTE this is only for test purposes, it will not support high sample rates
int
Xboard::GetSamplePair( short *eo )
{
   *eo = 0;
   return( 0 );
}

//------------------------------------------------------------------------------
int            localGetCount =0;
#define        FETCH_COUNT 2048
unsigned short localBf[FETCH_COUNT];
int
Xboard::Get2kSamples( short *bf )
{
    int cnt;


    if( 0==localGetCount ){
        printf("Xboard:Xboard::Get2kSamples Start\n");
        XspiWrite( XSPI_STOP );
        XspiWrite( XSPI_FLUSH );
        XspiWrite( XSPI_START );
        XspiWrite( XSPI_READ_SAMPLE );
        for( cnt=0; cnt<FETCH_COUNT; cnt++){
            localBf[ cnt ] = XspiWrite( XSPI_READ_SAMPLE );
        }
        localGetCount = 32768/FETCH_COUNT;
        printf("Xboard:Xboard::Get2kSamples End %d / %d\n",
                    FETCH_COUNT,localGetCount);
    }
    localGetCount--;
    for( cnt=0; cnt<FETCH_COUNT; cnt++){
        *bf = localBf[ cnt ];
         bf++;
    }


    return(0);
}

//------------------------------------------------------------------------------
int
Xboard::StartPrus()
{
    return( 0 );
}

//------------------------------------------------------------------------------
int
Xboard::SetSim( int sim )
{
    return( 0 );
}

//------------------------------------------------------------------------------
int
Xboard::SetComplexSampleRate( int complexSamplesPerSecond )
{
    mCSPS = complexSamplesPerSecond;
    printf("Xboard:SetComplexSampleRate %d CSPS\n",mCSPS);

    return(0);
}

//------------------------------------------------------------------------------
int
Xboard::GetComplexSampleRate()
{
    return( mCSPS );
}

//------------------------------------------------------------------------------
int Xboard::GetRms( int nSamples, short *aSamples, double *rrms )
{
    return( 0 );
}

//------------------------------------------------------------------------------
int
Xboard::XspiWrite( int wval )
{
    int rval;
    int obit,ibit,idx;

    if( mXspiDbg ){
       printf("xspi_write: wval = 0x%04x\n",wval);
    }

    rval    = 0;

    // expecting: sclk=0, ss=1
    mG6pg->GetSs1()->Set( 0 ); // us_sleep( mUsHold );

    for( idx=15; idx>=0; idx-- ){
        if( mXspiDbg ){
            printf("idx[%d]\n",idx);
        }

        if( wval&0x8000 ) obit = 1;
        else              obit = 0;

        if( mXspiDbg ){
            printf("   obit = %d\n",obit);
        }
        mG6pg->GetMoSi()->Set( obit ); // us_sleep( mUsHold );
        mG6pg->GetSclk()->Set( 1    ); // us_sleep( mUsHold );
        ibit = mG6pg->GetMiSo()->Get( );
        if( mXspiDbg ){
            printf("   ibit = %d\n",ibit);
        }

        mG6pg->GetSclk()->Set( 0 ); // us_sleep( mUsHold );

        rval = (rval<<1) | ibit;
        wval = (wval<<1);
    }

    // expecting: sclk=0, ss=0

    mG6pg->GetSs1()->Set( 1 ); // us_sleep( mUsHold );

    if( mXspiDbg ){
        printf("xspi_write: rval = 0x%04x\n",rval);
    }

    return(rval);
}
