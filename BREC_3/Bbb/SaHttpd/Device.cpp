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

#include "Device.h"


////////////////////////////////////////////////////////////////////////////////
/// Devices ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static Device *gpDevice = NULL;

//------------------------------------------------------------------------------
Device *GetDev()
{
    if( !gpDevice ){
       gpDevice = new Device();
    }
    return( gpDevice );
}

//------------------------------------------------------------------------------
Device::Device()
{
}

//------------------------------------------------------------------------------
int Device::Open()
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
        printf("******** Device::Open Starting F/Bdc/Ddc100 ****************\n");

        mBdc = new Bdc();
        mBdc->Open();

        mDdc = new Ddc100();
        mDdc->Attach( mBdc );

        mTbrd = new Tboard();
        mTbrd->Attach( (void*)( mBdc->GetPinGroup(0) ), (void*)0 );

        mMbrd = new Mboard();
        mMbrd->Attach( (void*)( mBdc->GetPinGroup(1) ), (void*)0 );

        mDdc->Open();

        // Comment this line out to use CPU based SPI to samples
        mDdc->StartPru();
    }

    else{
        // Just terminate here since exceptions will result
        fprintf(stderr,"No recognized device tree\n");
        exit(-1);
    }

    printf("*********** Device::Open End *****************************\n");

    return(0);
}

