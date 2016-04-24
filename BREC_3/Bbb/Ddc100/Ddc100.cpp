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

#include "Ddc100.h"

#include "pruinc.h"
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
#define GetSramWord( off )   ( *(unsigned int*  )((mPtrPruSram + (off))) )
#define SetSramWord( v,off ) ( *(unsigned int*  )((mPtrPruSram + (off))) = (v) )

#define GetSramShort( off )  ( *(unsigned short*)((mPtrPruSram + (off))) )
#define SetSramShort( v,off ) (*(unsigned short*)((mPtrPruSram + (off))) = (v) )

////////////////////////////////////////////////////////////////////////////////
/// External Methods ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
Ddc100::Ddc100()
{
    mTpg     = 4; 
    mFifoSrc = 0;
    mFsHz    = 10000000; // Function of the board
    mCSPS    = mFsHz;    // Function of board and channel selected
    mPtrPruSram = 0;
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
    unsigned short fwVer;

    printf("Ddc100:Open enter\n");

    fwVer = GetFwVersion();
    printf("Ddc100::Open::fw ver = 0x%08x\n",fwVer);

    mFsHz    = 40000000; // Function of the board
    mCSPS    = mFsHz;    

    printf("Ddc100::Open::FsHz        = %d\n",mFsHz);
    printf("Ddc100::Open::CSPS        = %d\n",mCSPS);

    // Set startup default signal paramters
    SetLoFreqHz( 640000 );

    SetSource( 0 ); 
    SetTpg( 0 ); 
    SetChannelMatch(0,1.0,0,1.0);

    printf("Ddc100:Open exit\n");

    return(0);
}

//------------------------------------------------------------------------------
int
Ddc100::SetChannelMatch( int Ioff, double Igain, int Qoff, double Qgain )
{
    int Inum,Qnum;

    Inum = -128 * Igain;
    Qnum = -128 * Qgain;

    mBdc->SpiRW16(  BDC_REG_WR | BDC_REG_R20 | (Ioff&0xff) );
    mBdc->SpiRW16(  BDC_REG_WR | BDC_REG_R21 | (Inum&0xff) );
    mBdc->SpiRW16(  BDC_REG_WR | BDC_REG_R22 | (Qoff&0xff) );
    mBdc->SpiRW16(  BDC_REG_WR | BDC_REG_R23 | (Qnum&0xff) );

    printf("Ddc100:Match: I=[+ %d, X %d (%f) ] Q=[+ %d, X %d (%f)]\n",
                Ioff,Inum,Igain, 
                Qoff,Qnum,Qgain
    );

    return(0);
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
  return( 1 );
}

//------------------------------------------------------------------------------
int
Ddc100::SetSource( int arg )
{
    printf("Ddc100:SetSource=%d (mTpg=%d)\n",arg,mTpg);
    mFifoSrc = arg;

    mBdc->SpiRW16(  BDC_REG_WR | BDC_REG_R16 | (mFifoSrc&0xff) );

    return( arg );
}

//------------------------------------------------------------------------------
int
Ddc100::GetFwVersion()
{
    int ver;

    mBdc->SpiRW16(  BDC_REG_RD | BDC_REG_R1 );
    ver = mBdc->SpiRW16( 0 );
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

    actHz     = pinc * mFsHz / 65536;
    mLoFreqHz = actHz;
    printf("Ddc100:SetFreq= actual %f Hz\n",actHz);

    return( actHz );
}

//------------------------------------------------------------------------------
int
Ddc100::FlushSamples()
{
    unsigned short cmd=PRU1_CMD_NONE;
    unsigned short res;

    // Place the fifo in reset
    mBdc->SpiRW16( BDC_REG_WR | BDC_REG_R16 | 0x40 | (mFifoSrc&0xff) );

    // If we are streaming then we need to take additional steps
    if( 0!=mPtrPruSram ){

       // Save the currently active pru1 command
       cmd = GetSramWord( PRU1_LOFF_CMD );

       // Make sure pru1 has stopped writing data to dram
       do{
           res = GetSramWord( PRU1_LOFF_RES );
           if( res!=PRU1_CMD_NONE ){
               SetSramShort(PRU1_CMD_NONE, PRU1_LOFF_CMD );
               us_sleep(10);
           }
       }while( res!=PRU1_CMD_NONE );
      
       // Update the cpu head index to current pru1 index
       // (div by 2 to convert from byte offset to short index)
       mPidx = ( GetSramWord( PRU1_LOFF_DRAM_OFF) / 2 ); 

    }

    // Take the fifo out of reset
    mBdc->SpiRW16( BDC_REG_WR | BDC_REG_R16 | (mFifoSrc&0xff) );

    // If the pru is active restore the pru1 active command
    if( 0!=mPtrPruSram ){
        SetSramShort(cmd, PRU1_LOFF_CMD );
    }

    return(0);
}

//------------------------------------------------------------------------------
int
Ddc100::Get2kSamples( short *bf )
{
    if( 0!=mPtrPruSram ){
        return( Get2k_Pru(bf) );
    }
    else{
        return( Get2k_Cpu(bf) );
    }
}

//------------------------------------------------------------------------------
int
Ddc100::Get2k_Pru( short *bf )
{
    int dstIdx;
    int pruIdx;
    int p;

    // TODO this should be converted to copy more than 16 bits at a time

    dstIdx = 0;
    p      = 0;
    pruIdx = GetSramWord( PRU1_LOFF_DRAM_OFF);
    pruIdx = pruIdx/2; // PRU1 dram offset is in bytes, need in shorts
    while( dstIdx<2048 ){

        // If our index is the same as pru, wait and then re-eval
        while( mPidx == pruIdx ){
            us_sleep(100);
            pruIdx = GetSramWord( PRU1_LOFF_DRAM_OFF);
            pruIdx = pruIdx/2; // byte offset to short offset
            p++;
        }

        // Copy a sample
        bf[ dstIdx ] = mPtrPruSamples[ mPidx ];
        dstIdx++;
        mPidx = (mPidx+1)%PRU_MAX_SHORT_SAMPLES;
    }
    return(p);
}

//------------------------------------------------------------------------------
int
Ddc100::Get2k_Cpu( short *bf )
{
    int          p;
    int          idx;
    unsigned int belowThresh;

    // Transfer 2k samples with single word cpu xfers for now

    // Wait until the threashold bit inidcates samples ready
    p = 0;
    do{
       mBdc->SpiRW16( BDC_REG_RD | BDC_REG_R61  );
       belowThresh = mBdc->SpiRW16( 0x0 );
       if( belowThresh ) us_sleep( 100 );
       p = 0;
    }while( belowThresh );

    // Read 2k of samples
    mBdc->SpiRW16( BDC_REG_RD | BDC_REG_R63  );
    for(idx=0;idx<2047;idx++){
        bf[idx] = mBdc->SpiRW16( BDC_REG_RD | BDC_REG_R63 );
    }
    bf[idx] = mBdc->SpiRW16( 0 ); // Fetch last value but do not init read

    return( p );
}

//------------------------------------------------------------------------------
int
Ddc100::StartPru()
{
    // NOTE: the constants share with pru code are relative to how it
    // references its sram which is zero based, however, cpu accesses
    // globally so pru1 is +0x2000
    mPtrPruSram    = mBdc->PruGetSramPtr() + 0x2000;
    mPtrPruSamples = mBdc->PruGetDramPtr();

    SetSramWord(  prussdrv_get_phys_addr( (void*)mPtrPruSamples ),
                  PRU1_LOFF_DRAM_PBASE 
               );

    SetSramWord(  0,
                  PRU1_LOFF_DRAM_OFF 
               );

    SetSramWord(  0,
                  PRU1_LOFF_DBG1 
               );

    SetSramWord(  0,
                  PRU1_LOFF_DBG2 
               );

    SetSramWord(  0,
                  PRU1_LOFF_DBG3 
               );

    SetSramShort( PRU1_CMD_NONE,
                  PRU1_LOFF_CMD 
               );

    prussdrv_pru_write_memory(PRUSS0_PRU1_IRAM,0,
                             (unsigned int*)pru_image01,sizeof(pru_image01) );

    prussdrv_pru_enable(1);

    SetSramShort(  PRU1_CMD_2KWORDS,
                   PRU1_LOFF_CMD 
               );

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
    int cc,rc;
    printf("Ddc100: %s",title);

    rg = 0;
    for(rc=0;rc<16;rc++){

        for(cc=0;cc<4;cc++){
            mBdc->SpiRW16( BDC_REG_RD | ((rg&0x3f)<<8) );
            val = mBdc->SpiRW16( 0 );
            printf("r[%02d] = 0x%04x  ",rg,val);
            rg++;
        }
        printf("\n");

    }

    if( 0!=mPtrPruSram ){
     printf("    PRU1 dbg1     0x%08x\n",GetSramWord(  PRU1_LOFF_DBG1 ) );
     printf("    PRU1 dbg2     0x%08x\n",GetSramWord(  PRU1_LOFF_DBG2 ) );
     printf("    PRU1 dbg3     0x%08x\n",GetSramWord(  PRU1_LOFF_DBG3 ) );
     printf("    PRU1 pbase    0x%08x\n",GetSramWord(  PRU1_LOFF_DRAM_PBASE) );
     printf("    PRU1 dram off 0x%08x\n",GetSramWord(  PRU1_LOFF_DRAM_OFF) );
     printf("    PRU1 cmd      0x%08x\n",GetSramShort( PRU1_LOFF_CMD ) );
     printf("    PRU1 res      0x%08x\n",GetSramShort( PRU1_LOFF_RES ) );

     int idx,cnt;
     idx = GetSramWord( PRU1_LOFF_DRAM_OFF);
     idx = (idx + PRU_MAX_SHORT_SAMPLES - 16)%PRU_MAX_SHORT_SAMPLES;
     for(cnt=0;cnt<8;cnt++){
       printf("%d 0x%04x, ",idx,mPtrPruSamples[idx]);
       idx = (idx + 1)%PRU_MAX_SHORT_SAMPLES;
       
     }
     printf("\n");

    }else{
     printf("    PRU1 not started\n");
    }

}

