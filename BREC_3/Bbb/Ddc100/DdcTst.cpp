#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <fftw3.h>

#include "Util/mcf.h"
#include "Ddc100.h"

////////////////////////////////////////////////////////////////////////////////
void Show2kIQ( short *bf, char fmt )
{
    int idx;
    
    if( 'x'==fmt ){
        for(idx=0;idx<2048;idx+=2){
            printf("CSV, %d, 0x%04hx, 0x%04hx, %d, %d\n",
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
int IqpTest_Check2kPattern(
   unsigned short *ubf, 
   unsigned short *key, 
   int             reset,
   int             showErrs )
{
    int            idx,nErrs;
    unsigned short expt,find;

    if( reset ){
       *key = ubf[0];
    }

    nErrs = 0;
    for(idx=0;idx<2048;idx++){
        expt =  (*key);
        find =  ubf[idx];

        // TODO - every 256 samples the 
        if( (idx%256)==0 ){ 
            expt = find;
        }

        if( expt!=find ){
            nErrs++;
            if( showErrs ){
                printf("%d seq error : expected 0x%hx found 0x%hx\n",
                              idx,expt,find);
            }
        }

        *key = ubf[idx] + 1; 
    }
    return( nErrs );
}

void IqpTest (Ddc100 *ddc )
{
    unsigned short ubf[4096];
    unsigned short key;
    int            nErrs,cnt,wcnt,us;
    int            reset;
    struct timeval tv1,tv2;
    int            errs;
    int            showErrs;
    int            stopOnErr;

    printf("Starting iq pattern test\n");

    cnt       = 0;
    reset     = 1;
    wcnt      = 0;
    errs      = 0;
    showErrs  = 1; //
    stopOnErr = 0; //
    key       = 0;
    ddc->FlushSamples();
    gettimeofday( &tv1, NULL );

    while( 1 ){

        // ddc->Get2kSamples( (short*)ubf );

        ddc->Get2kSamples( (short*)ubf );

        nErrs=IqpTest_Check2kPattern(ubf,&key,reset,showErrs);

        if( nErrs ){
            errs++;
            reset = 1;
            if( stopOnErr ){
                Show2kIQ( (short*)ubf, 'x' );
                return;
            }
        }

        cnt++;
        wcnt+=2048;
        reset=0;

        if( 0==(cnt%300) ){
           gettimeofday( &tv2, NULL );
           us = tv_delta_useconds( &tv2, &tv1 );

           printf("%8d 2k wrd checked,t=%8d uS,i=%d, %f MW/s, errs=%d\n",
                         cnt,us,wcnt,(wcnt/(double)us),errs );
           wcnt = 0;
           gettimeofday( &tv1, NULL );
        }

    } // End of infinite processing loop

}

////////////////////////////////////////////////////////////////////////////////
void Histogram (Ddc100 *ddc )
{
    short          bf[4096];

    ddc->SetSource( 0 );
    ddc->FlushSamples();
    ddc->Get2kSamples( bf );
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

void QuadTest (Ddc100 *ddc )
{
    short          bf[4096];

    ddc->SetLoFreqHz( 1 ); 
    ddc->SetSource( 7 );

    ddc->FlushSamples();
    ddc->Get2kSamples( bf );
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

void FilterScanTest (Ddc100 *ddc, int wide )
{
    short          bf[4096];
    double         f,fstart,fend,fstep;
    double         m2,m2max;

    ddc->SetTpg( 2 );
    ddc->SetSource( 7 );

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
        ddc->SetLoFreqHz( f ); 
        ddc->FlushSamples();
        ddc->Get2kSamples( bf );
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
    printf("-rw16   N     writes N to spi port and read results\n");
    printf("-samp         show samples\n");
    printf("-csv          produce samples in csv format\n");
    printf("-flush        flush source fifo\n");
    printf("-prustart     start pru\n");
    printf("-iqp          execute IQ pattern test\n");
    printf("-quad         execute quadrature data test\n");
    printf("-show         show device state\n");
    exit( exit_code );
}

int
main( int argc, char *argv[] )
{
    Bdc           *bdc;
    Ddc100        *ddc;
    int            idx;	  
    int            val;
    char          *end;
    short          bf[4096];

    int            ioff = 0;
    int            qoff = 0;
    double         igain= 1.0;
    double         qgain= 1.0;

    bdc = new Bdc();
    bdc->Open();

    ddc = new Ddc100();
    ddc->Attach( bdc );
    ddc->Open();
    ddc->SetComplexSampleRate( 5000000 );

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

        else if( 0==strcmp(argv[idx], "-usleep") ){
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);
            us_sleep( val );
        } 

        else if( 0==strcmp(argv[idx], "-rw16") ){
            int rval; 
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol( argv[idx+1], &end, 0 );

            rval = bdc->SpiRW16( val );
            printf("DdcTst:w=0x%04hx r=0x%04hx (%hd)\n",val,rval,rval);
        }

        else if( 0==strcmp(argv[idx], "-write") ){
            int rg,wr; 
            if( (idx+2) >= argc ){ usage(-1); }
            rg = strtol( argv[idx+1], &end, 0 );
            wr = strtol( argv[idx+1], &end, 0 );
            bdc->SpiRW16( BDC_REG_WR | ((rg&0x3f)<<8) | (wr&0xff) );
            printf("DdcTst:write rg %d = 0x%04hx (%hd)\n",rg,wr,wr);
        }

        else if( 0==strcmp(argv[idx], "-read") ){
            int rval,rg; 
            if( (idx+1) >= argc ){ usage(-1); }
            rg = strtol( argv[idx+1], &end, 0 );

            rval = bdc->SpiRW16( BDC_REG_RD | ((rg&0x3f)<<8) );
            rval = bdc->SpiRW16( 0 );
            printf("DdcTst:read rg %d = 0x%04hx (%hd)\n",rg,rval,rval);
        }

        else if( 0==strcmp(argv[idx], "-src") ){
            int src; 
            if( (idx+1) >= argc ){ usage(-1); }
            src = strtol( argv[idx+1], &end, 0 );
            ddc->SetSource( src );
        }
    
        else if( 0==strcmp(argv[idx], "-tpg") ){
            int tpg; 
            if( (idx+1) >= argc ){ usage(-1); }
            tpg = strtol( argv[idx+1], &end, 0 );
            ddc->SetTpg( tpg );
        }
    
        else if( 0==strcmp(argv[idx], "-ioff") ){
            if( (idx+1) >= argc ){ usage(-1); }
            ioff = strtol( argv[idx+1], &end, 0 );
        }

        else if( 0==strcmp(argv[idx], "-qoff") ){
            if( (idx+1) >= argc ){ usage(-1); }
            qoff = strtol( argv[idx+1], &end, 0 );
        }

        else if( 0==strcmp(argv[idx], "-igain") ){
            if( (idx+1) >= argc ){ usage(-1); }
            igain = atof( argv[idx+1] );
        }

        else if( 0==strcmp(argv[idx], "-qgain") ){
            if( (idx+1) >= argc ){ usage(-1); }
            qgain = atof( argv[idx+1] );
        }

        else if( 0==strcmp(argv[idx], "-match") ){
            ddc->SetChannelMatch(ioff,igain,qoff,qgain);
        }

        else if( 0==strcmp(argv[idx], "-csvx") ){
            ddc->Get2kSamples( bf );
            Show2kIQ(bf,'x');
        }

        else if( 0==strcmp(argv[idx], "-csvd") ){
            ddc->Get2kSamples( bf );
            Show2kIQ(bf,'d');
        }

        else if( 0==strcmp(argv[idx], "-flush") ){
            ddc->FlushSamples();
        }

        else if( 0==strcmp(argv[idx], "-prustart") ){
            ddc->StartPru();
        }

        else if( 0==strcmp(argv[idx], "-show") ){
            ddc->Show("DdcTst:\n");
        }

        else if( 0==strcmp(argv[idx], "-iqp") ){
            IqpTest( ddc );
        }

        else if( 0==strcmp(argv[idx], "-quad") ){
            QuadTest(ddc);
        }

        else if( 0==strcmp(argv[idx], "-filter-scan-wide") ){
            FilterScanTest(ddc,1);
        }

        else if( 0==strcmp(argv[idx], "-filter-scan-narrow") ){
            FilterScanTest(ddc,0);
        }
 
        ////////////////////////////////////////////////
        // the following tests need to be revisited ...

        else if( 0==strcmp(argv[idx], "-histo") ){
            Histogram(ddc);
        }

        else if( 0==strcmp(argv[idx], "-ntest") ){
            short bf[4096];
            short lastGood;
            int   idx, ns,goodCount;

            goodCount = 0;
            lastGood  = 0;
            ns = 0;
            while( 1 ){
                ddc->FlushSamples();
                ddc->Get2kSamples( bf );
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

