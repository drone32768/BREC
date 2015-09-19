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
   Pse();

   void ProcessCoherentInterval(
      int     aNave,
      int     aFftSize,
      double *aXvec,
      double *aYvec
    );

    void   PerformSetup(   int winType,   int fftSize );
    short* PerformFft(     int isComplex, int fftSize, short *src );
    void   PerformOutputX( int isComplex, int fftSize, double *aXvec,int csps );
    void   PerformOutputY( int isComplex, int fftSize, double *aYvec );
};
#endif /* __PSE__ */
