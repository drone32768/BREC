//
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
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include "Fboard/Fboard.h"
#include "Ddc100.h"

#include "prussdrv.h"
#include "pruss_intc_mapping.h"
#include "pru_images.h"

/**
 * NOTE: The interfaces here are not mutex safe.  Specifically the
 * get samples vs the flush.  In the case of I/Q data, it may misalign
 * the I/Q samples to I(n),Q(n+1) depending on when the flush occurs
 * relative to sample extraction.
 *
 * All callers must ensure mutex.
 *
 */

////////////////////////////////////////////////////////////////////////////////
/// Hardware definitions ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define XSPI_FIFO_EN     0x8000
#define XSPI_FIFO_RST    0x4000

#define XSPI_LED0        0x0100
#define XSPI_LED1        0x0200
#define XSPI_LED2        0x0400
#define XSPI_LED3        0x0800

#define XSPI_SEL_P1_CNTR 0x0000
#define XSPI_SEL_P1_FIFO 0x0010
#define XSPI_SEL_P1_R3   0x0030
#define XSPI_SEL_P1_R6   0x0060
#define XSPI_SEL_P1_R7   0x0070
#define XSPI_SEL_P1_R8   0x0080
#define XSPI_SEL_P1_R9   0x0090

#define XSPI_READ        0x0008
#define XSPI_WRITE       0x0004
#define XSPI_PORT1       0x0001
#define XSPI_PORT0       0x0000

#define XSPI_RP0         (XSPI_READ  | XSPI_PORT0)
#define XSPI_WP0         (XSPI_WRITE | XSPI_PORT0)
#define XSPI_RP1         (XSPI_READ  | XSPI_PORT1)
#define XSPI_WP1         (XSPI_WRITE | XSPI_PORT1)

#define XSPI_STOP        (               XSPI_SEL_P1_FIFO |  XSPI_WP0 )
#define XSPI_FLUSH       (XSPI_FIFO_RST| XSPI_SEL_P1_FIFO |  XSPI_WP0 )
#define XSPI_START       (XSPI_FIFO_EN | XSPI_SEL_P1_FIFO |  XSPI_WP0 )
#define XSPI_READ_SAMPLE (  XSPI_RP1 )

// NOTE: Use this start to collect counter values rather than fifo samples
// this is usefull for testing the full path as a continuous counter is 
// provided allowing the pattern to be checked
// #define XSPI_START  (XSPI_FIFO_EN | XSPI_SEL_P1_CNTR | XSPI_LED2 | XSPI_WP0 )

#define GetSramWord( off )   ( *(unsigned int*  )((mPtrPruSram + (off))) )
#define SetSramWord( v,off ) ( *(unsigned int*  )((mPtrPruSram + (off))) = (v) )
#define GetSramShort( off )  ( *(unsigned short*)((mPtrPruSram + (off))) )

////////////////////////////////////////////////////////////////////////////////
/// External Methods ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
Ddc100::Ddc100()
{
    // 0 = no test pattern
    // 2 = 1/8 Fs at half scale
    // 4 = 1/4 Fs at +/- 4
    mTpg     = 4; 
    mFifoSrc = 0;
    mFsHz    = 10000000; // Function of the board
    mCSPS    = mFsHz;    // Function of board and channel selected
}

//------------------------------------------------------------------------------
void
Ddc100::Attach( Bdc *bdc )
{
   mBdc = bdc;
}

//------------------------------------------------------------------------------
int
Ddc100::Open()
{
    int ret;
    unsigned short fwVer;

    printf("Ddc100:Open enter\n");

    mBdc->Show( "Ddc100::Open" );

    fwVer = GetFwVersion();
    printf("Ddc100::Open::fw ver = 0x%08x\n",fwVer);
    if( 0x0700 == (fwVer&0xff00) ){
       mFsHz    = 40000000; // Function of the board
       mCSPS    = mFsHz;    
    }
    else{
       mFsHz    = 40000000; // Function of the board
       mCSPS    = mFsHz;    
    }
    printf("Ddc100::Open::FsHz        = %d\n",mFsHz);
    printf("Ddc100::Open::CSPS        = %d\n",mCSPS);

    // Set startup default signal paramters
    SetLoFreqHz( 640000 );

    SetSource( 0 ); 
    SetTpg( 1 );  // FIXME - testing only
    SetChannelMatch(0,1.0,0,1.0);

    printf("Ddc100:Open exit\n");

    return(0);
}

//------------------------------------------------------------------------------
int
Ddc100::SetChannelMatch( int Ioff, double Igain, int Qoff, double Qgain )
{
    int Inum,Qnum;

    Inum = 127 * Igain;
    Qnum = 127 * Igain;

    mBdc->SpiRW16(  BDC_REG_WR | BDC_REG_R20 | (Ioff&0xff) );
    mBdc->SpiRW16(  BDC_REG_WR | BDC_REG_R21 | (Inum&0xff) );
    mBdc->SpiRW16(  BDC_REG_WR | BDC_REG_R22 | (Qoff&0xff) );
    mBdc->SpiRW16(  BDC_REG_WR | BDC_REG_R23 | (Qnum&0xff) );

    printf("Match:%d,%d %d,%d\n",Ioff,Inum,Qoff,Qnum);
}

//------------------------------------------------------------------------------
int
Ddc100::SetTpg( int arg )
{
    printf("Ddc100:SetTpg=%d\n",arg);
    mTpg = arg;

    mBdc->SpiRW16(  BDC_REG_WR | BDC_REG_R19 | (mTpg&0xff) );

    return(0);
}
//------------------------------------------------------------------------------
int
Ddc100::IsComplexFmt()
{
  return(1);
}

//------------------------------------------------------------------------------
int
Ddc100::SetSource( int arg )
{
    printf("Ddc100:SetSource=%d (mTpg=%d)\n",arg,mTpg);
    mFifoSrc = arg;

    mBdc->SpiRW16(  BDC_REG_WR | BDC_REG_R16 | (mFifoSrc&0xff) );

    Show( "at set\n" );
    return( arg );
}

//------------------------------------------------------------------------------
int
Ddc100::GetFwVersion()
{
    int ver;

    mBdc->SpiRW16(  BDC_REG_RD | BDC_REG_R0 );
    ver = mBdc->SpiRW16( 0 );

    FlushSamples(); // Necessary to start streaming

    return(ver);
}

//------------------------------------------------------------------------------
double
Ddc100::SetLoFreqHz( double freqHz )
{
    unsigned int   pincLo,pincHi;
    long long      hzMod,pinc;
    double         actHz;

    printf("Ddc100:SetFreq=%f Hz\n",freqHz);

    hzMod = ((long long)freqHz) % mFsHz;
    pinc  = hzMod * 65536 / mFsHz;
    pincLo=       pinc & 0x00ff;
    pincHi=  (pinc>>8) & 0x00ff;

    printf("Ddc100: pinc = %lld 0x%08x (0x%04x 0x%04x)\n",
                   pinc,(unsigned int)pinc,pincHi,pincLo);

    mBdc->SpiRW16( BDC_REG_WR | BDC_REG_R17 | pincLo );
    mBdc->SpiRW16( BDC_REG_WR | BDC_REG_R18 | pincHi );

    FlushSamples(); // Necessary to continue streaming

    actHz     = pinc * mFsHz / 65536;
    mLoFreqHz = actHz;
    printf("Ddc100:SetFreq= actual %f Hz\n",actHz);

    return( actHz );
}

//------------------------------------------------------------------------------
int
Ddc100::FlushSamples()
{
    // printf("Ddc100:FlushSamples Enter\n");

    // Stop the fpga acquisition

    // Flush the fpga fifos

    // TODO: Wait for the DRAM to drain to a constant spot

    // Start the fpga acquisition

    // TODO: Tell the pru to go back to streaming

    mBdc->SpiRW16( BDC_REG_WR | BDC_REG_R16 | 0x40 | (mFifoSrc&0xff) );
    mBdc->SpiRW16( BDC_REG_WR | BDC_REG_R16 | (mFifoSrc&0xff) );

    return(0);
}

//------------------------------------------------------------------------------
int
Ddc100::Get2kSamples( short *bf )
{
    int          p;
    int          srcIdx,idx;

    // Transfer 2k samples with single word cpu xfers for now

    // TODO - should wait until there are 2k present w/ indicator

    mBdc->SpiRW16( BDC_REG_RD | BDC_REG_R63  );
    for(idx=0;idx<2047;idx++){
        bf[idx] = mBdc->SpiRW16( BDC_REG_RD | BDC_REG_R63 );
    }
    bf[idx] = mBdc->SpiRW16( BDC_REG_RD | BDC_REG_R63 );

    mBdc->SpiRW16( BDC_REG_WR | BDC_REG_R16 | 0x40 | (mFifoSrc&0xff) );
    mBdc->SpiRW16( BDC_REG_WR | BDC_REG_R16 | (mFifoSrc&0xff) );

    return( p );
}

//------------------------------------------------------------------------------
int
Ddc100::StartPrus()
{
    // This method is not applicable.  pru00 is already started with fboard
    return( 0 );
}

//------------------------------------------------------------------------------
int
Ddc100::SetSim( int sim )
{
    // This method is not applicable
    return( 0 );
}

//------------------------------------------------------------------------------
int
Ddc100::SetComplexSampleRate( int complexSamplesPerSecond )
{
    printf("Ddc100:SetComplexSampleRate %d CSPS\n",complexSamplesPerSecond);
    printf("Ddc100:SetComplexSampleRate %d CSPS\n",mCSPS);
    return(0);
}

//------------------------------------------------------------------------------
int
Ddc100::GetComplexSampleRate()
{
    switch( mFifoSrc ){
        case 0  : return( mFsHz ); // raw input
        case 1  : return( mFsHz ); // equalized
        case 2  : return( mFsHz ); // dds/nco
        case 3  : return( mFsHz ); // mixer output
        case 4  : return( (mFsHz/10) );  // first stage decimation
        case 5  : return( (mFsHz/200) ); // second stage decimation
        default : return( mFsHz );
    }
}

//------------------------------------------------------------------------------
// NOTE: this adc interface is not supported
int
Ddc100::SetGain( int gn )
{
   if( gn!=0 ){
       fprintf(stderr,"Ddc100:SetGain invoked w/o 0 - err \n");
   }
   return( 0 );
}

//------------------------------------------------------------------------------
void
Ddc100::Show(const char *title )
{
    int rg,val;
    printf("Ddc100: %s",title);
    for(rg=0;rg<21;rg++){
        mBdc->SpiRW16( BDC_REG_RD | ((rg&0x3f)<<8) );
        val = mBdc->SpiRW16( 0 );
        printf("r[%02d] = 0x%04x\n",rg,val);
    }
}

