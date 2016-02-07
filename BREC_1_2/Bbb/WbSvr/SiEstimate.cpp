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
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "SiEstimate.h"

////////////////////////////////////////////////////////////////////////////////
/**
 * This is a simple fft from numerical recipes used as a reference and
 * as the fft method in cases where external numeric packages are not 
 * installed.  It is highly recommended to use the FFTW external library
 */
/*
 Inputs:
    data[] : array of complex* data points of size 2*NFFT+1.
         data[0] is unused,
         the n'th complex number x(n), for 0 <= n <= length(x)-1, is stored as:
            data[2*n+1] = real(x(n))
            data[2*n+2] = imag(x(n))
    if length(Nx) < NFFT, the remainder of the array must be padded with zeros

    nn : FFT order NFFT. This MUST be a power of 2 and >= length(x).
    isign:  if set to 1, 
                computes the forward FFT
            if set to -1, 
                computes Inverse FFT - in this case the output values have
                to be manually normalized by multiplying with 1/NFFT.
 Outputs:
    data[] : The FFT or IFFT results are stored in data, overwriting the input.
*/
static void four1(double data[], int nn, int isign)
{
#define TWOPI    (2.0*M_PI)
    int n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;
    
    n = nn << 1;
    j = 1;
    for (i = 1; i < n; i += 2) {
        if (j > i) {
            tempr = data[j];     data[j] = data[i];     data[i] = tempr;
            tempr = data[j+1]; data[j+1] = data[i+1]; data[i+1] = tempr;
        }
        m = n >> 1;
        while (m >= 2 && j > m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }
    mmax = 2;
    while (n > mmax) {
        istep = 2*mmax;
        theta = TWOPI/(isign*mmax);
        wtemp = sin(0.5*theta);
        wpr = -2.0*wtemp*wtemp;
        wpi = sin(theta);
        wr = 1.0;
        wi = 0.0;
        for (m = 1; m < mmax; m += 2) {
            for (i = m; i <= n; i += istep) {
                j =i + mmax;
                tempr = wr*data[j]   - wi*data[j+1];
                tempi = wr*data[j+1] + wi*data[j];
                data[j]   = data[i]   - tempr;
                data[j+1] = data[i+1] - tempi;
                data[i] += tempr;
                data[i+1] += tempi;
            }
            wr = (wtemp = wr)*wpr - wi*wpi + wr;
            wi = wi*wpr + wtemp*wpi + wi;
        }
        mmax = istep;
    }
}
////////////////////////////////////////////////////////////////////////////////
/**
 * The constructor does nothing other than setup internals.  The parameters
 * must be set (at which point actual space is created)
 */
SiEstimator::SiEstimator()
{
    mLog     = 0;
    mInOut   = NULL;
    mWin     = NULL;
    mNrInOut = NULL;

    mUseFftw    = 1;
    mFftwPlan   = NULL;
    mFftwOutput = NULL;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * This method free's any resources associated with the object
 */
void SiEstimator::Cleanup()
{
    if( mInOut ){
        free( mInOut );
        mInOut = NULL;
    }
    if( mWin   ){
        free( mWin );
        mWin = NULL;
    }
    if( mNrInOut   ){
        free( mNrInOut );
        mNrInOut = NULL;
    }

    if( mFftwPlan   ) {
        fftw_destroy_plan( mFftwPlan );
        mFftwPlan=NULL;
    }
    if( mFftwOutput ) {
        fftw_free( mFftwOutput ); 
        mFftwOutput=NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
 * The destructor free's any resources associated with the object
 */
SiEstimator::~SiEstimator()
{
    Cleanup();
}

////////////////////////////////////////////////////////////////////////////////
/**
 * This method configures the estimator with the supplied parameters.
 * NOTE: an estimator should be configured only once.
 */
void SiEstimator::Configure( 
    int fftSize, 
    int winType, 
    int fast,
    int mbins,
    int sbins
  )
{
    int idx;

    // Get rid of any previous work space
    Cleanup();

    // Setup the resources based on approach chosen
    if( fast ) mUseFftw = 1;
    else       mUseFftw = 0;

    // Establish the common vectors and parameters
    mNfftPoints = fftSize;
    mInOut      = (double*)malloc( mNfftPoints * sizeof(double) );
    mWin        = (double*)malloc( mNfftPoints * sizeof(double) );
    mTmp        = (double*)malloc( 2*sizeof(double)*mNfftPoints );

    // Establish the window function
    if( 0==winType ){
        for(idx=0;idx<mNfftPoints;idx++){
            mWin[idx] = 1.0;
        }
        mCoherentGain = 1.0;
    }
    else if( 1==winType ){
       double x,a0,a1,a2,sum;
       a0 = 0.42659;
       a1 = 0.49656;
       a2 = 0.076849;
       sum = 0.0;
       for(idx=0;idx<mNfftPoints;idx++){
           x = (TWOPI * idx)/(mNfftPoints-1);
           mWin[idx] = a0 - a1*cos(x) + a2*cos(2*x);
           sum+=mWin[idx];
       }
       mCoherentGain = sum / mNfftPoints;
    }

    // Establish the appropriate processing vectors depending on approach
    if( mUseFftw ){
        mFftwOutput = (fftw_complex*)fftw_malloc( 
                                        sizeof(fftw_complex) * mNfftPoints);
        mFftwPlan   = fftw_plan_dft_r2c_1d(mNfftPoints,mInOut,mFftwOutput,0);
        if( mLog & M2_LOG_PLAN ){ 
            fftw_print_plan( mFftwPlan );
            printf("\n");
        }
    }
    else{
        mNrInOut = (double*)malloc( 2*sizeof(double)*mNfftPoints );
    }

    mMeasureBins = mbins;
    mSumBins     = sbins;
    mBinAve      = 0;
    // printf("CFG: mMeasureBins=%d, mSumBins=%d\n",mMeasureBins,mSumBins);
}

////////////////////////////////////////////////////////////////////////////////
/**
 * This method is given a pointer to nFft short input samples and calculates
 * a power spectral estimate of the provided samples.
 */
int SiEstimator::Pse( short *input, int input_samples )
{
    int    n;
    short *src;
    int    nSamples;
    int    fftCount;

    if( mLog & M2_LOG_INPUT ){
        for(n=0;n<mNfftPoints;n++){
            printf("t=%d %d\n",n,input[n]);
        }
    }

    fftCount = 0;

    // FFTW Case
    if( mUseFftw ){
        src      = input;
        nSamples = input_samples;
        while( nSamples>=mNfftPoints ){
           for(n=0;n<mNfftPoints;n++){
              mInOut[n] = src[n] * mWin[n];
           }
           src      += (mNfftPoints/2);
           nSamples -= (mNfftPoints/2);
           fftCount++;

           fftw_execute( mFftwPlan );
           if( 1==fftCount ){
               for(n=0;n<mNfftPoints/2;n++){
                   mTmp[2*n]   = mFftwOutput[n][0];
                   mTmp[2*n+1] = mFftwOutput[n][1];
               }
           }else{
               for(n=0;n<mNfftPoints/2;n++){
                   mTmp[2*n]   += mFftwOutput[n][0];
                   mTmp[2*n+1] += mFftwOutput[n][1];
               }
           }
        }
    }

    // NR Case
    else{
        src      = input;
        nSamples = input_samples;
        while( nSamples>=mNfftPoints ){
           for(n=0;n<mNfftPoints;n++){
               mNrInOut[ 2*n     ] = src[n] * mWin[n];
               mNrInOut[ 2*n + 1 ] = 0.0;
           }
           src      += (mNfftPoints/2);
           nSamples -= (mNfftPoints/2);
           fftCount++;

           four1(mNrInOut - 2,mNfftPoints,1);
           if( 1==fftCount ){
               for(n=0;n<mNfftPoints/2;n++){
                   mTmp[2*n]   = mNrInOut[2*n];
                   mTmp[2*n+1] = mNrInOut[2*n+1];
               }
           }else{
               for(n=0;n<mNfftPoints/2;n++){
                   mTmp[2*n]   += mNrInOut[2*n];
                   mTmp[2*n+1] += mNrInOut[2*n+1];
               }
           }
        }
    }

    // Create final mag squared
    for(n=0;n<mNfftPoints/2;n++){
       mInOut[n] = mTmp[2*n  ]*mTmp[2*n  ] + 
                   mTmp[2*n+1]*mTmp[2*n+1];
    }

    if( mLog & M2_LOG_OUTPUT ){
        for(n=0;n<mNfftPoints/2;n++){
            printf("f=%d %f\n",n,mInOut[n]);
        }
    }
    return( fftCount );
}

////////////////////////////////////////////////////////////////////////////////
int SiEstimator::Estimate( 
  short  *aInputSamples, 
  int     sampleCount, 
  double *magOut )
{
    int    dstIdx,sumN;
    int    srcIdx,srcN;
    int    fftCount;
    double normalization;
    double m2;

    // Calculate the spectral estimate using the samples provided
    fftCount = Pse( aInputSamples, sampleCount );

    // 
    //
    // +------------------------------------------------------------------+
    // | 0 |..       ..| N/8 |  ...  | N/4 |  ...   | N*3/8 |  ...  | N/2 |
    // +------------------------------------------------------------------+
    //                    |                             |
    //                    |<--------- measure bins ---->| 
    //
    srcN   = mMeasureBins;
    srcIdx = (mNfftPoints/4) - (srcN/2);
    sumN   = mSumBins;

    // printf("EST:mNfftPoints=%d mMeasureBins=%d, mSumBins=%d mBinAve=%d\n",
    //             mNfftPoints, mMeasureBins,mSumBins,mBinAve);

    // Clamp summming
    if( sumN<1 )    sumN=1;
    if( sumN>srcN ) sumN=srcN;

    // Calculate normalization factor to account for fft size, db relative
    // to full scale 16 bit number, windowing attenuation.
    normalization =   20*log10( mNfftPoints ) 
                    + 20*log10( (double)(2<<16)/TWOPI )
                    + 20*log10( mCoherentGain )
                    + 10*log10( fftCount );

    // The above normalization will produce a total power estimate.  If
    // we want an average power estimate then we also need to normalize
    // for the number of bin's summed together in the estimate.
    if( mBinAve ){
        normalization += 10*log10( sumN );
    }
    
    // Loop over the source points summing as appropriate and converting
    // to dBfs16
    dstIdx = 0;
    while( srcN>0 ){
        int n;
        m2 = 0.0;
        for(n=0;n<sumN;n++){
           m2 += mInOut[srcIdx];
           srcIdx++;
           srcN--;
        }
        magOut[ dstIdx ] = 10*log10( m2 ) - normalization; 
        dstIdx++;
    }

    // Return the number of points actuall produced
    return(dstIdx);
}

