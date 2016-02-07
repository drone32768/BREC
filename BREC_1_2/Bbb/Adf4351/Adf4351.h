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
#ifndef __ADF4351_H__
#define __ADF4351_H__

#include <sys/types.h>

typedef int (*Adf4351SpiWriteFuncPtr)(int, int, uint32_t);
typedef int (*Adf4351GetLdFuncPtr)(int);

class Adf4351 {
private:
    int          mIsOpen;
    int          mDevNum; 
#   define       ADF_LOG_SPI_TESTS 0x00000001
#   define       ADF_LOG_FREQ_CALC 0x00000002
#   define       ADF_LOG_WRITES    0x00000004
    unsigned int mLog;

    unsigned int mRegs[6];
    int          mR;
    int          mInt;
    int          mMod;
    int          mFrac;
    int64_t      mFactHz;

    int          SpiTest();

    Adf4351SpiWriteFuncPtr mSpiWriteFunc;
    Adf4351GetLdFuncPtr    mGetLockFunc;

public:
    Adf4351();
    int          Open( Adf4351SpiWriteFuncPtr, Adf4351GetLdFuncPtr );
    void         SetDev( int dn );
    void         SetLog( unsigned int mask );
    int          GetLock();
    int64_t      SetFrequency( int64_t freqHz );
    int64_t      GetFrequency();
    int64_t      SetFrequencyWithLock( int64_t freqHz, int dus, int usTimeout );

    int          SetAuxPower( int pwr );
    int          SetAuxEnable( int enable );

    int          SetMainPower( int pwr );
    int          SetMainEnable( int enable );

    int          SetCpCurrent( int cur );
    int          SetMtld( int enable );

    int          SetPowerDown( int down );

    void         Show();
    int          SetLowSpurMode( int lsmode );

};

#endif
