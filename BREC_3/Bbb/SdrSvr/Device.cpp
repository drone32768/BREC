//
//
// This source code is available under the "Simplified BSD license".
//
// Copyright (c) 2016, J. Kleiner
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, 
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright 
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the original author nor the names of its contributors 
//    may be used to endorse or promote products derived from this software 
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <ifaddrs.h>
#include <sys/time.h>
#include <math.h>

#include "Device.h"

#include "Ddc100/Ddc100.h"
#include "Mboard/Mboard.h"
#include "Tboard/Tboard.h"


////////////////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////////////////

static    double if1Hz    = 1500.0e6;
static    double if2Hz    = 2.0e6;
static    double offsetHz = 70e3;

static Bdc     *gBdc;
static Ddc100  *gDdc;
static Adf4351 *gSyn;
static Mboard  *gMbrd;
static Tboard  *gTbrd;

Device::Device()
{
    int force;

    mThreadExit       = 0;
    mDisplayCount     = 0;

    mSwapIQ           = 0;
    mPmTuneMHz        = 92100000;
    mPmGainDb         = 0;

# ifdef TGT_X86
    force = 1;  // x86
# else
    force = 0;  // arm
# endif

    if( force || FindCapeByName("brecFpru")>0  ){
        double flt;

        printf("******** Devs::Open Starting F/Bdc/Ddc100 ****************\n");

        gBdc = new Bdc();
        gBdc->Open();

        gTbrd = new Tboard();
        gTbrd->Attach( (void*)( gBdc->GetPinGroup(0) ), (void*)0 );

        gMbrd = new Mboard();
        gMbrd->Attach( (void*)( gBdc->GetPinGroup(1) ), (void*)0 );

        gSyn = gMbrd->GetAdf4351( 0 );
        gSyn->SetAuxEnable(0);
        gSyn->SetMainPower(0);

        gDdc = new Ddc100();
        gDdc->Attach( gBdc );
        gDdc->Open();

        gDdc->SetChannelMatch( 3, 1.0, -14, 1.0 );
        gDdc->SetTpg( 0 );   
        // gDdc->SetTpg( 2 );   

        gDdc->SetSource( 5 );          // Set to 200kHz channel

        flt = gTbrd->SetFreqHz( if1Hz );
        printf("#####Tboard IF = %f Hz\n",flt);

        flt = gTbrd->SetBwHz( 2*if2Hz );
        printf("#####Tboard BW = %f Hz\n",flt);

        printf("#####Tboard bb gain set low\n");
        gTbrd->SetBbGainDb( 0 );

        flt =gDdc->SetLoFreqHz( if2Hz );
        printf("#####DDC100 IF = %f Hz\n",flt);

        printf("#####Tboard rf gain set med\n");
        gTbrd->SetRfGainDb( 40 );

        gDdc->StartPru();

        TunerSet( 92100000 );
    }

    else{
        // Just terminate here since exceptions will result
        fprintf(stderr,"No recognized device tree\n");
        exit(-1);
    }
}

//------------------------------------------------------------------------------
int Device::TunerSet( long long freqHz )
{
    double flt;
    printf("Device::TunerSet( %f Hz )\n",(double)freqHz);

    double lo1Hz = freqHz + if1Hz - if2Hz - offsetHz;

    mDevLock.Lock();
    flt = gSyn->SetFrequency( lo1Hz );
    printf("#####Mboard IF = %f Hz\n",flt);
    // gDdc->FlushSamples();
    mDevLock.Unlock();

    mPmTuneMHz = (double)freqHz / 1e6;
    printf("*** TunerSet *** at %f Hz\n",(double)freqHz);
    return(0);
}

//------------------------------------------------------------------------------
int Device::GainSet( double gainDb )
{
    mDevLock.Lock();
    gTbrd->SetRfGainDb( gainDb );
    mDevLock.Unlock();

    printf("*** GainSet *** at %f dB\n",gainDb);
    mPmGainDb = gainDb;
    return(0);
}

//------------------------------------------------------------------------------
void Device::Monitor()
{
    mDevLock.Lock();
    if( !gSyn->GetLock() ) mPmLock = 0;
    mDevLock.Unlock();
}

//------------------------------------------------------------------------------
void Device::RcvEvent( char *evtStr )
{
    char       *cmdStr;
    char       *argStr;
    char       *tokr;

    printf("Device   : rcv <%s>\n",evtStr);

    cmdStr = strtok_r( evtStr, " ", &tokr );
   
    if( 0==strcmp("device.sample-rate",cmdStr) ){
       int sampleRate;	      

       argStr     = strtok_r(NULL, " ", &tokr);
       sampleRate =  atoi( argStr );
       printf("sample rate = %d\n",sampleRate);
    }

    if( 0==strcmp("device.display",cmdStr) ){
       argStr = strtok_r(NULL, " ", &tokr);
       mDisplayCount =  atoi( argStr );
    }

    if( 0==strcmp("device.tune-hz",cmdStr) ){
       long long freq; 	     
      
       argStr = strtok_r(NULL, " ", &tokr);
       freq   = strtoll( argStr, NULL, 10 );

       TunerSet( freq );
    }    

    if( 0==strcmp("device.gain-db",cmdStr) ){
       double gainDb;
      
       argStr = strtok_r(NULL, " ", &tokr);
       gainDb = strtof( argStr, NULL );

       GainSet( gainDb );
    }    

    if( 0==strcmp("device.if-gain-db",cmdStr) ){
       double gainDb;
      
       argStr = strtok_r(NULL, " ", &tokr);
       gainDb = strtof( argStr, NULL );

       gTbrd->SetBbGainDb( gainDb );
    }    
}

//------------------------------------------------------------------------------
void Device::Main()
{
    while( !mThreadExit ){
        Monitor();
	us_sleep( 100000 );
    }
}

//------------------------------------------------------------------------------
//
// This method is expected to produce the specified number of complex 16 bit 
// samples at the location provided.  
// 
// It is periodically called and may pend waiting for samples.
//
// This is the standard device interface.  This simulation/test version
// rate limits to the specified sample rate and relies on GenSamples() to 
// actually produce the samples.
//
void Device::GetSamples( short *sampPtr, int nComplexSamples )
{
    static unsigned int runningSampleCount;
    int                 complexSampleCount;

    // TODO - move these into class
    static short        *eo;
    static short        bf[2048];
    static int          bfSamps=0;

    // Process samples
    complexSampleCount = 0;
    while( complexSampleCount < nComplexSamples ){

        // Get 2k samples to internal buffer
        if( bfSamps<=0 ){

            mDevLock.Lock();
            mPmPause += gDdc->Get2kSamples( bf );
            mDevLock.Unlock();

            eo      = bf;
            bfSamps = 2048;
        }
        sampPtr[0] = eo[0];
        sampPtr[1] = eo[1];

        // update sample count and buffer location
        runningSampleCount++;
        sampPtr+=2;
        complexSampleCount++;

        // Update internal buffer
        eo     +=2;
        bfSamps-=2;
    }

}

