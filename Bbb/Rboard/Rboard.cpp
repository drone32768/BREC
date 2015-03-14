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
//
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/time.h>

#include "../Util/gpioutil.h"
#include "Rboard.h"

#define MAX_RDEV 2
static int gnInitialized        = 0;
static GpioUtil gClkIo;
static GpioUtil gTxIo;
static GpioUtil gCsIo[ MAX_RDEV ];

//----------------------------------------------------------------------------
/**
 * Internal method to initialize internal file descriptor state for 
 * the gpio's to be used for device and place spi signals in initial
 * state.
 */
static int p126_init()
{
    if( gnInitialized ) return( 0 );

    gClkIo.Open(   68   );
    gTxIo.Open(    69   );
    gCsIo[0].Open( 4    );
    gCsIo[1].Open( 5    );

    gClkIo.Set(0);

    gCsIo[0].Set( 1 );
    gCsIo[1].Set( 1 );

    gnInitialized = 1;

    return( 0 );
}

//----------------------------------------------------------------------------
/**
 * This static method takes two timeval's and returns the difference
 * in signed integer microseconds.  First arg is latest time, second is 
 * earlier.
 * TODO - use common version
 */
static int p126_delta_usec( struct timeval *tv2, struct timeval *tv1 )
{
    int usec;

    usec  = 1000000*(tv2->tv_sec - tv1->tv_sec);
    usec += (tv2->tv_usec - tv1->tv_usec);
    return(usec);
}

//----------------------------------------------------------------------------
static int p126_spi_delay()
{
    struct timeval tv2, tv1;
    int dus;
    int cnt;

    cnt =0;
    gettimeofday( &tv1, NULL );
    do{
        gettimeofday( &tv2, NULL );
        dus = p126_delta_usec( &tv2, &tv1 );
	cnt++;
    }while( dus < 100 );
    //printf("delay cnt=%d\n",cnt);
    return(0);
}

//----------------------------------------------------------------------------
static int p126_spi_clockbit( int bv )
{

    gTxIo.Set(bv);

    p126_spi_delay();

    gClkIo.Set(1);

    p126_spi_delay();

    gClkIo.Set(0);

    p126_spi_delay();
    return(0);
}

//----------------------------------------------------------------------------
static int p126_spi_write( int dev, int log, uint16_t word )
{
    int nb;

    if( log ){
        printf("rboard write dev=%d reg=0x%04x\n",dev,word);
    }

    gClkIo.Set(0);
    gCsIo[dev].Set( 0 );

    p126_spi_delay();
    for(nb=0;nb<16;nb++){
        p126_spi_clockbit( (word&0x8000)?1:0 );
        word = (word<<1);
    }

    p126_spi_delay();

    gCsIo[dev].Set( 1 );
    gClkIo.Set(0);

    return(0);
}

//----------------------------------------------------------------------------
Rboard::Rboard()
{
    mDevNum      = -1;
    mIsOpen      = 0;
    mAttenHalfDb = 40;
    //mLog         = 0xffffffff;
    mLog         = 0x0;
}

//----------------------------------------------------------------------------
void Rboard::SetDev( int dn )
{
    mDevNum = dn;
}

//----------------------------------------------------------------------------
int Rboard::Open()
{
    int err;

    if( mLog & RBRD_LOG_WRITES ){
       printf("### Initializing p126() ########################\n");
    }
    err = p126_init();
    if( err ){
        fprintf(stderr,"p126_init() failed\n");
    }
    mIsOpen = 1;

    SetChannel( 0 );

    return(0);
}

//----------------------------------------------------------------------------
int Rboard::SetChannel( int cn )
{
    int      err;
    uint16_t bits;

    printf("Rboard:SetChannel %d\n",cn);

    /*
     * NOTE: When channels other than 1 (atten) are selected the T bits
     * should be 0 (this prevents the shift registers from trying to pull
     * the outputs high while the device is powered down).
     *
     * NOTE: When channel 1 (atten) is first enabled we select a parallel
     * attenuation setting other than all 1's or all 0's for initialization
     * and power up.  We then go back, after 500uS and set the last set
     * attenuation value
     */
    switch( cn ){
        case 0: 
            bits = (1<<8) | (3<<4) | (0<<2); // w e=0001 a=11(rf4) b=10(rf1)
            break;
        case 1:
            bits = (8<<8) | (2<<4) | (1<<2); // x e=1000 a=01(rf3) b=00(rf2)
	    bits = bits | 0x1000; // Set lowest tbit
            break;
        case 2:
            bits = (4<<8) | (1<<4) | (2<<2); // y e=0100 a=00(rf2) b=01(rf3)
            break;
        case 3:
        default:
            bits = (2<<8) | (0<<4) | (3<<2); // z e=0010 a=10(rf1) b=11(rf4)
            break;
    }

    // Save the bits to be programmed
    mWord = bits;

    // Apply the parallel settings
    err = p126_spi_write(mDevNum,0,mWord);

    // Save the current channel
    mCurCh = cn;

    // If we just enabled channel 1 (atten) then reset the attenuation value
    if( 1==mCurCh ){
        SetAtten( mAttenHalfDb );
    }

    return( err );
}

//----------------------------------------------------------------------------
int Rboard::SetAtten( int halfDb )
{
    int      err;
    uint16_t tbits;

    printf("Rboard:SetAtten %d (x0.5dB)\n",halfDb);

    // Save and clamp the setting
    mAttenHalfDb = halfDb;
    if( mAttenHalfDb < 0 )  mAttenHalfDb = 0;
    if( mAttenHalfDb > 63 ) mAttenHalfDb = 63;

    // Convert the setting to actual digital parallel line value
    tbits = 0x3f & ( ~mAttenHalfDb );

    // If the current channel is not 1 (atten) then nothing else to do
    if( 1!=mCurCh ){
        return( 0 );
    }

    printf("Rboard:SetAtten tbits=%d\n",tbits);

    // Set Atten value
    mWord = mWord & ~0xf003; 
    mWord = mWord | (tbits&0x0f)<<12;
    mWord = mWord | (tbits&0x30)>>4;
    err = p126_spi_write(mDevNum,0,mWord);

    return( err );
}

//----------------------------------------------------------------------------
int Rboard::SetW( int w )
{
    int err;
    mWord = mWord & ~0x00C0; 
    mWord = mWord | (w&0x2)<<6;
    mWord = mWord | (w&0x1)<<7;
    err = p126_spi_write(mDevNum,0,mWord);
    return( err );
}

//----------------------------------------------------------------------------
int Rboard::WriteRaw( uint16_t word )
{
    int err;

    mWord = word;
    err = p126_spi_write(mDevNum,1,mWord);
    return( err );
}

