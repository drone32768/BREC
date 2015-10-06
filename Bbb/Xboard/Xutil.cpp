#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <fftw3.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "Xboard.h"

////////////////////////////////////////////////////////////////////////////////
void Show2kIQ( short *bf, char fmt )
{
    int idx;
    
    if( 'x'==fmt ){
        for(idx=0;idx<2048;idx+=2){
            printf("CSV, %d, 0x%hx, 0x%hx, %d, %d\n",
               idx,(unsigned short)bf[idx],(unsigned short)bf[idx+1],
               (unsigned short)bf[idx],(unsigned short)bf[idx+1] 
               ); 
        }
    }
    else if ('d'==fmt) {
        for(idx=0;idx<2048;idx+=2){
            printf("CSV, %d, %d, %d\n",idx,bf[idx],bf[idx+1] ); 
        }
    }
    else if ( 'D'==fmt) {
        for(idx=0;idx<2048;idx+=1){
            printf("CSV, %d, %hd\n",idx,bf[idx]);
        }
    }
    else{
       printf("Show2kIQ::unknown print format\n");
    }
}

////////////////////////////////////////////////////////////////////////////////
int IqpTest_Check2kPattern(unsigned short *ubf, unsigned short *key, int reset )
{
    int            idx,nErrs;
    unsigned short expt,find;

    if( reset ){
       *key = ubf[0] - 100;
    }

    nErrs = 0;
    for(idx=0;idx<2048;idx+=2){
        expt =  (*key + 100 ); // I(n) to I(n+1) = 100;
        find =  ubf[idx];
        if( expt!=find ){
            printf("%d seq error : expected 0x%hx found 0x%hx\n",idx,expt,find);
            nErrs++;
        }

        expt =  ubf[idx+1] + 35;
        find =  ubf[idx];
        if( expt!=find ){
            printf("%d iq phase : expected 0x%hx found 0x%hx\n",idx,expt,find);
            nErrs++;
        }
        *key = ubf[idx]; 
    }
    return( nErrs );
}

void IqpTest (Xboard *xbrd )
{
    unsigned short ubf[4096];
    unsigned short key;
    int            nErrs,cnt,icnt,us;
    int            reset;
    unsigned short pinc;
    struct timeval tv1,tv2;

    printf("Starting iq pattern test\n");

    xbrd->SetLoFreqHz( 0 ); 
    xbrd->SetTpg( 1 );
    xbrd->SetSource( 7 );

    cnt       = 0;
    reset     = 1;
    pinc      = 1;
    icnt      = 0;
    gettimeofday( &tv1, NULL );
    while( 1 ){
        xbrd->Get2kSamples( (short*)ubf );
        nErrs=IqpTest_Check2kPattern(ubf,&key,reset);
        if( nErrs ){
            Show2kIQ( (short*)ubf, 'x' );
            return;
        }
        cnt++;
        icnt++;
        reset=0;

        if( 0==(cnt%300) ){
           gettimeofday( &tv2, NULL );
           us = tv_delta_useconds( &tv2, &tv1 );

           printf("%8d 2k words checked, t=%8d uS, i=%6d, %f k word/sec\n",
                         cnt,us,icnt,(2.0*1e6*icnt/(double)us) );
           icnt = 0;
           gettimeofday( &tv1, NULL );
        }
        if( 0==(cnt%4000) ){
           printf("changing lo...\n");
           xbrd->SetLoFreqHz( pinc ); 
           pinc = (pinc+1)%4096;
           reset= 1;
        }
    }

}

////////////////////////////////////////////////////////////////////////////////
void Histogram (Xboard *xbrd )
{
    short          bf[4096];

    xbrd->SetSource( 0 );
    xbrd->FlushSamples();
    xbrd->Get2kSamples( bf );
}

////////////////////////////////////////////////////////////////////////////////
int ShowPhases( short *bf, int nPts )
{
    int   idx;
    double max;
    double iphase,qphase,dphase,tphase,mag;
    int    nErrs;

    // Find the maximum
    nErrs = 0;
    max   = -1;
    for(idx=0;idx<nPts;idx++){
       if( bf[idx] > max ) max = bf[idx];
    }
    printf("Max = %f\n",max);

    printf("?,CSV,idx,I,Q,Iph,Qph,del-ph,ph,m^2\n");

    // Loop through data calculating phases
    for(idx=0;idx<nPts;idx+=2){
       iphase = asin( (double)bf[idx]   / max );
       qphase = acos( (double)bf[idx+1] / max );
       dphase = iphase - qphase;
       tphase = atan2( (double)bf[idx], (double)bf[idx+1] );
       mag    = (double)bf[idx] * (double)bf[idx] 
                + (double)bf[idx+1]*(double)bf[idx+1];

       if( (dphase > -1.56) || (dphase < -1.58) ){
          nErrs++;
          printf("E,CSV,");
       }
       else{
          printf(" ,CSV,");
       }
       printf("%3d,%5d,%5d,%10f,%10f,%10f,%10f, %f\n",idx,bf[idx],bf[idx+1],
                          iphase,qphase,dphase,tphase,mag);
    }

    return( nErrs );
}

void QuadTest (Xboard *xbrd )
{
    short          bf[4096];

    xbrd->SetLoFreqHz( 1 ); 
    xbrd->SetSource( 7 );

    xbrd->FlushSamples();
    xbrd->Get2kSamples( bf );
    ShowPhases( bf, 2048 );
}

////////////////////////////////////////////////////////////////////////////////
double Mag2k(short *bf)
{
    int    idx;
    double m2 =0.0;
    for(idx=0;idx<2048;idx+=2){
       m2 += (bf[idx]*bf[idx] + bf[idx+1]*bf[idx+1]);
    }
    return(m2);
}

static fftw_complex *mFftwOutput = NULL;
static fftw_plan     mFftwPlan;
static double       *mWin;

double Peak2k(short *bf)
{
    int    idx;
    int    fftSize = 1024;
    double max,m2;

    // if first fft setup
    if( NULL==mFftwOutput ){

       mFftwOutput = (fftw_complex*)fftw_malloc(
                                  sizeof(fftw_complex)*fftSize );

       mFftwPlan = fftw_plan_dft_1d(
                  fftSize,
                  mFftwOutput,
                  mFftwOutput,
                  FFTW_FORWARD,
                  FFTW_ESTIMATE );

       mWin = (double*)malloc( sizeof(double) * fftSize );
       double x,a0,a1,a2,sum;
       a0 = 0.42659;
       a1 = 0.49656;
       a2 = 0.076849;
       sum = 0.0;
       for(idx=0;idx<fftSize;idx++){
           x = (2.0*M_PI * idx)/(fftSize-1);
           mWin[idx] = a0 - a1*cos(x) + a2*cos(2*x);
           sum+=mWin[idx];
       }

    }

    // setup fft input
    for(idx=0;idx<fftSize;idx++){
       mFftwOutput[idx][0] = bf[2*idx +1] * mWin[idx];
       mFftwOutput[idx][1] = bf[2*idx   ] * mWin[idx];
    }

    // execute fft
    fftw_execute( mFftwPlan );

    // get max mag2
    max = -1000;
    for(idx=0;idx<fftSize;idx++){
       m2 = (mFftwOutput[idx][0] * mFftwOutput[idx][0]) +
            (mFftwOutput[idx][1] * mFftwOutput[idx][1]);
       if( m2>max ) max=m2;
    }
  
    // return peak value
    return( max );
}

void FilterScanTest (Xboard *xbrd, int wide )
{
    short          bf[4096];
    double         f,fstart,fend,fstep;
    double         m2,m2max;

    xbrd->SetTpg( 2 );
    xbrd->SetSource( 7 );

    fstart= 1.25e6;
    if( wide ){
        fend  = fstart + 500e3;
        fstep = 1e3;
    }
    else{
        fend  = fstart + 50e3;
        fstep = 100;
    }
    f     = fstart;
    printf("CSV,f(hz),m^2,del(Hz),10log10(m2),10log10(m2/m2[0])\n");
    while( f<fend ){
        xbrd->SetLoFreqHz( f ); 
        xbrd->FlushSamples();
        xbrd->Get2kSamples( bf );
        // m2 = Mag2k(bf);
        m2 = Peak2k(bf);
        if( f==fstart ){
           m2max = m2;
        }
        printf("CSV, %f, %f, %f, %f, %f\n",
                 f,m2,f-fstart,10*log10(m2),10*log10(m2/m2max));
        f += fstep;
    }

}

////////////////////////////////////////////////////////////////////////////////
void usage( int exit_code )
{
    printf("This utility uses the X board device interface software");
    printf("-echo Msg     Echo's Msg to output\n");
    printf("-open         open's device (required first step)\n");
    printf("-usleep N     sleeps N microseconds\n");
    printf("-write  N     writes N to spi port\n");
    printf("-samp         show samples\n");
    printf("-csv          produce samples in csv format\n");
    printf("-flush        flush source fifo\n");
    printf("-pru          show pru state\n");
    printf("-iqp          execute IQ pattern test\n");
    printf("-quad         execute quadrature data test\n");
    exit( exit_code );
}

int
main( int argc, char *argv[] )
{
    int            idx;	  
    Xboard        *xbrd;
    int            val;
    char          *end;
    short          bf[4096];

    xbrd = NULL;

    idx = 1;
    while( idx < argc ){

        if( 0==strcmp(argv[idx], "-help") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-h") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-echo") ){
            if( (idx+1) >= argc ){ usage(-1); }
            printf("%s\n",argv[idx+1]);
        } 

        else if( 0==strcmp(argv[idx], "-open") ){
            xbrd = new Xboard();
            xbrd->Open();
            xbrd->StartPrus();
            xbrd->SetComplexSampleRate( 5000000 );
        }		 

        else if( 0==strcmp(argv[idx], "-usleep") ){
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);
            us_sleep( val );
        } 

        else if( 0==strcmp(argv[idx], "-write") ){
            int rval; 
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol( argv[idx+1], &end, 0 );

            rval = xbrd->XspiWrite( val );
            printf("Xutil:w=0x%04hx r=0x%04hx (%hd)\n",val,rval,rval);
        }
    
        else if( 0==strcmp(argv[idx], "-samp") ){
            xbrd->Get2kSamples( bf );
            Show2kIQ(bf,'x');
        }

        else if( 0==strcmp(argv[idx], "-csv") ){
            Show2kIQ(bf,'D');
        }

        else if( 0==strcmp(argv[idx], "-flush") ){
            xbrd->FlushSamples();
        }

        else if( 0==strcmp(argv[idx], "-pru") ){
            xbrd->ShowPrus( "Xutil: pru state" );
        }

        else if( 0==strcmp(argv[idx], "-iqp") ){
            IqpTest( xbrd );
        }

        else if( 0==strcmp(argv[idx], "-quad") ){
            QuadTest(xbrd);
        }

        else if( 0==strcmp(argv[idx], "-filter-scan-wide") ){
            FilterScanTest(xbrd,1);
        }

        else if( 0==strcmp(argv[idx], "-filter-scan-narrow") ){
            FilterScanTest(xbrd,0);
        }
 
        ////////////////////////////////////////////////
        // the following tests need to be revisited ...

        else if( 0==strcmp(argv[idx], "-histo") ){
            Histogram(xbrd);
        }

        else if( 0==strcmp(argv[idx], "-ntest") ){
            short bf[4096];
            short lastGood;
            int   idx, ns,goodCount;

            goodCount = 0;
            lastGood  = 0;
            ns = 0;
            while( 1 ){
                xbrd->FlushSamples();
                xbrd->Get2kSamples( bf );
                for(idx=0;idx<2048;idx++){
                   if( (bf[idx] > 2080) || (bf[idx]<2020) ){
                   // if( (bf[idx] > 2200) || (bf[idx]<1700) ){
                       printf("%d  %hd 0x%x (lg=%hd) gc=%d\n",
                               ns,bf[idx],bf[idx],lastGood,goodCount);
                       goodCount = 0;
                   }
                   else{
                       lastGood = bf[idx];
                       goodCount++;
                   }
                   ns++;
                }
            }
        }

        // Move to next argument for parsing
        idx++;
    }

}

