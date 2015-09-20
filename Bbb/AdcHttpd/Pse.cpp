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
Pse::Pse()
{
    mLog        = 0; // (PSE_LOG_PCOHI | PSE_LOG_OUTX | PSE_LOG_OUTY) ; 
    mMaxFft     = 32768;
    mMaxSamp    = 8*mMaxFft;
    mInput      = (short*)malloc(  2 * mMaxSamp * sizeof(short)  );
    mFftSum     = (double*)malloc( 2 * mMaxFft  * sizeof(double) );
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

    // No matter what, zero the running fft sum
    for(idx=0;idx<fftSize;idx++){
       mFftSum[ 2*idx    ] = 0.0;
       mFftSum[ 2*idx +1 ] = 0.0;
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
Pse::PerformFft( int complexInput, int fftSize, short *inputData, int nave )
{
    int    idx;
    short *newHead;

    if( mLog&PSE_LOG_FFT ){
        printf("Pse:PerformFft: Copying input\n");
    }

    // Prepare the input samples
    if( complexInput ){
       for(idx=0;idx<fftSize;idx++){
          mFftwOutput[idx][0] = inputData[2*idx    ] * mWin[idx];
          mFftwOutput[idx][1] = inputData[2*idx + 1] * mWin[idx];
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
            printf("Pse:mInOut(in)[%d] %g %g\n",
                idx,mFftwOutput[idx][0],mFftwOutput[idx][1] );
        }
    }

    // Perform the fft
    fftw_execute( mFftwPlan );


    // Add the result into running coherent average
    for(idx=0;idx<fftSize;idx++){
        mFftSum[ 2*idx    ] += (mFftwOutput[idx][0] );
        mFftSum[ 2*idx + 1] += (mFftwOutput[idx][1] );
    }
 
    return( newHead );
}

void
Pse::PerformOutputX( int isComplex, int fftSize, double *aXvec, int csps )
{
    int    idx;
    double last;
    int    nPts;

    nPts = fftSize / 2;
 
    if( isComplex ){
        aXvec[0] = - (csps/2.0);
        last     = + (csps/2.0);
    }
    else{
        aXvec[0] = 0.0;
        last     = csps;
    }

    if( mLog&PSE_LOG_OUTX ){
        printf("Pse:PerformOutputX: pts=%d,csps=%d,cmplx=%d\n",
               nPts,csps,isComplex);
    }

    for(idx=1;idx<nPts;idx++){
        aXvec[idx] = aXvec[0] + ((double)idx * (double)csps)/(double)nPts;
    }
    aXvec[idx-1] = last; // Force the last bin to end of extent

}

void
Pse::PerformOutputY( int isComplex, int fftSize, double *aYvec, int aNave )
{
    double magSqr,normalize;
    int    nPts;
    int    didx,sidx;


    // Update Y values
    normalize =   20*log10( 32768 )            // Ref 16 signed bit
                  + 20*log10( mCoherentGain )  // Remove Windowing
                  + 20*log10( fftSize )        // Remove  1/N^2 from dft
                  + 20*log10( aNave )          // Remove Averaging 
                ;

    // Loop over output creating dBFS16
    nPts = fftSize/2;
    sidx = isComplex?(fftSize/4):0;
    didx = 0;
    if( mLog&PSE_LOG_OUTY ){
        printf("Pse:PerformOutputY: fft=%d,cmplx=%d,norm=%f\n",
                     fftSize,isComplex,normalize);
    }
    while( didx < nPts ){

       magSqr = (mFftSum[2*sidx]     * mFftSum[2*sidx]    )
                +
                (mFftSum[2*sidx + 1] * mFftSum[2*sidx + 1]);

       aYvec[didx] = 10*log10(magSqr) - normalize;

       didx++;
       sidx++;
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
    int    cnt,lim;
    int    csps;
    int    isComplex;
 
    int    fftSize;

    // Get sample rate
    csps = Dp()->Adc()->GetComplexSampleRate();

    // Get format
    isComplex = Dp()->Adc()->IsComplexFmt();

    // Based on the desire number of points and the
    // format of the input data figure out the fft size
    fftSize = 2 * aPts;

    if( mLog&PSE_LOG_PCOHI ){
        printf("Pse:nav=%d,pts=%d,fft=%d,cmplx=%d,csps=%d\n",
                aNave,aPts,fftSize,isComplex,csps);
    }

    // Sanity check args to prevent downstream ambiguous errors
    if( (fftSize*aNave) >= mMaxSamp ){
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
    // The adc interface is really 2k sample words so
    // in complex cases we are really only getting 1k complex
    // samples each time.
    dst = mInput;
    cnt = 0;
    lim = fftSize * aNave;
    lim = lim * (isComplex?2:1);
    Dp()->Adc()->FlushSamples();
    while( cnt<lim ) {
       Dp()->Adc()->Get2kSamples( dst ); 
       dst += 2048;
       cnt += 2048;
    }

    if( mLog&PSE_LOG_PCOHI ){
        printf("Pse::ProcessCoherentInterval: %d words, %d samples, %d coll\n",
                     lim, fftSize*aNave,cnt);
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
       src = PerformFft( isComplex, fftSize, src, aNave );
       cnt = cnt + 1;
    }

    // Format and output the X values
    PerformOutputX( isComplex, fftSize, aXvec, csps );

    // Format and output the Y values
    PerformOutputY( isComplex, fftSize, aYvec, aNave );

}

