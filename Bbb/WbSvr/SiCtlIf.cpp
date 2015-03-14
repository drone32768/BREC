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
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#include "SiCtlIf.h"

//------------------------------------------------------------------------------
/**
The spectral investigator is used in two configurations.  The first
is as a single stage IF using a the B board for down conversion and 
the C board for signal generation to a device under test.  The second 
configuration uses a two stage conversion (B for the first, C for the 
second).  Those configurations are shown below.

Single Stage IF:

                       ------------------------
                      |                        |
    B board (Lo0)     |      C board(Lo1)      |        A board
    +-----------+     |     +-----------+      |    +-------------+
    |      Aux  |     |     |       Aux |--    |    |             |
    |           |     |     |           |  |   |    |             |
    |     IFout |-----      |     IFout |  |    --->| IFin   Net  |<--->
    |           |           |           |  |        |             |
    |      RFin |<----      |      RFin |  |        |             |
    |           |     |     |           |  |        |             |
    +-----------+     |     +-----------+  |        +-------------+
                      |                    | 
                      |     +-----------+  |
                       -----|    DUT    |<-
                            +-----------+

Two Stage IF:

    B board (Lo0)            C board(Lo1)            A board
    +-----------+           +-----------+           +-------------+
    |      Aux  |           |       Aux |           |             |
    |           |           |           |           |             |
    |     IFout |--------   |     IFout |---------->| IFin   Net  |<--->
    |           |        |  |           |           |             |
    |      RFin |<----   |  |      RFin |<-         |             |
    |           |     |  |  |           |  |        |             |
    +-----------+     |  |  +-----------+  |        +-------------+
                      |  |                 |
                      |   -----------------
                      |                  
                      |    +-----------+ 
                       ----|    DUT    |
                           +-----------+

Note that the physical cabling is different between the two configurations. 
The single stage conversion configuration can be used without the C board.

*/

//------------------------------------------------------------------------------
void SiCtlIf::ConfigureHw()
{
    // Dp()->Lo(0)->SetLog(0xffffffff);
    Dp()->Lo(0)->SetLog(0);
    Dp()->Lo(0)->SetAuxEnable( 0 );
    Dp()->Lo(0)->SetMainPower( 0 );

    // TODO need to decide how to handle local and remote secondar lo/tg

    // Dp()->Lo(1)->SetLog(0xffffffff);
    Dp()->Lo(1)->SetLog(0);
    Dp()->Lo(1)->SetAuxEnable( 0 );
    Dp()->Lo(1)->SetMainPower( 0 );
    Dp()->Lo(0)->SetMtld(      1 );

    Dp()->Adc()->SetGain( mIfGain );
    Dp()->Adc()->SetComplexSampleRate( 500000 );
}

//------------------------------------------------------------------------------
/**
 * This method sets the specified LO to the specified frequency in Hertz.
 * Prior to returning the method waits for the LO's lock indicator to 
 * become true.  This is evaluted over 100uS/0.1mS intervals for up to 
 * 1 second.
 *
 * Returns 0 on successful frequency set and lock, non zero on failure.
 *
 */
double SiCtlIf::SetLoWithLockWait( int lon, double fHz )
{
    int    cnt;
    double actHz; 

    actHz = Dp()->Lo(lon)->SetFrequency(  (int64_t)(fHz) );

    cnt = 0;
    while( !Dp()->Lo(lon)->GetLock() ){
        mLockWait++;
        us_sleep( 100 );
        cnt++;
        if( cnt > 200 ){ // 200 * 100uS = 20mS
            mLockWaitAbort++;		 
            return( -1 );
      	    break;
	}     
    }

    return( actHz );
}

//------------------------------------------------------------------------------
double SiCtlIf::SetRemoteTG( double fHz )
{
    // TODO - resolve how we know which syn is present/useable
    mRemoteTg.SetFreqHz( 1, fHz );
    return(0);
}

//------------------------------------------------------------------------------
double SiCtlIf::SetRemoteLO( int ln, double fHz )
{
    double fAct0,fAct1;
 
    fAct0 = mRemoteLo[ln].SetFreqHz( 0, fHz );
    fAct1 = mRemoteLo[ln].SetFreqHz( 1, fHz );

    if( fAct0 > 0.0 ){
       fHz = fAct0;
    }
    if( fAct1 > 0.0 ){
       fHz = fAct1;
    }
    printf("SiCtlIf::SetRemoteLO[%d]: %f %f\n",ln,fAct0,fAct1);

    return( fHz );
}

//------------------------------------------------------------------------------
SiCtlIf::SiCtlIf()
{
    mThreadExit = 0;
    mIpPort     = 0;
    mRun        = 0;

    mFftSize        = 1024;
    mFminHz         = 100.000*1e6;
    mFmaxHz         = 100.250*1e6;
#   define MAX_SAMPLES_PER_STEP 65536
    mSamples        = (short*)malloc( sizeof(short) * MAX_SAMPLES_PER_STEP );
    mScanLimit      = 0;
    mLastFreqHz     = 0;
    mFreqHz         = 1;

    mTgPoints       = 0;
    mIfGain         = 0;
    mRfCh           = 0;
    mMbwKhz         = 250000;
    mFftn           = 8192;
    mAvne           = 1;
    mAttenDb        = 0;
    mIfGain         = 0;

    mNif            = 1;
    SetDefaultIFs();

    // TODO need provisions for setting these via CLI
    mRemoteLo[0].SetAddr( "192.168.0.2", 6790 );
    mRemoteLo[1].SetAddr( "192.168.0.3", 6790 );
    mRemoteLo[2].SetAddr( "192.168.0.4", 6790 );
    mRemoteTg.SetAddr(    "192.168.0.4", 6790 );

    // mRemoteLo[1].SetAddr( "192.168.0.19", 6790 );
    // mRemoteTg.SetAddr(    "192.168.0.19", 6790 );
}

//------------------------------------------------------------------------------
void SiCtlIf::SetSvcPort( int port )
{
    mIpPort = port;
}

//------------------------------------------------------------------------------
void SiCtlIf::RcvEvent( char *evtStr ) 
{
    printf("SiCtlIf : rcv <%s>\n",evtStr);
}

//------------------------------------------------------------------------------
static const char *helpStr =
"help\n"
"  This command\n"
"halt\n"
"  Stop executing SA\n"
"run\n"
"  Run the SA\n"
"fmin-hz\n"
"  SA min scanning frequency in Hertz\n"
"fmax-hz\n"
"  SA max scanning frequency in Hertz\n"
"lo0 <N>\n"
"  Set LO0 frequency to N Hertz\n"
"lo1 <N>\n"
"  Set LO1 frequency to N Hertz\n"
"si-scan-limit <N>\n"
"  Set the number of scans to conduct to N and then stop\n"
"if-gain <N>\n"
"  Set IF gain N=[0..3]\n"
"si-if-count <N>\n"
"  Set number of IFs used to N\n"
"si-if-hz <N> <F>\n"
"  Set IF <N> to <F> Hz\n"
"si-cal-save <fname>\n"
"  Save calibration data to file fname\n"
"si-cal-adc <val>\n"
"  Set ADC calibration value to <val>\n"
"si-cal-if <N> <val>\n"
"  Set IF gain <N> calibration value to <val>\n"
"si-cal-cnv <N> <val>\n"
"  Set conversion gain for stage <N> calibration value to <val>\n"
"si-cal-var-reset \n"
"  Reset variable calibration (must be done before setting refs)\n"
"si-cal-var <N> <val> \n"
"  Set variable calibration for N*10.0MHz to <val>\n"
"\n"
;

//------------------------------------------------------------------------------
void SiCtlIf::SvcErr( TcpSvrCon *tsc )
{
    char           lineBf[128];
    sprintf( lineBf,"err, arguments, see help\n");
    TcpSvrWrite(tsc,lineBf,strlen(lineBf));
}

//------------------------------------------------------------------------------
// NOTE: most command output nothing as a response since the scan
// data is being sent on the return flow
void SiCtlIf::SvcCmd( TcpSvrCon *tsc, Cli *cli )
{
    char          *cmdStr;
    char           *arg1;
    char           *arg2;
    double         fHz;
    double         dval;
    char           lineBuffer[256];

    unsigned long long llVal;
    double             dVal;

    cmdStr = CliArgByIndex( cli, 0 );
    printf("Cmd= %s\n",cmdStr);        

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"help")  ){
        TcpSvrWrite(tsc,helpStr,strlen(helpStr));
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"run")  ){
        ConfigureHw();
        mDoScan    = 1;
	mScanReset = 1;
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"halt")  ){
        mDoScan    = 0;
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"lo0")  ){
        arg1 = CliArgByIndex( cli, 1 );
        fHz       = atof( arg1 );      
        SetLoWithLockWait(0,fHz);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"lo1")  ){
        arg1 = CliArgByIndex( cli, 1 );
        fHz       = atof( arg1 );      
        SetLoWithLockWait(1,fHz);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"rms")  ){
	double rms,mean;     
        mean = Dp()->Adc()->GetRms( MAX_SAMPLES_PER_STEP, mSamples, &rms);
	sprintf(lineBuffer,"rms dB=%f mean=%f\n",rms,mean);
        TcpSvrWrite(tsc,lineBuffer,strlen(lineBuffer));
        return;
    }	     

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"fmax-hz")  ){
        arg1 = CliArgByIndex( cli, 1 );

        dVal       = atof( arg1 );      
        mFmaxHz    = dVal;
	mScanReset = 1;
        printf("fmax-hz = %f\n",mFmaxHz);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"fmin-hz")  ){
        arg1 = CliArgByIndex( cli, 1 );
        dVal       = atof( arg1 );      
        mFminHz    = dVal;
	mScanReset = 1;
        printf("fmin-hz = %f\n",mFminHz);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"si-scan-limit")  ){
        arg1 = CliArgByIndex( cli, 1 );
        llVal      = atol( arg1 );      
        mScanLimit = llVal;
	mScanReset = 1;
        mScanCount = 0;
        printf("mScanLimit = %f\n",(float)mScanLimit);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"tg-points")  ){
        arg1 = CliArgByIndex( cli, 1 );
        mTgPoints  = atoi( arg1 );
	mScanReset = 1;
        mScanCount = 0;
        printf("mTgPoints = %d\n",mTgPoints);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"if-gain")  ){
        arg1 = CliArgByIndex( cli, 1 );
        mIfGain    = atoi( arg1 );
        Dp()->Adc()->SetGain( mIfGain );
	mScanReset = 1;
        printf("mIfGain = %d\n",mIfGain);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"rf-ch")  ){
        arg1 = CliArgByIndex( cli, 1 );
        mRfCh      = atoi( arg1 );
	mScanReset = 1;
        Dp()->Rbrd()->SetChannel( mRfCh );
        printf("mRfCh = %d\n",mRfCh);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"mbw-khz")  ){
        arg1 = CliArgByIndex( cli, 1 );
        mMbwKhz    = atoi( arg1 );
        if( mMbwKhz > 475 ) mMbwKhz=475; // Avoids DC for all fft sizes
	mScanReset = 1;
        printf("mMbwKhz = %d\n",mMbwKhz);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"fftn")  ){
        arg1 = CliArgByIndex( cli, 1 );
        mFftn      = atoi( arg1 );
	mScanReset = 1;
        printf("mFftn = %d\n",mFftn);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"avne")  ){
        arg1 = CliArgByIndex( cli, 1 );
        mAvne      = atoi( arg1 );
	mScanReset = 1;
        printf("mAvne = %d\n",mAvne);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"integrate")  ){
        arg1 = CliArgByIndex( cli, 1 );
        mInt      = atoi( arg1 );
	mScanReset = 1;
        printf("mInt = %d\n",mInt);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"atten")  ){
        arg1 = CliArgByIndex( cli, 1 );
        mAttenDb   = atof( arg1 );
	mScanReset = 1;
        Dp()->Rbrd()->SetAtten( 2 * mAttenDb  );
        printf("mAttenDb = %f\n",mAttenDb);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"si-if-count")  ){
        arg1 = CliArgByIndex( cli, 1 );
        mInt = atoi( arg1 );
        if( mInt<1 )       mInt=1;
        if( mInt>MAX_IFS ) mInt=MAX_IFS;

        mNif = mInt;
        SetDefaultIFs();
	mScanReset = 1;
        printf("mNif = %d\n",mNif);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"si-if-hz")  ){
        arg1 = CliArgByIndex( cli, 1 );
        mInt = atoi( arg1 );
        if( mInt<0 )        mInt=0;
        if( mInt>=MAX_IFS ) mInt=(MAX_IFS-1);

        arg1 = CliArgByIndex( cli, 2 );
        mIFHz[ mInt ]   = atof( arg1 );
	mScanReset = 1;
        printf("mIfHz[%d] = %f\n",mNif,mIFHz[mInt]);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"si-cal-save")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }

        mCal.CalSave( arg1 );
        printf("Calibration data saved to \"%s\"\n",arg1);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"si-cal-adc")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        dval   = atof( arg1 );

        mCal.CalSetAdc( dval );
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"si-cal-if")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        mInt = atoi( arg1 );

        arg2 = CliArgByIndex( cli, 2 );
        if( !arg2 ) { SvcErr(tsc); return; }
        dval   = atof( arg2 );

        mCal.CalSetIf( mInt, dval );
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"si-cal-cnv")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        mInt = atoi( arg1 );

        arg2 = CliArgByIndex( cli, 2 );
        if( !arg2 ) { SvcErr(tsc); return; }
        dval   = atof( arg2 );

        mCal.CalSetCnv( mInt, dval );
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"si-cal-var-reset")  ){
        mCal.CalSetVarReset();
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"si-cal-var")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        mInt = atoi( arg1 );

        arg2 = CliArgByIndex( cli, 2 );
        if( !arg2 ) { SvcErr(tsc); return; }
        dval   = atof( arg2 );

        mCal.CalSetVarRef( mInt, dval );
        return;
    }
}

//------------------------------------------------------------------------------
void SiCtlIf::SetDefaultIFs()
{
    if( 1==mNif ){
       mIFHz[0]        =   10750000;
    }
    else if ( 2==mNif ){
       mIFHz[0]        = 1590000000;
       mIFHz[1]        =   10750000;
    }
    else{
       mIFHz[0]        = 1575000000;
       mIFHz[1]        =  315000000;
       mIFHz[2]        =   10750000;
    }
}

//------------------------------------------------------------------------------
void SiCtlIf::ConfigureStaticLOs()
{
    double loHz, loHzAct;
    int    idx;

    printf("ConfigureStatcLOs Start\n");
    for(idx=1;idx<mNif;idx++){
       loHz    = mIFHz[idx - 1] + mIFHz[idx];
       loHzAct = SetRemoteLO(idx,loHz);
       if( loHzAct > 0 ){
          mLOHzAct[idx] = loHzAct;
       }
       else{
          mLOHzAct[idx] = loHz;
       }
    }
    printf("ConfigureStatcLOs End\n");
}

//------------------------------------------------------------------------------
void SiCtlIf::DoScanReset( TcpSvrCon *tsc )
{
    int          nBytes;
    double       df;
    int          mb,sb;
    const char  *refStr;
    double       feGain;
    char         lineBf[128];

    // Clear scan reset
    mScanReset = 0;

    // Configure static LOs
    ConfigureStaticLOs();

    // Reset the lock counts
    mLockWait     =0;
    mLockWaitAbort=0;

    // Reset scan based counts
    mScanCount = 0;
    gettimeofday( &mScanCountTvBase, NULL );

    // Reset the frequency to scan min
    mFreqHz    = mFminHz;

    // Calculate the working span
    df = mFmaxHz - mFminHz;

    // Calculate the frequency step
    if( 0==mTgPoints ){
        mFstepHz   = mMbwKhz * 1000;
    }
    else{
        mFstepHz   = df/mTgPoints;
    }

    // Update the fft size used and sample count
    mFftSize   = mFftn;
    mSampleCnt = mAvne*mFftSize;
    if( mSampleCnt > MAX_SAMPLES_PER_STEP ){
        mSampleCnt = MAX_SAMPLES_PER_STEP;
    }

    // Measurement bins are a fraction of fft/2 bins avail relative
    // to specified measurement bw by sampling bw
    mb = (mFftSize/2) * mMbwKhz / 500;

    // Integrating/summing measurement bins reduces the number of output points
    // but changes the peak and floor depending on how total power
    // vs average power is configured.
    if( 0!=mTgPoints || mInt ){
        sb = mb;
    }
    else{
        sb = 1;
    }

    // Reconfigure the estimator
    mSiEst.Configure( mFftSize, 
                      1,  // Window enable
                      1,  // Fast implementation 
                      mb, // Measurement bins
                      sb  // Summed measurement bins
                    ); 

    // Reconfigure the calibration factors 
    switch( mRfCh ){
        case 0:{
           feGain = 0.0;
           break;
        }
        case 1:{
           feGain = -mAttenDb;
           break;
        }
        case 2:{
           feGain = 0.0;
           break;
        }
        case 3:{
           feGain = +20.0;
           break;
        }
    }
    refStr = mCal.CalConfigure( mNif, mIfGain, feGain );

    // Indicate reset of scan parameters
    nBytes = sprintf( lineBf, "reset 1\n");
    TcpSvrWrite(tsc,lineBf,nBytes);

    // Indicate new scan starting
    nBytes = sprintf( lineBf, "scan 1\n");
    TcpSvrWrite(tsc,lineBf,nBytes);

    // Indicate new reference string
    nBytes = sprintf( lineBf, "ref %s\n",refStr);
    TcpSvrWrite(tsc,lineBf,nBytes);

}

//------------------------------------------------------------------------------
int SiCtlIf::SvcScan( TcpSvrCon *tsc )
{
    int    idx,nPts;
    int    nbReq,nbAct;
    int    endOfScan;
    double f,df,c;
    double freqHzOfCurSamples;
    char   lineBf[128];
    double m2Data[65536];

    // If we aren't actually scanning just wait a little to avoid
    // spinning on the control socket
    if( !mDoScan ){
        us_sleep( 100000 );
        return(0);
    }

    // Lots of book keeping on scan reset
    if( mScanReset  ){
        DoScanReset(tsc);
    }

    // Start the timing on a single scan loop
    mSvcScanSw.Start();

    // Assume this is not the end of the scan until
    // decision is made later 
    endOfScan = 0;


    // Collect the samples
    mAdcSw.Start();
    for( idx=0; idx<mSampleCnt; idx+=2 ){
        Dp()->Adc()->GetSamplePair( mSamples+idx );
    }
    mAdcSw.StopWithHistory();


    // Save the freqency center these samples 
    // were collected at.
    freqHzOfCurSamples = mFreqHz;

    // Step the frequency
    // From this point forward mFreqHz is the next frequency
    // centery we are working on (not the current one);
    mFreqHz += mFstepHz;

    // New Scan 
    if( mFreqHz >= mFmaxHz ){

       // Start new scan at min frequency
       mFreqHz   = mFminHz;

       // Update the scan count
       mScanCount++;

       // Next freq being set is start of new scan.  Current 
       // sampled points are end of the scan.  Need to process
       // current points then emit scan marker.
       endOfScan = 1;

    }

    // Check for changed min and reset
    if( mFreqHz < mFminHz ){
       mFreqHz = mFminHz;
    }   

    // Set the actual frequency for next step
    // NOTE: if frequency hasn't really changed then don't reconfigure the hw
    mFreqSw.Start();
    if( mFreqHz != mLastFreqHz ){
        double lo0;
        double lo0Act;
        double fAct;

        lo0 = mFreqHz + mIFHz[0];

        /*
        printf("mFreq=%.0f, lo0=%.0f, f1=%.0f\n", 
                     mFreqHz, 
                     lo0, 
                     mIFHz[0]);
        */

        // Set first LO  (account for failure to lock/set)
        lo0Act = SetLoWithLockWait( 0, lo0);
        if( lo0Act > 0 ){
           mLOHzAct[0] = lo0Act;
        }
        else{
           mLOHzAct[0] = lo0;
        }

        // Set tracking generator if enabled 
        if( 0!=mTgPoints ) SetRemoteTG( mFreqHz );

        /* 
        Since the LO's may truncate to a frequency based on the PLL
        configuration, we need to compensate for the final center frequency
        at the ADC.

        For each stage we have:

                     LO(N)
                      |
                      V
               f(N) ->X-> IF(N)

          (1) IF(n) = LO(n) +/- f(n) 
        When using high side inject we use: 
          (2) LO(n) = f(n) + IF(n)
        or
              LO(n) - IF(n) = f(n)
          (3) f(n) = LO(n) - IF(n)
        The last IF or IF(N-1) is fixed by the ADC filter 
        and center frequency at 10.75M.  When cascading conversion
        we have f(N+1) = IF(N) and using (3) we then have

        N=1:
           f(0) = LO(0) - IF(0)=10.75M
        N=2:
           f(1) = LO(1) - IF(1)=10.75M
           f(0) = LO(0) - f(1)
        N=3:
           f(2) = LO(2) - IF(2)=10.75M
           f(1) = LO(1) - f(2)
           f(0) = LO(0) - f(1)
        */ 
        switch( mNif ){
            case 1:
                fAct = mLOHzAct[0] - mIFHz[0];
                break;
            case 2:
                fAct = mLOHzAct[0] - (mLOHzAct[1] - mIFHz[1]);
                break;
            case 3:
                fAct = mLOHzAct[0] - (mLOHzAct[1] - (mLOHzAct[2] - mIFHz[2]));
            default:
                break;
        }
        
#       if 0
        printf("f=%10.f\n", fAct);
        for(idx=0;idx<mNif;idx++){
            printf("  LO[%d]=%10.f IF[%d]~=%10.f\n",
                                   idx,mLOHzAct[idx],
                                   idx,mIFHz[idx]);
        }
#       endif

	// Update the next frequency under evaluation to reflect actual settings
        mFreqHz = fAct; 

        // Under simulation and test we change the test level
#       ifdef TGT_X86
        {
            extern void SignalSimSetLevel( double );
            // if( (mFreqHz > 495.0e6) && (mFreqHz<505.0e6) ){
            if( mFreqHz>=(72.0e6 - 63e3) && mFreqHz<=(72.0e6+63e3) ){
                SignalSimSetLevel( 1.0 ); 
            }
            else{
                SignalSimSetLevel( 1.0e-4 ); 
            }
        }
#       endif
    }
    mFreqSw.StopWithHistory();
    mLastFreqHz = mFreqHz;

    // Now that we are locked, go ahead and flush the adc queue
    // This lets samples accumulate for next interval while
    // we are processing this interval
    Dp()->Adc()->FlushSamples();

    // Get the spectral estimate for this step
    mSiSw.Start();
    nPts = mSiEst.Estimate( mSamples, mSampleCnt, m2Data );
    mSiSw.StopWithHistory();

    // Send the results
    mNetSw.Start();
    c = mCal.CalGet( freqHzOfCurSamples );
    f = freqHzOfCurSamples - (mFstepHz/2);
    df= (double)mFstepHz/(double)nPts; 
    f = f + df/2;
    for( idx=0;idx<nPts;idx++){
        // if( idx==0 ) m2Data[idx] = -160; // Marker....
        nbReq = sprintf( lineBf, "%f %f\n", f, m2Data[idx] + c );
        nbAct = TcpSvrWrite( tsc,lineBf,nbReq );
        if( nbAct < 0 ){
           printf("Socket error while sending scan\n");
           break;  // check for socket err/close
        }
        f = f + df;
    }

    // If we just sent the last of the points within a scan
    // then send the scan marker.
    if( endOfScan ){
       endOfScan = 0;

       // Send the scan marker
       nbReq = sprintf( lineBf, "scan\n");
       TcpSvrWrite(tsc,lineBf,nbReq);

       // If limited scanning and this is the end, stop scanning
       if( (mScanLimit!=0) && (mScanCount>mScanLimit) ){
           mDoScan = 0;
       }	        
    }

    mNetSw.StopWithHistory();

    mSvcScanSw.StopWithHistory();
    return(0);
}

//------------------------------------------------------------------------------
int SiCtlIf::SvcCtl( TcpSvrCon *tsc, Cli *cli )
{
    int              err,idx,readBytes;
    char             bf[128];
    
    // Return immediately if nothing ready to read
    if( !TcpSvrReadReady(tsc) ) return(0);
    
    // Read the data that is ready (possibly error/closing)
    printf("Read command data\n");
    readBytes = TcpSvrRead( tsc, bf, sizeof(bf)-1 );
    if( readBytes<=0 ){
       return( -1 );
    }

    // Loop through all of the input bytes inserting them in the cli
    // If a command line is ready, service it, reset parsing, and continue
    // processing input bytes 
    if( readBytes>0 ) bf[readBytes]=0;
    for( idx=0; idx<readBytes; idx++ ){
        err = CliInputCh( cli, bf[idx] );
        if( err > 0 ){
            printf("Servicing command\n");
            SvcCmd( tsc, cli );
            CliReset( cli );
        }
    }		 

    return( 0 );
}

//------------------------------------------------------------------------------
void SiCtlIf::Main()
{
    int              err;
    char             peerStr[128];
    Cli              *cli;
    TcpSvrCon        *tsc;
    struct timeval    tv1,tv2;
    unsigned int      dus,sus;

    printf("SiCtlIf:Entering main control loop\n");
    while( !mThreadExit ){

        printf("SiCtlIf:Allocate new control connect\n");
        tsc = TcpSvrConNew();
        if( !tsc ) return;

        printf("SiCtlIf:Waiting for cli control connection p=%d\n",mIpPort);
        err = TcpSvrConWaitNewCon(tsc,mIpPort);
        if( err<0 ) continue;

        TcpSvrGetClnIpStr( tsc, peerStr, sizeof(peerStr) );
        printf("SiCtlIf:Connection from %s\n",peerStr);

        printf("SiCtlIf:Servicing connection\n");
        cli = CliNew();
        if( !cli ) break;

        err = 0;
        gettimeofday( &tv1, NULL );
        while( !err  ){

            SvcScan( tsc );
            err = SvcCtl( tsc,cli );

            gettimeofday( &tv2, NULL );
            dus = tv_delta_useconds( &tv2, &tv1 );
            if( dus > 2000000 ){
               printf("Fmin(MHz)=%f, Fmax(MHz)=%f, Scan=%d nIF=%d\n",
                     mFminHz/1e6,
                     mFmaxHz/1e6,
                     mScanCount,
                     mNif
                     );
               printf("    mFftSize=%d, mSampleCnt=%d,\n",
                     mFftSize,
                     mSampleCnt
                     );
               sus = tv_delta_useconds( &tv2, &mScanCountTvBase );
               printf("    Scan/Sec=%f, df(MHz)=%f Fsteps=%f LockWait=%d/%d\n",
                     ((double)mScanCount/((double)sus/1e6)),
                     (mFmaxHz - mFminHz)/1e6,
                     (mFmaxHz - mFminHz)/mFstepHz,
		     mLockWait, mLockWaitAbort
                     );
               printf("    mFstepHz=%d, tgp=%d, Nif=%d, RFch=%d, attn=%f\n",
                     mFstepHz,
                     mTgPoints,
                     mNif,
                     mRfCh,
                     mAttenDb 
                     );
               printf("    uS :SvcScan=%lu,Adc=%lu,Si=%lu,Ne=%lu,Fr=%lu\n",
                     mSvcScanSw.MedianuS(),
                     mAdcSw.MedianuS(),
                     mSiSw.MedianuS(),
                     mNetSw.MedianuS(),
                     mFreqSw.MedianuS()
                     );
               printf("    IF0=%10.f  IF1=%10.f  IF2=%10.f\n",
                     mIFHz[0], 
                     mIFHz[1], 
                     mIFHz[2]
                     );
               printf("    LO0=%10.f  LO1=%10.f  LO2=%10.f\n",
                     mLOHzAct[0], 
                     mLOHzAct[1], 
                     mLOHzAct[2]
                     );
               printf("    rTG : addr=%-16s err=%d\n",
                     mRemoteTg.GetAddr(),
                     mRemoteTg.Error()
                     );
               printf("    rLO0: addr=%-16s err=%d\n",
                     mRemoteLo[0].GetAddr(),
                     mRemoteLo[0].Error()
                     );
               printf("    rLO1: addr=%-16s err=%d\n",
                     mRemoteLo[1].GetAddr(),
                     mRemoteLo[1].Error()
                     );
               printf("    rLO2: addr=%-16s err=%d\n",
                     mRemoteLo[2].GetAddr(),
                     mRemoteLo[2].Error()
                     );
               gettimeofday( &tv1, NULL );
            }

        } // End of primary loop

        TcpSvrClose( tsc );
        free( tsc );
        free( cli );
    }
}
