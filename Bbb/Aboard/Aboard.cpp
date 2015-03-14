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

#include "../Util/gpioutil.h"
#include "pructl.h"
#include "prussdrv.h"
#include "pruss_intc_mapping.h"
#include "Aboard.h"
#include "pru_images.h"

//------------------------------------------------------------------------------
static int gnInitialized = 0;
static GpioUtil gClkDiv[2];
static GpioUtil gIfGain[2];

#define ABOARD_NPORTS 2
static Gpio6PinGroup gPinGroup[ ABOARD_NPORTS ];

//------------------------------------------------------------------------------
int clock_and_gain_init()
{
    if( gnInitialized ) return(0);

    gClkDiv[0].Open( 113 );
    gClkDiv[1].Open( 111 );

    gClkDiv[0].Set( 0 );
    gClkDiv[1].Set( 0 );

    gIfGain[0].Open( 31 );
    gIfGain[1].Open( 30 );

    gIfGain[0].Set( 0 );
    gIfGain[1].Set( 0 );

    gnInitialized = 1;
    return(0);
}

//------------------------------------------------------------------------------
Aboard::Aboard()
{
   mPidx = 0;

   // B board pins
   gPinGroup[0].DefineMoSi       ( 67 ); 
   gPinGroup[0].DefineMiSo       ( 45 ); 
   gPinGroup[0].DefineSclk       ( 66 ); 
   gPinGroup[0].DefineSs1        (  3 ); 
   // gPinGroup[0].DefineSs2        (  2 ); 
   //gPinGroup[0].DefineGpio2      ( 44 ); 

   // R board pins
   gPinGroup[1].DefineMoSi       ( 69 ); 
   // NOT USED gPinGroup[1].DefineMiSo       ( 26 ); 
   gPinGroup[1].DefineSclk       ( 68 ); 
   gPinGroup[1].DefineSs1        (  4 ); 
   // gPinGroup[1].DefineSs2        (  5 ); 
   // NOT USED gPinGroup[1].DefineGpio2      ( 23 ); 

}

//------------------------------------------------------------------------------
Gpio6PinGroup* Aboard::AllocPort( int pn )
{
   // TODO - check for double allocation
   return( &gPinGroup[ pn&1 ] );
}

//------------------------------------------------------------------------------
int
Aboard::Open()
{
    int ret;

    printf("Aboard:Open\n");

    clock_and_gain_init();

    prussdrv_init();
    ret = prussdrv_open(PRU_EVTOUT_0);
    if( ret ){
        fprintf(stderr,"prussdrv_open failed\n");
        return(-1);
    }
    prussdrv_map_extmem( (void**)( &mPtrPruSamples ) ); 
    prussdrv_map_prumem( PRUSS0_PRU0_DATARAM, (void**)(&mPtrPruSram) );
    mPtrHead = (unsigned int*)(mPtrPruSram + PRU0_OFFSET_DRAM_HEAD);
    return(0);
}

//------------------------------------------------------------------------------
/*
 * TODO Investigate use of :
 * #include <asm/cachectl.h>
 * cacheflush( mPtrPruSamples, 2*PRU_MAX_SHORT_SAMPLES, DCACHE);
 *
 */
int
Aboard::Flush()
{
    mPidx = mPtrHead[0]/2;
#ifdef TGT_ARM
    // Ugh... There has to be a better builtin approach
    FILE *fp;	 
    fp=fopen("/proc/sys/vm/drop_caches","w");
    fprintf(fp,"3");
    fclose(fp);
#endif

    return(0);
}

//------------------------------------------------------------------------------
int 
Aboard::FlushSamples()
{
    mPidx = mPtrHead[0]/2;
    return(0);
}

//------------------------------------------------------------------------------
int
Aboard::Get2kSamples( short *bf )
{
    int          idx,p;
    unsigned int mask;

    p    = 0;
    mask = ~(2048-1);
    while( 1 ){
        if( (mPidx&mask) == ((mPtrHead[0]/2)&mask) ){
            us_sleep( 5000 );
            p++;
        }
        else{
            break;
        }
    }

    for(idx=0;idx<2048;idx++){
        bf[idx] = ( mPtrPruSamples[ mPidx ] );
        mPidx = (mPidx+1)%PRU_MAX_SHORT_SAMPLES;
    }
    return( p );
}

//------------------------------------------------------------------------------
int
Aboard::GetSamplePair( short *eo )
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
/**
 * Loads and starts the default embedded pru images for pru0 and pru1.
 */
int
Aboard::StartPrus()
{
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    // Stop the prus
    prussdrv_pru_disable(0);
    prussdrv_pru_disable(1);

    prussdrv_pruintc_init(&pruss_intc_initdata);

    // Initialize required ram values
    *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_DRAM_PBASE) ) = 
                                prussdrv_get_phys_addr((void*)mPtrPruSamples);
    *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_SPIN_COUNT) ) = 40;

    // Write the instructions
    prussdrv_pru_write_memory(PRUSS0_PRU0_IRAM,0,
                              (unsigned int*)pru_image0,sizeof(pru_image0) );
    prussdrv_pru_write_memory(PRUSS0_PRU1_IRAM,0,
                              (unsigned int*)pru_image1,sizeof(pru_image1) );

    // Run/Enable prus
    prussdrv_pru_enable(0);
    prussdrv_pru_enable(1);

    return(0);
}

//------------------------------------------------------------------------------
int
Aboard::SetComplexSampleRate( int complexSamplesPerSecond )
{
    switch( complexSamplesPerSecond ){
        case 833333 :{
            gClkDiv[0].Set( 1 );
            gClkDiv[1].Set( 0 );

            *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_SPIN_COUNT) ) = 1;
	    printf("Aboard:SetComplexSampleRate - 833k CSPS\n");
            mCSPS = 833333;
	    break;
        }
        case 625000 :{
            gClkDiv[0].Set( 0 );
            gClkDiv[1].Set( 1 );

            *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_SPIN_COUNT) ) = 1;
	    printf("Aboard:SetComplexSampleRate - 625k CSPS\n");
            mCSPS = 625000;
	    break;
        }

        case 500000 :{
            gClkDiv[0].Set( 1 );
            gClkDiv[1].Set( 1 );

            *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_SPIN_COUNT) ) = 40;
	    printf("Aboard:SetComplexSampleRate - 500k CSPS\n");
            mCSPS = 500000;
	    break;
        }

        default: {
            gClkDiv[0].Set( 0 );
            gClkDiv[1].Set( 0 );
            mCSPS = 0;
	    printf("Aboard:SetComplexSampleRate - off\n");
        }
    }

    return(0);
}

//------------------------------------------------------------------------------
int
Aboard::GetComplexSampleRate( )
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
int Aboard::GetRms( int nSamples, short *aSamples, double *rrms )
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

//------------------------------------------------------------------------------
int
Aboard::SetGain( int gn )
{
    gIfGain[0].Set( gn&2?1:0 );
    gIfGain[1].Set( gn&1?1:0 );

    return( 0 );
}
