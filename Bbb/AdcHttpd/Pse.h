////////////////////////////////////////////////////////////////////////////////
/// Processing  ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*
The key input is number of points

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

   double  *mWin;      // Windowing function, real
   double  *mInOut;    // Fft working space, complex i/q
   double  *mFftSum;   // Coherent fft sum

   int      mCurFftSize;   // Currently configured fft size
   int      mCurWinType;   // Currently configured window type
   double   mCoherentGain; // Current coherent gain

   int      mFftCount;
public:
   Pse();

   void ProcessCoherentInterval(
      int     aNave,
      int     aFftSize,
      double *aXvec,
      double *aYvec
    );

    void   PerformSetup( int winType, int fftSize );
    short* PerformFft( int isComplex, short *src, int fftSize );
    void   PerformOutputX( double *aXvec, int nPts, int csps,  int isComplex );
    void   PerformOutputY( double *aYvec, int fftSize, int isComplex );
};
