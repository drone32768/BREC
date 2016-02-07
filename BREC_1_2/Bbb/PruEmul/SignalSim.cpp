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
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <math.h>

#include "../Util/mcf.h"
#include "../Aboard/pructl.h"

volatile unsigned char *gDdrMem;
volatile unsigned char *gSramMem[2];

/* NOTE: we used fixed signed 16 bit value for most calculations
 * and then convert down to 12 bits before inserting in emulated pru 
 * dram fifo.
 */

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define TBL_ENTRIES (32*1024)
//#define TBL_ENTRIES (1024*1024)

static short gnSinTbl[ TBL_ENTRIES ];

static void TrigInit()
{
    int    n;
    double x;

    for(n=0; n<TBL_ENTRIES; n++){
        x = 2.0 * 3.141592652 * n / TBL_ENTRIES;
        gnSinTbl[n] =  (32767*sin( x ));
    }
}

//------------------------------------------------------------------------------
static short TrigSin( unsigned int ph )
{
    if( ph > TBL_ENTRIES ) ph = ph%TBL_ENTRIES;
    return( gnSinTbl[ph] );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/** 
 * This class implements an object which generates test data and makes it
 * available via an emulated pruss / pru interface.  It is an mcf thread
 * which acts on the following attributes:
 *    sample-rate
 *      sets sampling rate to specified value
 *    test-freq
 *      sets carrier frequency to specified value
 *    test-bw
 *      sets bandwidth of fm modulated signal 
 *    test-tone
 *      sets frequency of baseband single tone applied in fm signal
 *    test-noise
 *      sets noise level of signal
 *
 * To initiate a free running instance use the C method:
 *         SignalSimStart()
 *    
 *
 */
class SignalSim : public McF {
  private:
    int           mThreadExit;

    int           mTestComplexSamplesPerSecond;
    int           mTestFreqHz;
    int           mTestToneHz;
    int           mTestBwHz;
    int           mTestNoiseAmpl;
    int           mDisplayCount;
    double        mTestLevel;  // [ 0.0, 1.0 ]

    short GenTestSample();

  public:
    SignalSim();
    void Main();
    void RcvEvent( char *evtStr );

    void  SetLevel( double v );
};

//----------------------------------------------------------------------------
SignalSim::SignalSim()
{
    TrigInit();

    mThreadExit = 0;
    mTestComplexSamplesPerSecond = 500000; 
    mTestFreqHz                  = 240000; // 240000; // 50000;
    mTestBwHz                    = 0;      // 20000;  // 5000; 
    mTestNoiseAmpl               = 0; // 2;
    mTestToneHz                  = 3000;
    mDisplayCount                = 0;
    mTestLevel                   = 1.0;
}

//----------------------------------------------------------------------------
void SignalSim::RcvEvent( char *evtStr )
{
    char *cmdStr;
    char *argStr;
    char *tokr;

    printf("SignalSim: rcv <%s>\n",evtStr);

    cmdStr = strtok_r( evtStr, " ", &tokr );
   
    if( 0==strcmp("sample-rate",cmdStr) ){
       argStr = strtok_r(NULL, " ", &tokr);
       mTestComplexSamplesPerSecond =  atoi( argStr );
    }

    if( 0==strcmp("d1",cmdStr) ){
       argStr = strtok_r(NULL, " ", &tokr);
       mDisplayCount =  atoi( argStr );
    }

    if( 0==strcmp("test-freq",cmdStr) ){
       argStr = strtok_r(NULL, " ", &tokr);
       mTestFreqHz =  atoi( argStr );
    }

    if( 0==strcmp("test-bw",cmdStr) ){
       argStr = strtok_r(NULL, " ", &tokr);
       mTestBwHz =  atoi( argStr );
    }

    if( 0==strcmp("test-tone",cmdStr) ){
       argStr = strtok_r(NULL, " ", &tokr);
       mTestToneHz =  atoi( argStr );
    }

    if( 0==strcmp("test-noise",cmdStr) ){
       argStr = strtok_r(NULL, " ", &tokr);
       mTestNoiseAmpl =  atoi( argStr );
    }

    if( 0==strcmp("test-level",cmdStr) ){
       argStr = strtok_r(NULL, " ", &tokr);
       mTestLevel =  atof( argStr );
    }

}

//----------------------------------------------------------------------------
void SignalSim::Main()
{
    static struct timeval   tv1,tv2;
    int                     sCnt;
    int                     idx, n, dus;
    double                  rate;
    int                     x;    

    volatile unsigned long  *ptrHead;
    volatile short *ptrPruSamples;

    printf("Entering SignalSim::Main()\n");

    ptrHead    = (volatile unsigned long*)(gSramMem[0]+PRU0_OFFSET_DRAM_HEAD);
    ptrPruSamples = (volatile short*)gDdrMem;

    idx = 0;
    sCnt= 0;
    while( !mThreadExit ){

        //  Capture start time of interval
        if( sCnt==0 ){
            gettimeofday( &tv1, NULL );
        }

        // Generate a set of samples
        for(n=0;n<1024;n++){
           // comment out to test higher sample rates
	   // with data generation, bbb tops out at 250ksps,
	   // witout it can meet 625ksps
           x = GenTestSample();
	   
#if 0
           // convert the fixed signed 16 bit signal into a 12 bit
           // unsigned signal
           x = (x + 32768)>>4;

           // Center the 12 bits within the 16 bits (like AD7276)
           x = x<<2;
#else
           x = (x + 32768);
           x = x>>4;
           x = x & (0xfffe | (random()&1));  // put on a single bit of noise
           x = x<<4;
#endif

           ptrPruSamples[ idx ]  = x;
           ptrHead[0]            = (idx * 2);

if( mDisplayCount > 0 ){
printf("%d, ",ptrPruSamples[idx]);
mDisplayCount--;
}
           idx = (idx+1)%PRU_MAX_SHORT_SAMPLES;
           sCnt++;
        }

        // Wait, if needed, until time has elpased to match rate
        while( 1 ){
            gettimeofday( &tv2, NULL );
            dus = tv_delta_useconds( &tv2, &tv1 ); 
            rate= 1000000.0*(double)sCnt / (double)dus;
            if( rate <= (2*mTestComplexSamplesPerSecond) ) break;
        }

        // Reset the rate measurement interval
        if( dus > 500000 ){
            sCnt = 0;
            // printf(" SignalSim r = %f\n",rate);
        }
    }
}

//----------------------------------------------------------------------------
short SignalSim::GenTestSample()
{
    static unsigned long long phi;
    static unsigned long long psi;
    int        fs,fc;
    short      v;
    int        tf, sps;

    sps = 2*mTestComplexSamplesPerSecond;
    tf  = mTestFreqHz;

    psi = psi + TBL_ENTRIES *( (double)mTestToneHz / (double)sps );
    psi = psi%TBL_ENTRIES;
    fs  = TrigSin( psi );
    fc  = tf + ( mTestBwHz * fs / 32767) ; 


    phi = phi + TBL_ENTRIES * ( (double)fc / (double)sps );
    phi = phi%TBL_ENTRIES;
    v   = (short)( mTestLevel*TrigSin( phi ) );
    // printf("fc=%d phi=%lld v=%d\n",fc,phi,(int)v);
    v   += mTestNoiseAmpl*((double)random())/RAND_MAX;
    return( v );
}

//----------------------------------------------------------------------------
static SignalSim   *gpSignalSim = NULL;

void SignalSim::SetLevel( double v )
{
   mTestLevel = v;
}

void SignalSimSetLevel( double v ){
   if( gpSignalSim ) gpSignalSim->SetLevel( v );
}

//----------------------------------------------------------------------------
extern "C" {

    void SignalSimStart()
    {
        gpSignalSim = new SignalSim();
        GetEvrSingleton()->Register( gpSignalSim );
        gpSignalSim->Start();
    }

}
