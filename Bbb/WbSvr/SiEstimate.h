/*
 *
 * This source code is available under the "Simplified BSD license".
 *
 * Copyright (c) 2013, J. Kleiner
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the original author nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef __SI_ESTIMATE__
#define __SI_ESTIMATE__ 

#include <fftw3.h>

////////////////////////////////////////////////////////////////////////////////
/**
 * This class defines a dB power estimator.  It uses an FFT with a window
 * on a set of 16 bit samples to produce a log estimate.
 */
class SiEstimator {
    private: 

      /** This is a bitmask of logging flags */
#     define M2_LOG_PLAN   0x00000001
#     define M2_LOG_INPUT  0x00000002
#     define M2_LOG_OUTPUT 0x00000004
      unsigned int  mLog;

      /** This is the number of fft points used */
      int           mNfftPoints;

      /** This flag indicates that Fftw should be used */
      int           mUseFftw;

      /** This is the input and output vector */
      double       *mInOut;

      /** This is the window/envelop vector */
      double       *mWin;

      /** This is the attenuation factor to be used to account for windowing */
      double       mCoherentGain;

      /** This is the plan to use with fftw */
      fftw_plan     mFftwPlan;

      /** This is the output of fftw */
      fftw_complex *mFftwOutput;

      /** This is the input and output used with the NR fft */
      double       *mNrInOut;

      /** This is the accumulating fft */
      double       *mTmp;

      /** This is the number of bins overwhich a measurement is taken 
       *  NOTE: it can range from 1.. fft bins/2
       */
      int          mMeasureBins;

      /** Number of measurement bins to sum together for a single estimate 
       *  NOTE: it must be an integer division of measurement bins
       */
      int          mSumBins;

      /** Control whether we are using total power estimate or ave power */
      int          mBinAve;

      void Cleanup();

    public:
      SiEstimator();
      ~SiEstimator();
      void Configure( int fftSize, int wt, int fast, int mbins, int sbins );
      int  Pse( short *samples, int input_samples );  
      int  Estimate( short  *aInputSamples, int sampleCount, double *magOut );
};

#endif /*  __SI_ESTIMATE__  */
