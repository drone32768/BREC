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
#include "Lboard.h"
#include "../Util/mcf.h"          // for us_sleep
#include "../Util/SimpleTable.h"  

//----------------------------------------------------------------------------
// Internal forward decls.


//----------------------------------------------------------------------------
int Lboard::SpiDelay()
{
    struct timeval tv2, tv1;
    unsigned long long dus;
    int cnt;

    cnt =0;
    gettimeofday( &tv1, NULL );
    do{
        gettimeofday( &tv2, NULL );
        dus = tv_delta_nseconds( &tv2, &tv1 );
	cnt++;
    }while( dus < mSpiNsDelay );

    // printf("delay cnt=%d\n",cnt);
    return(0);
}

//----------------------------------------------------------------------------
int Lboard::SpiClockBitOut( int pn, int bv )
{
    mPinGrp->GetMoSi()->Set( bv );
    SpiDelay();
    mPinGrp->GetSclk()->Set( 1 );
    SpiDelay();
    mPinGrp->GetSclk()->Set( 0 );
    SpiDelay();
    return(0);
}

//----------------------------------------------------------------------------
int Lboard::SpiClockBitIn( int pn )
{
    int bn;

    mPinGrp->GetSclk()->Set( 1 );
    SpiDelay();

    bn = mPinGrp->GetMiSo()->Get();

    mPinGrp->GetSclk()->Set( 0 );
    SpiDelay();

    return(bn);
}

//----------------------------------------------------------------------------
int Lboard::SpiWrite( int pn, int log, uint32_t word, int nbits )
{
    int          nb;
    unsigned int mask;

    if( log ){
        printf("SpiWrite %d 0x%08x:\n",pn,word);
    }

    mPinGrp->GetSclk()->Set( 0 );
    if( 0==pn) {
        mPinGrp->GetSs1 ()->Set( 0 );
    }
    else{
        mPinGrp->GetStat ()->Set( 0 );
    }

    SpiDelay();

    mask = 1<<(nbits-1);
    for(nb=0;nb<nbits;nb++){
        SpiClockBitOut( pn, (word&mask)?1:0 );
        word = (word<<1);
    }
    SpiDelay();

    if( 0==pn) {
        mPinGrp->GetSs1 ()->Set( 1 );
        SpiDelay();
        mPinGrp->GetSs1 ()->Set( 0 ); // NOTE: PE4302 indicates pull low after
    }
    else{
        mPinGrp->GetStat ()->Set( 1 );
    }

    mPinGrp->GetSclk()->Set( 0 );

    return(0);
}

//----------------------------------------------------------------------------
int Lboard::SpiRead( int pn, int log, uint32_t *wword, int nbits )
{
    int nb;
    int bn;

    if( log ){
        printf("SpiRead %d nbits=%d:\n",pn,nbits);
    }

    mPinGrp->GetSclk()->Set( 0 );
    if( 0==pn) {
        mPinGrp->GetSs1 ()->Set( 0 );
    }
    else{
        mPinGrp->GetStat ()->Set( 0 );
    }

    SpiDelay();

    *wword = 0;
    for(nb=0;nb<nbits;nb++){ 
        *wword = (*wword<<1);
        bn = SpiClockBitIn( pn );
        *wword = *wword | bn;
    }

    SpiDelay();

    if( 0==pn) {
        mPinGrp->GetSs1 ()->Set( 1 );
    }
    else{
        mPinGrp->GetStat ()->Set( 1 );
    }

    mPinGrp->GetSclk()->Set( 0 );

    return(0);
}

//----------------------------------------------------------------------------
Lboard::Lboard()
{
    printf("Lboard::Lboard()\n");
    mIsOpen = false;
    mPinGrp = NULL;
    // mLog    =0xffffffff;
    mLog    =0x0;
}

static SimpleTable gSamplePwrDeltaTbl;

//----------------------------------------------------------------------------
int Lboard::Open( Gpio6PinGroup *g6pg )
{
    printf("Lboard::Open()\n");

    // Save pin group
    mPinGrp = g6pg;

    // NOTE: On this board IO2 = CS2 = pin group stat
    mPinGrp->GetStat()->SetDirInput( 0 );

    // Configure values of pins
    mPinGrp->GetSs1()->Set(  1 );
    mPinGrp->GetStat()->Set(  1 );
    mPinGrp->GetMoSi()->Set( 0 );
    mPinGrp->GetSclk()->Set( 0 );

    // Configure spi clocking delay
    mSpiNsDelay = 200;

    // Setup the sample calibration table for power meter
    gSamplePwrDeltaTbl.AddEntry(  10.0,  -7.9 );
    gSamplePwrDeltaTbl.AddEntry(  50.0,  -7.9 );
    gSamplePwrDeltaTbl.AddEntry( 100.0,  -8.0 );
    gSamplePwrDeltaTbl.AddEntry( 200.0,  -9.0 );
    gSamplePwrDeltaTbl.AddEntry( 400.0,  -9.6 );
    gSamplePwrDeltaTbl.AddEntry( 500.0, -10.2 );
    gSamplePwrDeltaTbl.AddEntry( 800.0, -11.6 );
    gSamplePwrDeltaTbl.AddEntry( 900.0, -12.0 );
    gSamplePwrDeltaTbl.AddEntry(1000.0, -12.3 );
    gSamplePwrDeltaTbl.AddEntry(1500.0, -13.3 );
    gSamplePwrDeltaTbl.AddEntry(2000.0, -13.9 );
    gSamplePwrDeltaTbl.AddEntry(2100.0, -14.0 );
    gSamplePwrDeltaTbl.AddEntry(2500.0, -14.4 );
    gSamplePwrDeltaTbl.AddEntry(3000.0, -15.3 );
    gSamplePwrDeltaTbl.AddEntry(3500.0, -17.2 );
    gSamplePwrDeltaTbl.Print();

    // Depending configuration...
    if( 1 ){
       mPwrSample    = 0;
       mMvPerDbm     =  32.5;
       mDbmIntercept = -43.0;
    }
    else{
       mPwrSample    = 1;
       mMvPerDbm     =  32.0;
       mDbmIntercept = -42.0;
    }

    return( 0 );
}

//----------------------------------------------------------------------------
unsigned int Lboard::SetLog( unsigned int lg )
{
   unsigned int prev;
   prev = mLog;
   mLog = lg;
   return( prev );
}

//----------------------------------------------------------------------------
float Lboard::GetMaxAttenDb()
{
    return( 31.5 );
}

//----------------------------------------------------------------------------
int Lboard::SetAttenDb( float attenDb )
{
    int          err;
    unsigned int word;

    word = (attenDb*2);
    word = word & 0x3f;

    if( mLog & LBRD_LOG_API ){
       printf("Lboard::SetAttenDb dB=%f w=%08x\n",attenDb, word);
    }

    err = SpiWrite(0, mLog&LBRD_LOG_ATTEN, word, 6 );
    return( err );
}

//----------------------------------------------------------------------------
float Lboard::GetPwrCode()
{
    int          err;
    float        dBm;
    unsigned int word;

    err = SpiRead(1, mLog&LBRD_LOG_METER, &word, 12 );
    if( err ) word = -1;

    // NOTE: No conversion, just raw adc code
    dBm = word;

    return( dBm );
}

//----------------------------------------------------------------------------
float Lboard::GetPwrDbm()
{
    return( GetPwrDbm( 500e6 ) );
}

//----------------------------------------------------------------------------
float Lboard::GetPwrDbm( float hintHz )
{
    int          err;
    float        dBm,mV;
    unsigned int word;

    err = SpiRead(1, mLog&LBRD_LOG_METER, &word, 12 );
    if( err ) word = -1;

    // Convert 12 bit ADC code to mV (assume 3.3V Vref)
    mV  = word * 3300 / 4096;

    // Convert from mV to dBm
    dBm = mDbmIntercept + mV / mMvPerDbm;

    // If we are sampling, compensate for attenuation of sampler
    if( mPwrSample ){
        dBm -= gSamplePwrDeltaTbl.Interp( hintHz/1e6 );
    }

    if( mLog & LBRD_LOG_API ){
        printf("Lboard::GetPwrDbm Hz=%f, code=%d, dBm=%f\n",hintHz,word,dBm);
    }

    return( dBm );
}


