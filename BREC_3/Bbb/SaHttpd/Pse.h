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
#ifndef __PSE__
#define __PSE__
#include <fftw3.h>

/*
The key input is number of desired output points

Output     Format   FftSize    NshortIn
nPoints    complex    nPoints  2*nPoints
           real     2*nPoints  2*nPoints
*/

class Pse {
#  define   PSE_LOG_SETUP    0x00000001
#  define   PSE_LOG_FFT      0x00000002
#  define   PSE_LOG_FFT_IN   0x00000004
#  define   PSE_LOG_OUTX     0x00000008
#  define   PSE_LOG_OUTY     0x00000010
#  define   PSE_LOG_PCOHI    0x00000020

   int      mLog;      // logging mask
   int      mMaxSamp;  // max number coherent samples
   int      mMaxFft;   // max fft size

   short   *mInput;    // Input samples

   fftw_plan     mFftwPlan;
   fftw_complex *mFftwOutput;

   double  *mWin;          // Windowing function, real
   double  *mFftSum;       // Coherent fft sum
   int      mFftCount;     // Number of fft's in sum

   int      mCurFftSize;   // Currently configured fft size
   int      mCurWinType;   // Currently configured window type
   double   mCoherentGain; // Current coherent gain

public:


    void   PerformSetup(   int winType,   int fftSize );
    short* PerformFft(     int isComplex, int fftSize, short *src );

    Pse();
    void   GetEstimate(
              short *samples,
              int    nComplexSamples,
              int    inBins,
              int    outBins,
              double *output
           );
};
#endif /* __PSE__ */
