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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include "../Fboard/Fboard.h"
#include "Bdc.h"

#include "prussdrv.h"
#include "pruss_intc_mapping.h"
#include "pru_images.h"


////////////////////////////////////////////////////////////////////////////////
/// Hardware definitions ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define BDC_R0 0

#define GetSramWord( off )   ( *(unsigned int*  )((mPtrPruSram + (off))) )
#define SetSramWord( v,off ) ( *(unsigned int*  )((mPtrPruSram + (off))) = (v) )
#define GetSramShort( off )  ( *(unsigned short*)((mPtrPruSram + (off))) )

////////////////////////////////////////////////////////////////////////////////
/// External Methods ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
Bdc::Bdc()
{
}

//------------------------------------------------------------------------------
int
Bdc::Open()
{
    int ret;
    unsigned short fwVer;

    printf("Bdc:Open enter\n");

    mFbrd.Open();

    // Since we may just have powered on fpga let it load
    us_sleep( 100000 );

    mFbrd.Show();

    fwVer = GetFwVersion();
    printf("Bdc::Open::fw ver = 0x%08x\n",fwVer);

    printf("Bdc:Open exit\n");

    return(0);
}

//------------------------------------------------------------------------------
int
Bdc::GetFwVersion()
{
    int ver;
    ver = SpiRW16( BDC_R0 );
    return(ver);
}

//------------------------------------------------------------------------------
int
Bdc::StartPrus()
{
    // This method is not applicable.  pru00 is already started with fboard
    return( 0 );
}

//------------------------------------------------------------------------------
int
Bdc::SpiRW16( int wval )
{
    unsigned short sbf[256];
    int            rval;
 
    sbf[0] = (unsigned short)(wval&0xffff);
    mFbrd.SpiXferArray16( sbf, 1 );
    rval   = (int)( sbf[0] );

    return(rval);
}

//------------------------------------------------------------------------------
void
Bdc::Show(const char *title )
{
    printf("Bdc: %s",title);
    mFbrd.Show();
}

