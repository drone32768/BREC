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
//
#ifndef __BDC_INCLUDE__
#define __BDC_INCLUDE__

#include "../Util/mcf.h"
#include "../Fboard/Fboard.h"


#define BDC_REG_RD  0x8000
#define BDC_REG_WR  0x4000

#define BDC_REG_R0  0x0000
#define BDC_REG_R1  0x0100
#define BDC_REG_R2  0x0200
#define BDC_REG_R3  0x0300
#define BDC_REG_R4  0x0400
#define BDC_REG_R5  0x0500
#define BDC_REG_R6  0x0600
#define BDC_REG_R7  0x0700
#define BDC_REG_R8  0x0800

#define BDC_REG_R9  0x0900
#define BDC_REG_R10 0x0A00
#define BDC_REG_R11 0x0B00
#define BDC_REG_R12 0x0C00

#define BDC_REG_R13 0x0D00
#define BDC_REG_R14 0x0E00
#define BDC_REG_R15 0x0F00

////////////////////////////////////////////////////////////////////////////////

#define BDC_GPIO0_DIR_WR (BDC_REG_WR | BDC_REG_R11)
#define BDC_GPIO1_DIR_WR (BDC_REG_WR | BDC_REG_R14)

#define BDC_GPIO0_OUT_WR (BDC_REG_WR | BDC_REG_R12)
#define BDC_GPIO1_OUT_WR (BDC_REG_WR | BDC_REG_R15)

#define BDC_GPIO0_INP_RD (BDC_REG_RD | BDC_REG_R10)
#define BDC_GPIO1_INP_RD (BDC_REG_RD | BDC_REG_R13)

////////////////////////////////////////////////////////////////////////////////
class Bdc {

private:

    ////////////////////////////////////////
    int                      mPidx;
    volatile unsigned short *mPtrPruSamples;
    volatile unsigned char  *mPtrPruSram;

    ////////////////////////////////////////
    Fboard                   mFbrd;
 
public:
    Bdc();

    int Open();
    int StartPrus();
    int StartPru();

    int GetFwVersion( );

    // For testing only
    int  SpiRW16( int wval );
    void Show( const char *title );

};

#endif
