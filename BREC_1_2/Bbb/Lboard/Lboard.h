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
#ifndef __LBOARD_H__
#define __LBOARD_H__

#include <sys/types.h>

class Lboard {
private:
    int            mIsOpen;
#   define         LBRD_LOG_API     0x00000001
#   define         LBRD_LOG_ATTEN   0x00000002
#   define         LBRD_LOG_METER   0x00000004
    unsigned int   mLog;

public:
    Lboard();
    int          Open( Gpio6PinGroup *g6pg );
    unsigned int SetLog( unsigned int lg );
    int          SetAttenDb( float attenDb );
    float        GetMaxAttenDb();
    float        GetPwrCode();          
    float        GetPwrDbm();
    float        GetPwrDbm( float hintHz );          

private:
    int          mPwrSample;  // true=sample, false = divide
    double       mMvPerDbm;
    double       mDbmIntercept;

    // Local SPI interface code.  Not factored to common due to 
    // spi idiosynchracies of devices on this board (e.g. they
    // don't claim to be SPI compliant, rather just 3 wire interface).

    unsigned int   mSpiNsDelay;    
    Gpio6PinGroup   *mPinGrp;

    int             SpiDelay();
    int             SpiClockBitOut( int pn, int bv );
    int             SpiClockBitIn( int pn );
    int             SpiWrite( int pn, int log, uint32_t word, int nbits );
    int             SpiRead( int pn, int log, uint32_t *wword, int nbits );
};

#endif
