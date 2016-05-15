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
//
#ifndef __DEVMTDF__
#define __DEVMTDF__

#include "Util/mcf.h"
#include "Util/net.h"
#include "Util/gpioutil.h"

#include "Device.h"

#include "Interfaces/AdcIf.h"

#include "Ddc100/Ddc100.h"

#include "Mboard/Mboard.h"
#include "Tboard/Tboard.h"

/**
 * This is the mtdf (M board, T board, D board F board) device for use
 * by SaHttpd
 */
class DevMtdf : public Device {
  private:

    // For locking device access
    SimpleMutex mMutex;

    // For hw model
    AdcIf   *mAdc;
    MixerIf *mMix;

    // Configuration specific
    Bdc      *mBdc;
    Ddc100   *mDdc;
    Mboard   *mMbrd;
    Tboard   *mTbrd;

  public:

    DevMtdf();

    int      Open();
    int      Lock()             { return( mMutex.Lock() ); }
    int      Unlock()           { return( mMutex.Unlock() ); }

    int      GetComplexSampleRate();
    int      FlushSamples();
    int      Get2kSamples( short *dst );
    double   SetTuneHz( double freqHz );
    int      SetChannel( int chId );
};

#endif
