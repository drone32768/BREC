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

#include "Devs.h"

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
    if( FindCapeByName("brecX")>0  ){
/*** TODO
        printf("*********** Devs::Open Starting X board ****************\n");
        mAdc = new Xboard();
        mAdc->Open();
        mAdc->StartPrus();
        mAdc->SetComplexSampleRate( 5000000 );
        ((Xboard*)( mAdc ))->SetSource( 0 );
        ((Xboard*)( mAdc ))->SetFrequency( 10640000 );
*/
    }

    // x86 simulation
#   ifdef TGT_X86
    {
        printf("*********** Devs::Open Starting X board ****************\n");
/*** TODO
        mAdc = new Xboard();
        mAdc->Open();
        mAdc->StartPrus();
        mAdc->SetComplexSampleRate( 5000000 );
        // ((Xboard*)( mAdc ))->SetSource( 5 );
        ((Xboard*)( mAdc ))->SetSource( 0 );
        ((Xboard*)( mAdc ))->SetFrequency( 10640000 );
*/
    }
#   endif

    printf("*********** Devs::Open End *****************************\n");

    return(0);
}

//------------------------------------------------------------------------------

