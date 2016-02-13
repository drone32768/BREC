//
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
#ifndef __MAX2112_H__
#define __MAX2112_H__

class MAX2112 {

public:

    MAX2112();

    uint32_t Configure( UI2C *ui2c );

    uint32_t WriteReg( 
                    uint8_t  devAddr, 
                    uint8_t  regAddr, 
                    uint8_t *regBytes, 
                    int      nBytes );

    uint32_t ReadReg(  
                    uint8_t  devAddr, 
                    uint8_t  regAddr, 
                    uint8_t *regBytes, 
                    int      nBytes );

    double SetFreqHz( double freqHz );
    uint32_t Show();
    
#define MAX2112_DBG_WRITE 0x00000001
#define MAX2112_DBG_READ  0x00000002
private:
    uint32_t         mDbg;
    UI2C            *mI2c;
    int              mNdiv;    // integer divider
    int              mFdiv;    // fractional divider
    int              mDevAddr; // I2C device address
    int              mXd;      // ref osc divider to use
    int              mOscHz;   // ref osc freq in hertz
    int              mBbgDb;   // base band gain in db
    double           mLpfHz;   // LPF 3dB point in hertz
    double           mFreqHz;  // frequency in hertz
};

#endif
