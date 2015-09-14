#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "../Util/mcf.h"
#include "Devs.h"
#include "Pse.h"

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

Pse::Pse()
{
    mLog        = 0x0;
    mMaxSamp    = 2*65536;
    mMaxFft     = 32768;
    mInput      = (short*)malloc(      mMaxSamp * sizeof(short)  );
    mWin        = (double*)malloc(     mMaxFft  * sizeof(double) );
    mInOut      = (double*)malloc( 2 * mMaxFft  * sizeof(double) );
    mFftSum     = (double*)malloc( 2 * mMaxFft  * sizeof(double) );
    mCurFftSize = -1; // invalid, ensure setup executes
    mCurWinType = -1; // invalid, ensure setup executes
}

void
Pse::PerformSetup( int winType, int fftSize )
{
    int idx;

    if( mLog&PSE_LOG_SETUP ){
       printf("Pse:PerformSetup: Clear history win=%d, fft=%d\n",
                              winType,fftSize);
    }

    // No matter what, zero the running fft sum
    for(idx=0;idx<fftSize;idx++){
       mFftSum[ 2*idx    ] = 0.0;
       mFftSum[ 2*idx +1 ] = 0.0;
    }
    mFftCount=0;

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
           x = (TWOPI * idx)/(mCurFftSize-1);
           mWin[idx] = a0 - a1*cos(x) + a2*cos(2*x);
           sum+=mWin[idx];
       }
       mCoherentGain = sum / mCurFftSize;
    }
}

short * 
Pse::PerformFft( int complexInput, short *inputData, int fftSize )
{
    int    idx;
    short *head;

    if( mLog&PSE_LOG_FFT ){
        printf("Pse:PerformFft: Copying input\n");
    }

    // Prepare the input samples
    if( complexInput ){
       for(idx=0;idx<fftSize;idx++){
          mInOut[2*idx+1] = inputData[2*idx    ] * mWin[idx];
          mInOut[2*idx+2] = inputData[2*idx + 1] * mWin[idx];
       }
       head = inputData + 2*fftSize;
    }
    else{
       for(idx=0;idx<fftSize;idx++){
          mInOut[2*idx+1] = inputData[idx    ] * mWin[idx];
          mInOut[2*idx+2] = 0.0;
       }
       head = inputData + fftSize;
    }

    if( mLog&PSE_LOG_FFT_IN ){
        for(idx=1;idx<fftSize;idx++){
            printf("Pse:mInOut(in)[%d] %g %g\n",
                idx,mInOut[2*idx],mInOut[2*idx+1] );
        }
    }

    // Perform the fft
    four1( mInOut,fftSize,1 );

    // Add the result into running coherent average
    for(idx=0;idx<fftSize;idx++){
        mFftSum[ 2*idx    ] += mInOut[2*idx + 1 ];
        mFftSum[ 2*idx + 1] += mInOut[2*idx + 2 ];
        mFftCount++;
    }
 
    return( head );
}

void
Pse::PerformOutputX( double *aXvec, int nPts, int csps, int isComplex )
{
    int idx;
    
    if( mLog&PSE_LOG_OUTX ){
        printf("Pse:PerformOutputX: pts=%d,csps=%d,cmplx=%d\n",
               nPts,csps,isComplex);
    }

    if( isComplex ){
        aXvec[0] = - (csps/2.0);
    }
    else{
        aXvec[0] = 0.0;
    }

    for(idx=1;idx<nPts;idx++){
        aXvec[idx] = aXvec[0] + ((double)idx * (double)csps)/(double)nPts;
    }
}

void
Pse::PerformOutputY( double *aYvec, int fftSize, int isComplex )
{
    double magSqr,normalize;
    int    nPts;
    int    idx;

    if( mLog&PSE_LOG_OUTY ){
        printf("Pse:PerformOutputY: fft=%d,cmplx=%d\n",fftSize,isComplex);
    }

    // Update Y values
    normalize =   20*log10( 32768 )            // Ref 16 signed bit
//                  + 10*log10( fftSize/4 )      // fft (if dft !removed 1/N^2)
                  + 20*log10( mFftCount )      // Averaging
                  + 20*log10( mCoherentGain )  // Windowing
                ;


    // Figure out if we are taking all of fft or non-redundant half
    nPts = isComplex?fftSize:(fftSize/2);

    // Loop over output creating dBFS16
    for( idx=0;idx<nPts;idx++ ){

       magSqr = (mFftSum[2*idx]     * mFftSum[2*idx]    )
                +
                (mFftSum[2*idx + 1] * mFftSum[2*idx + 1]);

       aYvec[idx] = 10*log10(magSqr) - normalize;
    }
}

void 
Pse::ProcessCoherentInterval( 
      int     aNave,
      int     aPts,
      double *aXvec,
      double *aYvec
    )
{
    short *dst;
    short *src;
    int    cnt;
    int    csps;
    int    isComplex;
 
    int    fftSize;

    // Get sample rate
    csps = Dp()->Adc()->GetComplexSampleRate();

    // Get format
    isComplex = Dp()->Adc()->IsComplexFmt();

    // Based on the desire number of points and the
    // format of the input data figure out the fft size
    if( isComplex ){
        fftSize = aPts;
    }
    else{
        fftSize = 2 * aPts;
    }

    if( mLog&PSE_LOG_PCOHI ){
        printf("Pse:nav=%d,pts=%d,fft=%d,cmplx=%d,csps=%d\n",
                aNave,aPts,fftSize,isComplex,csps);
    }

    // Sanity check args to prevent downstream ambiguous errors
    if( (aPts*aNave) >= mMaxSamp ){
        fprintf(stderr,"Pse::fft x ave > max samples\n");
        return;
    }
    if( fftSize <=0 ){
        fprintf(stderr,"Pse::fft <= 0\n");
        return;
    }
    if( aNave <=0 ){
        fprintf(stderr,"Pse::ave <= 0\n");
        return;
    }

    // Capture the coherent number of required samples
    dst = mInput;
    cnt = 0;
    Dp()->Adc()->FlushSamples();
    while( cnt<(aPts*aNave) ) {
       Dp()->Adc()->Get2kSamples( dst ); 
       dst += 2048;
       cnt += 2048;
    }

    if( mLog&PSE_LOG_PCOHI ){
        printf("Pse::ProcessCoherentInterval: %d coherent samples collected\n",
             cnt);
    }

    // Setup for fft's
    PerformSetup( 1 /* 0=no win, 1=win */, fftSize );

    // Loop over number of averages
    src = mInput;
    cnt = 0;
    while( cnt < aNave ){
       if( mLog&PSE_LOG_PCOHI ){
           printf("Pse::ProcessCoherentInterval:fft[%d] %d\n",cnt,fftSize);
       }
       src = PerformFft( isComplex, src, fftSize );
       cnt = cnt + 1;
    }

    // Format and output the X values
    PerformOutputX( aXvec, aPts, csps, isComplex );

    // Format and output the Y values
    PerformOutputY( aYvec, fftSize, isComplex );

}

