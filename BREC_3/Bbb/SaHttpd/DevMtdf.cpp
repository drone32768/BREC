//
// This source code is available under the "Simplified BSD license".
//
// Copyright (c) 2015, J. Kleiner
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
//
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <iostream>

#include "DevMtdf.h"

//------------------------------------------------------------------------------
DevMtdf::DevMtdf()
{
    // No constructor actions required
 
    mIf1Hz = 1500.0e6;
    mIf2Hz = 2.0e6;
    mOffHz = 70e3;
}

//------------------------------------------------------------------------------
int DevMtdf::Open()
{
    printf("******** DevMtdf::Open Starting F/Bdc/Ddc100 ****************\n");

    // Create a bdc device and open it
    mBdc = new Bdc();
    mBdc->Open();

    // Create a ddc device and attach to bdc
    mDdc = new Ddc100();
    mDdc->Attach( mBdc );

    // Create a Tboard and attach to port 0
    mTbrd = new Tboard();
    mTbrd->Attach( (void*)( mBdc->GetPinGroup(0) ), (void*)0 );

    // Create an Mboard and attach to port 1
    mMbrd = new Mboard();
    mMbrd->Attach( (void*)( mBdc->GetPinGroup(1) ), (void*)0 );

    mMbrd->GetAdf4351( 0 )->SetAuxEnable(0);
    mMbrd->GetAdf4351( 0 )->SetMainPower(0);

    // Open ddc
    mDdc->Open();

    // Configure the ddc
    mDdc->SetChannelMatch( 3, 1.0, -14, 1.0 ); // TODO - change how done
    mDdc->SetTpg( 0 );

    // Comment this line out to use CPU based SPI to samples
    mDdc->StartPru();

    printf("*********** DevMtdf::Open End *****************************\n");

    return(0);
}

//------------------------------------------------------------------------------
int DevMtdf::GetComplexSampleRate()
{
    return( mDdc->GetComplexSampleRate() );
}

//------------------------------------------------------------------------------
int DevMtdf::FlushSamples()
{
    return( mDdc->FlushSamples() );
}

//------------------------------------------------------------------------------
int DevMtdf::Get2kSamples( short *dst )
{
    return( mDdc->Get2kSamples(dst) );
}

//------------------------------------------------------------------------------
int DevMtdf::SetChannel( int chId )
{
    double flt;

    mIf1Hz = 1500.0e6;
    mOffHz = 70e3;

    switch( chId ){
        case 3:
           mIf2Hz  = 100e3;
           break;
        case 4:
           mIf2Hz  = 2e6;
           break;
        case 5:
           mIf2Hz  = 10e6;
           break;
        default:
           break;
    }

    mDdc->SetSource(chId);

    flt = mTbrd->SetFreqHz( mIf1Hz );
    printf("#####Tboard IF = %f Hz\n",flt);

    flt = mTbrd->SetBwHz( 2*mIf2Hz );
    printf("#####Tboard BW = %f Hz\n",flt);

    printf("#####Tboard bb gain set low\n");
    mTbrd->SetBbGainDb( 0 );

    flt =mDdc->SetLoFreqHz( mIf2Hz );
    printf("#####DDC100 IF = %f Hz\n",flt);

    return(0);
}

//------------------------------------------------------------------------------
double DevMtdf::SetRefDbm( double refDbm )
{
    // TODO - this is incorrect but sufficient for short term testing

    if( refDbm > 0 )   { refDbm = 0;   }
    if( refDbm < -70 ) { refDbm = -70; }

    printf("#####Tboard rf gain set %f\n", -refDbm);
    mTbrd->SetRfGainDb( -refDbm );

    return( refDbm );
}

//------------------------------------------------------------------------------
double DevMtdf::SetTuneHz( double freqHz )
{
    double flt;
    double lo1Hz = freqHz + mIf1Hz - mIf2Hz - mOffHz;

    flt =  mMbrd->GetAdf4351( 0 )->SetFrequency( lo1Hz );
    printf("#####Mboard IF = %f Hz / %f Hz\n",flt,lo1Hz);

    // TODO - incorporate first lo freq correction at ddc to get precise tune

    printf("*** TunerSet *** at %f Hz\n",(double)freqHz);
    return(freqHz);
}

