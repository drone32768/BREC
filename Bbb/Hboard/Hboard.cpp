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
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include "../Util/gpioutil.h"
#include "Hpru.h"
#include "prussdrv.h"
#include "pruss_intc_mapping.h"
#include "Hboard.h"
#include "pru_images.h"

//------------------------------------------------------------------------------
Hboard::Hboard()
{
   mPidx      = 0;
   mSimImage  = 0; // FIXME: default for testing...
   mDecimate  = 1;
//   mSamplingType = SAMPLE_TYPE_POLY_FIR;
   mSamplingType = SAMPLE_TYPE_DOWNSAMPLE;
//   mSamplingType = SAMPLE_TYPE_CIC_2_2;
}

//------------------------------------------------------------------------------
int
Hboard::SetGain( int gn )
{
   if( gn!=0 ){
       fprintf(stderr,"Hboard:SetGain invoked w/o 0 - err \n");
   }
   return( 0 );
}

//------------------------------------------------------------------------------
int
Hboard::Open()
{
    int ret;

    printf("Hboard:Open\n");

    prussdrv_init();
    ret = prussdrv_open(PRU_EVTOUT_0);
    if( ret ){
        fprintf(stderr,"prussdrv_open failed\n");
        return(-1);
    }
    prussdrv_map_extmem( (void**)( &mPtrPruSamples ) ); 
    prussdrv_map_prumem( PRUSS0_PRU0_DATARAM, (void**)(&mPtrPruSram) );
    mPtrHead = (unsigned int*)(mPtrPruSram + PRU0_OFFSET_DRAM_HEAD);

    mGpioEnable.Define( 22 );
    mGpioEnable.Export();
    mGpioEnable.SetDirInput( 0  );
    mGpioEnable.Open();
    mGpioEnable.Set( 1 );
    return(0);
}

//------------------------------------------------------------------------------
int
Hboard::Flush()
{
    mPidx = mPtrHead[0]/2;
    return(0);
}

//------------------------------------------------------------------------------
int 
Hboard::FlushSamples()
{
    mPidx = mPtrHead[0]/2;
    return(0);
}

//------------------------------------------------------------------------------
// NOTE this is only for test purposes, it will not support high sample rates
int
Hboard::GetSamplePair( short *eo )
{
   int n,p;

   for(p=0,n=0;n<2;n++){
        //  printf("ptr 0x%08x 0x%08x\n",mPtrHead[0],mPidx);
        while( mPidx == (mPtrHead[0]/2)  ){
            us_sleep( 5000 );
            p++;
        }
        eo[n] = ( mPtrPruSamples[ mPidx ] );
        mPidx = (mPidx+1)%PRU_MAX_SHORT_SAMPLES;
   }
   return(p);
}

//------------------------------------------------------------------------------
int
Hboard::Get2kSamples( short *bf )
{
   switch( mSamplingType ){
       case SAMPLE_TYPE_CIC_2_2:
           return( Get2kSamples_Cic_2_2(bf) );
	   break;
       case SAMPLE_TYPE_POLY_FIR:
           return( Get2kSamples_PolyFir(bf) );
	   break;
       case SAMPLE_TYPE_DOWNSAMPLE:
       default:
           return( Get2kSamples_DownSample(bf) );	         
   }
}

//------------------------------------------------------------------------------
int
Hboard::Get2kSamples_DownSample( short *bf )
{
    int          p;
    unsigned int mask;

    // Setup pause count and block mask
    p    = 0;
    mask = ~(2048-1);

    // Make sure we are aligned to 2k so we don't copy past end of src
    mPidx = (mPidx&mask);

    // Wait while the PRU source is within the same 2k block we want
    while( 1 ){
        if( (mPidx&mask) == ((mPtrHead[0]/2)&mask) ){
            us_sleep( 5000 );
            p++;
        }
        else{
            break;
        }
    }

    // NOTE: memcpy is just as fast as the hand optimized
    // asm with ldmia/stmia moving 32 bytes at a time
    memcpy( (void*)bf, (void*)(mPtrPruSamples+mPidx), 4096);
    mPidx = (mPidx+2048)%PRU_MAX_SHORT_SAMPLES;

    return( p );
}

//------------------------------------------------------------------------------
int
Hboard::Get2kSamples_Cic_2_2( short *bf )
{
    int          p;
    unsigned int mask;

    // Setup pause count and block mask
    p    = 0;
    mask = ~(2048-1);

    // Make sure we are aligned to 2k so we don't copy past end of src
    mPidx = (mPidx&mask);

    // Wait while the PRU source is within the same 2k block we want
    while( 1 ){
        if( (mPidx&mask) == ((mPtrHead[0]/2)&mask) ){
            us_sleep( 5000 );
            p++;
        }
        else{
            break;
        }
    }

    // NOTE: memcpy is just as fast as the hand optimized
    // asm with ldmia/stmia moving 32 bytes at a time
    memcpy( (void*)bf, (void*)(mPtrPruSamples+mPidx), 4096);
    mPidx = (mPidx+2048)%PRU_MAX_SHORT_SAMPLES;

    return( p );
}

//------------------------------------------------------------------------------
static short h00=1,h10=1,h20=1,h30=1;
static int mac0, mac1, mac2, mac3;

static void ppf( short *src, short *dst )
{
    // *dst = *src;
    //  return;

    mac0  = h00*src[0];
    mac1  = h10*src[1];
    mac2  = h20*src[2];
    mac3  = h30*src[3];
    
    *dst = mac0+mac1+mac2+mac3;
}


/*
NOTE: 
Use 10MSPS with fir=4 taps, dec=4.  
  Unable to keep up.
Use 5MSPS  with fir=4 taps, dec=4.  
  This barely keeps up with SDR (may be slightly low).
*/
int
Hboard::Get2kSamples_PolyFir( short *bf )
{

    volatile unsigned short *src;
    int           srcSamps;

    // Setup dst for output and count of output samples
    short        *dst      = bf;
    int           dstSamps = 0;

    // Setup pause count and block mask
    int             p = 0;
    unsigned int mask = ~(2048-1);

    // Loop until we have produced 2k output samples
    while( dstSamps < 2048 ){

        // Make sure we are aligned to 2k so we don't copy past end of src
        mPidx = (mPidx&mask);

        // Wait while the PRU source is within the same 2k block we want
        while( 1 ){
            if( (mPidx&mask) == ((mPtrHead[0]/2)&mask) ){
                us_sleep( 5000 );
                p++;
            }
            else{
                break;
            }
        }

        // Process the 2k input samples
        src      = mPtrPruSamples+mPidx;
        srcSamps = 2048;
        while( srcSamps>0 ){
            ppf( (short*)src, (short*)dst );

            src      +=4;
            srcSamps -=4;

            dst      +=1;
            dstSamps +=1;

        }

        // Move to next input block
        mPidx = (mPidx+2048)%PRU_MAX_SHORT_SAMPLES;
    }

    return( p );
}

//------------------------------------------------------------------------------
/**
 * Loads and starts the default embedded pru images for pru0 and pru1.
 */
int
Hboard::StartPrus()
{
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    printf("Hboard:StartPrus Enter\n");
    printf("Hboard:StartPrus dec=%d sim=%d\n",mDecimate,mSimImage);

    // Stop the prus
    prussdrv_pru_disable(0);
    prussdrv_pru_disable(1);

    prussdrv_pruintc_init(&pruss_intc_initdata);

    // Put a known data pattern in data sram
    {
       int idx;
       short *dst;

       dst = (short*)(mPtrPruSram + 0x00);
       for( idx=0;idx<2048;idx++ ){
           // *dst = (idx&0x2)?(8192+2048):(8192-2048)+(rand()&0x3);
           *dst = 32768 + (8192 * sin( 2*M_PI * 128  *(float)idx/2048.0 ));
	   *dst = *dst + ( rand() & 0x030 );
	   *dst = 0;
           dst++;
       }
    }

    // Initialize required ram values
    *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_DRAM_PBASE) ) = 
                                prussdrv_get_phys_addr((void*)mPtrPruSamples);
    *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_SPIN_COUNT) ) = 7;
    *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_DEC) )        = mDecimate;

    // Write the instructions
    if( mSimImage ){
        prussdrv_pru_write_memory(PRUSS0_PRU0_IRAM,0,
                             (unsigned int*)pru00_sim,sizeof(pru00_sim) );
        prussdrv_pru_write_memory(PRUSS0_PRU1_IRAM,0,
                             (unsigned int*)pru01_sim,sizeof(pru01_sim) );
    }
    else if( SAMPLE_TYPE_DOWNSAMPLE == mSamplingType ){
        prussdrv_pru_write_memory(PRUSS0_PRU0_IRAM,0,
                    (unsigned int*)pru00_downsample,sizeof(pru00_downsample) );
        prussdrv_pru_write_memory(PRUSS0_PRU1_IRAM,0,
                    (unsigned int*)pru01_downsample,sizeof(pru01_downsample) );
    }
    else if( SAMPLE_TYPE_CIC_2_2 == mSamplingType ){
        prussdrv_pru_write_memory(PRUSS0_PRU0_IRAM,0,
                    (unsigned int*)pru00_cic_2_2,sizeof(pru00_cic_2_2) );
        prussdrv_pru_write_memory(PRUSS0_PRU1_IRAM,0,
                    (unsigned int*)pru01_cic_2_2,sizeof(pru01_cic_2_2) );
    }
    else if( SAMPLE_TYPE_POLY_FIR == mSamplingType ){
        prussdrv_pru_write_memory(PRUSS0_PRU0_IRAM,0,
                    (unsigned int*)pru00_downsample,sizeof(pru00_downsample) );
        prussdrv_pru_write_memory(PRUSS0_PRU1_IRAM,0,
                    (unsigned int*)pru01_downsample,sizeof(pru01_downsample) );
    }
    else{
        printf("Hboard: Error - Unrecognized sampling type \n");
    }

    // Run/Enable prus
    prussdrv_pru_enable(0);
    prussdrv_pru_enable(1);

    printf("Hboard:StartPrus Exit\n");

    return(0);
}

//------------------------------------------------------------------------------
int
Hboard::SetSim( int sim )
{
    int idx;
    short *dst;

    mSimImage = sim;

    // Fill in ram pattern
    if( mSimImage ){
       dst = (short*)(mPtrPruSram + 0x00);
       for( idx=0;idx<2048;idx++ ){
           // *dst = (idx&0x2)?(16384):(0);
           *dst = 16384 * sin( 6.28 * 0.5 *(float)idx/2048.0 );
           dst++;
       }
    }

    // Restart PRUs based on new parameters
    StartPrus();

    return( 0 );
}

//------------------------------------------------------------------------------
int
Hboard::SetComplexSampleRate( int complexSamplesPerSecond )
{

    switch( complexSamplesPerSecond ){
        case 0:{
	    printf("Hboard:SetComplexSampleRate - off\n");
            prussdrv_pru_disable(0);
            mCSPS = 0;
            break;
        }

        case 500000:{
            mCSPS     = 500000;
            mDecimate = 10;
            StartPrus();
            break;
        }

        case 1000000:{
            mCSPS     = 1000000;
            mDecimate = 5;
            StartPrus();
            break;
        }

        case 1250000:{
            mCSPS     = 1250000;
            mDecimate = 4;
            StartPrus();
            break;
        }

        case 2500000:{
            mCSPS     = 2500000;
            mDecimate = 2;
            StartPrus();
            break;
        }

        case 5000000:
        default: {
            mCSPS     = 5000000;
            mDecimate = 1;
            StartPrus();
        }
    }
    printf("Hboard:SetComplexSampleRate %d CSPS\n",mCSPS);

    return(0);
}

//------------------------------------------------------------------------------
int
Hboard::GetComplexSampleRate()
{
    return( mCSPS );
}

//------------------------------------------------------------------------------
/**
 * This method collects the specified number of samples, calculates the
 * rms level in dB relative to a 16 bit full scale and the mean value
 * placing both results in the provided locations.
 *
 * NOTE:
 * The samples are treated as 16 bit values.  The 12 bit adc results
 * are shifted up by 2 bits (e.g. multiply by 4).  The samples are also
 * unsigned shorts so range from 0*4 .. 4096*4=16k.  To remove the
 * DC bias (determined by the final opamp feedback which have mfg 
 * variation) and assess bias point and any potential oscillations or
 * instabilities, the DC results / mean are produced.  This should be
 * on the order of 2048 (half 12 bit scale) X 4 for the shift or 8192.
 * The AC value is the difference between successive samples, squared
 * and then averaged over the sample set.  This means the dB report
 * of AC value ranges from :
 *
 *    max = (0*4 - 4096*4)^2 / 65536^2, then log of this x 10 
 *        = 10 * log10( 16k^2 / 64k^2 ) = 10*log10( 0.0625 ) 
 *        = -12 dBFS16
 *
 *    min = ((8192-4) - (8192+4))^2 / 65536 ^2, then log of this x 10
 *        = 10 * log10( 8^2 / 65536^2 ) = 10*log10( 1.48E-8 )
 *        = -78 dBFS16
 *
 * In practice, once you get above -16dBFS16 the bias point shifts and
 * things seem to peg out at quarter scale or larger producing odd 
 * results (i.e. large signal values have significant non-linearities
 * and complex behavior, for reasonable results don't go beyond quarter scale)
 *
 */
int Hboard::GetRms( int nSamples, short *aSamples, double *rrms )
{
    int    idx;
    double delta,sum,rmsSum,mean;

    Flush();

    sum = 0.0;
    for( idx=0; idx<nSamples; idx+=2 ){
        GetSamplePair( aSamples+idx );
        sum += aSamples[ idx   ];
        sum += aSamples[ idx+1 ];
    }
    mean = sum / nSamples;

    rmsSum = 0.0;
    for( idx=1; idx<nSamples; idx++){
        delta  = aSamples[idx] - mean;
        rmsSum = rmsSum + (delta*delta);
    }

    *rrms = rmsSum / nSamples;
    *rrms = *rrms / (65536.0*65536.0);
    *rrms = 10 * log10( *rrms );

    return( mean );
}

