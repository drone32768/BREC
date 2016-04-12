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
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "Util/mcf.h"
#include "Devs.h"
#include "Tim.h"

////////////////////////////////////////////////////////////////////////////////
Tim::Tim()
{
    mLog        = 0; 
    mMaxSamp    = 8192;
    mInput      = (short*)malloc(  2 * mMaxSamp * sizeof(short)  );
}

void
Tim::PerformOutputX( int isComplex, int nPts, double *aXvec )
{
    int    idx;

    if( mLog&TIM_LOG_OUTX ){
        printf("Pse:PerformOutputX: pts=%d,cmplx=%d\n",
               nPts,isComplex);
    }

    for(idx=0;idx<nPts;idx++){
        aXvec[idx] = idx;
    }

}

void
Tim::PerformOutputY( int isComplex, int nPts, double *aYvec )
{
    int    didx,sidx;

    // Loop over output creating dBFS16
    sidx = 0;
    didx = 0;
    if( mLog&TIM_LOG_OUTY ){
        printf("Tim:PerformOutputY: npts=%d\n", nPts);
    }
    while( didx < nPts ){
       aYvec[didx] = (double)( mInput[sidx] );
       didx++;
       sidx++;
    }
}

void 
Tim::ProcessCoherentInterval( 
      int     aNave,
      int     aPts,
      double *aXvec,
      double *aYvec
    )
{
    short *dst;
    int    cnt,lim;
    int    isComplex;
 
    // Get format
    isComplex = Dp()->Adc()->IsComplexFmt();

    if( mLog&TIM_LOG_PCOHI ){
        printf("Tim:nav=%d,pts=%d,cmplx=%d\n",
                aNave,aPts,isComplex);
    }

    // Sanity check args to prevent downstream ambiguous errors
    if( aPts >= mMaxSamp ){
        fprintf(stderr,"Tim::pts > max samples\n");
        return;
    }
    if( aPts <=0 ){
        fprintf(stderr,"Tim::pts <= 0\n");
        return;
    }
    if( aNave <=0 ){
        fprintf(stderr,"Tim::ave <= 0\n");
        return;
    }

    // Capture the coherent number of required samples
    // The adc interface is really 2k sample words so
    // in complex cases we are really only getting 1k complex
    // samples each time.
    dst = mInput;
    cnt = 0;
    lim = aPts;
    Dp()->Adc()->FlushSamples();
    while( cnt<lim ) {
       Dp()->Adc()->Get2kSamples( dst ); 
       dst += 2048;
       cnt += 2048;
    }

    if( mLog&TIM_LOG_PCOHI ){
        printf("Tim::ProcessCoherentInterval: %d words, %d samples, %d coll\n",
                     lim, aPts,cnt);
    }

    // Format and output the X values
    PerformOutputX( isComplex, aPts, aXvec );

    // Format and output the Y values
    PerformOutputY( isComplex, aPts, aYvec );

}

