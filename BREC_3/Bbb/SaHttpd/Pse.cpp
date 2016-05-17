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
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "Util/mcf.h"
#include "Device.h"
#include "Pse.h"

////////////////////////////////////////////////////////////////////////////////
Pse::Pse()
{
    mLog        = 0; // (PSE_LOG_PCOHI | PSE_LOG_OUTX | PSE_LOG_OUTY) ; 
    mMaxFft     = 32768;
    mMaxSamp    = 8*mMaxFft;
    mInput      = (short*)malloc(  2 * mMaxSamp * sizeof(short)  );
    mWin        = (double*)malloc(     mMaxFft  * sizeof(double) );
    mCurFftSize = -1;   // invalid, ensure setup executes
    mCurWinType = -1;   // invalid, ensure setup executes
    mFftwPlan   = NULL; // this is adjusted on the fly by setup
    mFftwOutput = NULL; // this is adjusted on the fly by setup
}

void
Pse::PerformSetup( int winType, int fftSize )
{
    int idx;

    if( mLog&PSE_LOG_SETUP ){
       printf("Pse:PerformSetup: Clear history win=%d, fft=%d\n",
                              winType,fftSize);
    }

    // if the window type and fft size are the same we are done
    if( winType==mCurWinType && 
        fftSize==mCurFftSize ){
       return;
    }

    if( mLog&PSE_LOG_SETUP ){
        printf("Pse:PerformSetup: Change setup win=%d, fft=%d\n",
                 winType,fftSize);
    }

    // save the new win type and fft size
    mCurWinType = winType;
    mCurFftSize = fftSize;

    // Establish the window function
    // This includes saving the calculate coherent gain
    if( 0==mCurWinType ){
        for(idx=0;idx<mCurFftSize;idx++){
            mWin[idx] = 1.0;
        }
        mCoherentGain = 1.0;
    }
    else {
       double x,a0,a1,a2,sum;
       a0 = 0.42659;
       a1 = 0.49656;
       a2 = 0.076849;
       sum = 0.0;
       for(idx=0;idx<mCurFftSize;idx++){
           x = (2.0*M_PI * idx)/(mCurFftSize-1);
           mWin[idx] = a0 - a1*cos(x) + a2*cos(2*x);
           sum+=mWin[idx];
       }
       mCoherentGain = sum / mCurFftSize;
    }

    // Establish the fftw workspace
    if( mFftwOutput ) {
        fftw_free( mFftwOutput );
    }
    mFftwOutput = (fftw_complex*)fftw_malloc(
                                  sizeof(fftw_complex)*mCurFftSize );

    // Establish the fftw plan
    if( mFftwPlan ){
       fftw_destroy_plan( mFftwPlan );
    }
    mFftwPlan = fftw_plan_dft_1d(
                  mCurFftSize, 
                  mFftwOutput, 
                  mFftwOutput,
                  FFTW_FORWARD, 
                  FFTW_ESTIMATE );

}

short * 
Pse::PerformFft( int complexInput, int fftSize, short *inputData )
{
    int    idx;
    short *newHead;

    if( mLog&PSE_LOG_FFT ){
        printf("Pse:PerformFft: Copying input\n");
    }

    // Prepare the input samples
    if( complexInput ){
       for(idx=0;idx<fftSize;idx++){
          mFftwOutput[idx][0] = inputData[2*idx + 1] * mWin[idx];
          mFftwOutput[idx][1] = inputData[2*idx    ] * mWin[idx];
       }
       newHead = inputData + 2*fftSize;
    }
    else{
       for(idx=0;idx<fftSize;idx++){
          mFftwOutput[idx][0] = inputData[idx] * mWin[idx];
          mFftwOutput[idx][1] = 0.0;
       }
       newHead = inputData + fftSize;
    }

    if( mLog&PSE_LOG_FFT_IN ){
        for(idx=1;idx<fftSize;idx++){
            printf("Pse:mInOut(in)[%d], %g, %g\n",
                idx,mFftwOutput[idx][0],mFftwOutput[idx][1] );
        }
    }

    // Perform the fft
    fftw_execute( mFftwPlan );

    return( newHead );
}

void
Pse::GetEstimate(
    short *inputData,
    int    nComplexSamples,
    int    inBins,
    int    outBins,
    double *output
)
{
    int    didx,sidx,pidx;
    int    scnt,pcnt;
    double normalize;
    double magSqr,pwr;
    double bsum;

    // Setup
    PerformSetup( 1 /* 0=no win, 1=win */, nComplexSamples );

    // Calculate fft
    PerformFft(   1 /*complex=true*/,nComplexSamples,inputData );

    // Capture the number of input bins per output bin (bins summed)
    bsum = (double)inBins / (double)outBins;

    // Calculate the normalization factor
    // NOTE: this must be done after setup so gain is correct
    normalize =   0.0
                  + 20*log10( 32768 )            // Ref 16 signed bit
                  + 20*log10( mCoherentGain )    // Remove Windowing
                  + 20*log10( nComplexSamples )  // Remove  1/N^2 from dft
                ;

    // Copy out values
    didx = 0;
    sidx = nComplexSamples - (inBins/2);
    scnt = 0;
//printf("sidx=%d outBins=%d\n",sidx,outBins);
    while( didx < outBins ){

        pwr = 0.0;
        pcnt= (int)( ((double)(didx+1) * bsum)+0.5);
//printf("didx=%d pcnt=%d sidx=%d ",didx,pcnt,sidx);
        while( scnt < pcnt ){
            magSqr = (mFftwOutput[sidx][0] * mFftwOutput[sidx][0]    ) +
                     (mFftwOutput[sidx][1] * mFftwOutput[sidx][1]);
            pwr    = pwr + magSqr;
            sidx   = (sidx+1)%(nComplexSamples);
            scnt++;
        }

        // Move forward source and destination indecies
        output[ didx ]  = 10*log10(pwr) - normalize;
//printf("out=%f\n",output[didx]);

        // Move to the next destination bin
        didx++;
    }
}
