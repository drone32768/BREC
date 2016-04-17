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

#include "Devs.h"


////////////////////////////////////////////////////////////////////////////////
/// Devices ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
static Devs *gpDevs = NULL;

//------------------------------------------------------------------------------
Devs *Dp()
{
    if( !gpDevs ){
       gpDevs = new Devs();
    }
    return( gpDevs );
}

//------------------------------------------------------------------------------
Devs::Devs()
{
}

//------------------------------------------------------------------------------
int Devs::Open()
{
    int force;

    // This just opens the dev(s).  See HwModel:HwInit() for initial
    // parameters

# ifdef TGT_X86
    force = 1;  // x86
# else
    force = 0;  // arm 
# endif

    if( force || FindCapeByName("brecFpru")>0  ){
        printf("******** Devs::Open Starting F/Bdc/Ddc100 ****************\n");

        mBdc = new Bdc();
        mBdc->Open();

        mTbrd = new Tboard();
        mTbrd->Attach( (void*)mBdc, (void*)0 /* port */ );

        mMbrd = new Mboard();
        mMbrd->Attach( (void*)mBdc, (void*)1 /* port */ );

        mDdc = new Ddc100();
        mDdc->Attach( mBdc );

        mAdc = mDdc;
        mAdc->Open();
        mMix = (Ddc100*)mAdc;
    }

    else{
        // Just terminate here since exceptions will result
        fprintf(stderr,"No recognized device tree\n");
        exit(-1);
    }

    printf("*********** Devs::Open End *****************************\n");

    // Register this devices token/cli handler
    TokRegister( this );

    return(0);
}

////////////////////////////////////////////////////////////////////////////////
/// Command line parser ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int
Devs::TokParse( 
  std::vector<std::string> & arInputTokens , 
  std::ostringstream       & arOutStrSt 
)
{
   double freqHz;
   double bwHz;
   double gainDb;

   int consumed = 1;

   if( 0==arInputTokens[0].compare("help") ){
       arOutStrSt << "mboard freq <Hz>       Set Mboard tuning\n";
       arOutStrSt << "mboard pwr  [0..3]     Set Mboard LO power level\n";
       arOutStrSt << "tboard freq <Hz>       Set Tboard tuning\n";
       arOutStrSt << "tboard gain <dB>       Set Tboard RF gain\n";
       arOutStrSt << "tboard bw   <Hz>       Set Tboard IF filter bandwidth\n";
       consumed = 0; // special, allow other callbacks to add help
   }

   else if( 0==arInputTokens[0].compare("mboard" ) ){
       Adf4351 *syn = mMbrd->GetAdf4351( 0 );
       arOutStrSt << "selecting mboard\n";

       if( arInputTokens.size() < 3 ){
          arOutStrSt << "missing argument\n";
       }

       else if( 0==arInputTokens[1].compare("freq") ){
          freqHz = atof( arInputTokens[2].c_str() );  // stod C++11
          syn->SetFrequency( freqHz );
          syn->SetAuxEnable( 0 );
          arOutStrSt << "freq        = " << freqHz         << "\n";
          arOutStrSt << "lock status = " << syn->GetLock() << "\n";
          arOutStrSt << "ok\n";
       }
       else if( 0==arInputTokens[1].compare("pwr") ){
          syn->SetMainPower( 0 );
          arOutStrSt << "ok\n";
       }
   }

   else if( 0==arInputTokens[0].compare("tboard" ) ){
       arOutStrSt << "selecting tboard\n";

       if( arInputTokens.size() < 3 ){
          arOutStrSt << "missing argument\n";
       }
       else if( 0==arInputTokens[1].compare("freq") ){
          freqHz = atof( arInputTokens[2].c_str() );  // stod C++11
          mTbrd->SetFreqHz( freqHz );
          arOutStrSt << "ok\n";
       }
       else if( 0==arInputTokens[1].compare("gain") ){
          gainDb = atof( arInputTokens[2].c_str() );  // stod C++11
          mTbrd->SetGainDb( gainDb );
          arOutStrSt << "ok\n";
       }
       else if( 0==arInputTokens[1].compare("bw") ){
          bwHz = atof( arInputTokens[2].c_str() );  // stod C++11
          mTbrd->SetBwHz( bwHz );
          arOutStrSt << "ok\n";
       }
   }

   else{
       consumed = 0;
   }

   return( consumed );
}
