//
//
// This source code is available under the "Simplified BSD license".
//
// Copyright (c) 2013, J. Kleiner
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

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "../Util/mcf.h"
#include "../Util/net.h"
#include "../Adf4351/Adf4351.h"

#include "../Aboard/Aboard.h"
#include "../Bboard/Bboard.h"

#include "../Hboard/Hboard.h"
#include "../Iboard/Iboard.h"
#include "../Mboard/Mboard.h"

#include "../Xboard/Xboard.h"
#include "Fir.h"

class Device : public McF {
  private:
    SimpleMutex   mDevLock; // Mutex for all device accesses
    int           mThreadExit;
    int           mComplex;
    Fir           miF3;
    Fir           mqF3;

    int           mDisplayCount;
    int           mPmPause;
    int           mPmLock;
    double        mPmTuneMHz;
    int           mSwapIQ;

    AdcIf         *mAdc;
    Adf4351       *mLo0;
    Adf4351       *mLo1;
    int            mNLO;

    int           TunerSet( long long freqHz );

    int           mXboard;
    int           mHboard;
    int           mAboard;

    void          GetSamples_A( short *sampPtr, int nSamples );
    void          GetSamples_H( short *sampPtr, int nSamples );
    void          GetSamples_X( short *sampPtr, int nSamples );
    
  public:
    Device();
    void          Main();
    void          RcvEvent( char *evtStr );
    void          GetSamples( short *sampPtr, int nSamples );

    void          PmReset();
    int           PmPauseCount();
    int           PmLockStatus();
    double        PmTuneMHz();
};

#endif
