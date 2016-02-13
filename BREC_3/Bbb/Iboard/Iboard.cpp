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
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/time.h>

#include "Util/gpioutil.h"
#include "Iboard.h"

//----------------------------------------------------------------------------
Iboard::Iboard()
{
    int idx;

    mLog         = 0xffffffff;

    for(idx=0;idx<IBOARD_NPORTS;idx++){
        mAllocated[idx]  = 0;
        mInitialized[idx]= 0;
    }
}

//----------------------------------------------------------------------------
int Iboard::Open()
{
    return( 0 );
}

//----------------------------------------------------------------------------
int Iboard::Init( int pn )
{
    if( mLog & IBRD_LOG_WRITES ){
       printf("### Initializing p130() ########################\n");
    }

    if( mInitialized[pn] ) return( 0 );

    if( 0==pn ){
        mPinGroup[0].DefineGroupId    ( 0  );
        mPinGroup[0].DefineMiSo       ( 50 );  // gpio_1_18 
        mPinGroup[0].DefineMoSi       ( 31 );  // gpio_0_31 
        mPinGroup[0].DefineSclk       ( 51 );  // gpio_1_19
        mPinGroup[0].DefineSs1        ( 30 );  // gpio_0_30
        mPinGroup[0].DefineSs2        ( 48 );  // gpio_1_16
        mPinGroup[0].DefineStat       ( 60 );  // gpio_1_28

        mPinGroupEnables[0].Define    ( 66 );  // gpio_2_2
        mPinGroupEnables[0].Export();
        mPinGroupEnables[0].SetDirInput(0);
        mPinGroupEnables[0].Open();
    }

    if( 1==pn ){

        mPinGroup[1].DefineMiSo       (  2 );  // gpio_0_2  
        mPinGroup[1].DefineMoSi       (  3 );  // gpio_0_3 
        mPinGroup[1].DefineSclk       ( 15 );  // gpio_0_15
        mPinGroup[1].DefineSs1        (  5 );  // gpio_0_5
        mPinGroup[1].DefineSs2        ( 49 );  // gpio_1_17
        mPinGroup[1].DefineStat       (  4 );  // gpio_0_4

        mPinGroupEnables[1].Define    ( 67 );  // gpio_2_3
        mPinGroupEnables[1].Export();
        mPinGroupEnables[1].SetDirInput(0);
        mPinGroupEnables[1].Open();
    }

    if( 2==pn ){
        mPinGroup[2].DefineGroupId    ( 2  );
        mPinGroup[2].DefineMiSo       (115 );  // gpio_3_19  
        mPinGroup[2].DefineMoSi       (112 );  // gpio_3_16 
        mPinGroup[2].DefineSclk       (111 );  // gpio_3_15
        mPinGroup[2].DefineSs1        (113 );  // gpio_3_17
        mPinGroup[2].DefineSs2        (110 );  // gpio_3_14
        mPinGroup[2].DefineStat       (117 );  // gpio_3_21

        mPinGroupEnables[2].Define    ( 68 );  // gpio_2_4
        mPinGroupEnables[2].Export();
        mPinGroupEnables[2].SetDirInput(0);
        mPinGroupEnables[2].Open();
    }

    mPinGroup[pn].GetMiSo()->Export();
    mPinGroup[pn].GetMoSi()->Export();
    mPinGroup[pn].GetSclk()->Export();
    mPinGroup[pn].GetSs1()->Export();
    mPinGroup[pn].GetSs2()->Export();
    mPinGroup[pn].GetStat()->Export();

    mInitialized[pn] = 1;

    return(0);
}

//----------------------------------------------------------------------------
int Iboard::EnablePort( int pn, int en )
{
    int err;
    if( pn<0 || pn>=IBOARD_NPORTS ) return( -1 );

    if( mLog & IBRD_LOG_WRITES ){
        printf("Iboard::EnablePort %d to %d\n",pn,en);
    }

    if( en ){
        // Enable power
        err = mPinGroupEnables[pn].Set( 0  );
 
        // Set input AND output directions
        mPinGroup[pn].GetMiSo()->SetDirInput(   1 );
        mPinGroup[pn].GetMoSi()->SetDirInput(   0 );
        mPinGroup[pn].GetSclk()->SetDirInput(   0 );
        mPinGroup[pn].GetSs1()->SetDirInput(    0 );
        mPinGroup[pn].GetSs2()->SetDirInput(    0 );
        mPinGroup[pn].GetStat()->SetDirInput(   1 );

        // Open
        mPinGroup[pn].GetMiSo()->Open();
        mPinGroup[pn].GetMoSi()->Open();
        mPinGroup[pn].GetSclk()->Open();
        mPinGroup[pn].GetSs1()->Open();
        mPinGroup[pn].GetSs2()->Open();
        mPinGroup[pn].GetStat()->Open();
    }

    else{
        // Close
        mPinGroup[pn].GetMiSo()->Close();
        mPinGroup[pn].GetMoSi()->Close();
        mPinGroup[pn].GetSclk()->Close();
        mPinGroup[pn].GetSs1()->Close();
        mPinGroup[pn].GetSs2()->Close();
        mPinGroup[pn].GetStat()->Close();

        // Set all pins to input 
        mPinGroup[pn].GetMiSo()->SetDirInput(   1 );
        mPinGroup[pn].GetMoSi()->SetDirInput(   1 );
        mPinGroup[pn].GetSclk()->SetDirInput(   1 );
        mPinGroup[pn].GetSs1()->SetDirInput(    1 );
        mPinGroup[pn].GetSs2()->SetDirInput(    1 );
        mPinGroup[pn].GetStat()->SetDirInput(   1 );

        // Disable power
        err = mPinGroupEnables[pn].Set( 1 );
    }

    return( err );
}

//----------------------------------------------------------------------------
Gpio6PinGroup * Iboard::AllocPort( int pn )
{
   if( pn>=0 && pn<IBOARD_NPORTS ){
      if( mAllocated[pn] ){
         fprintf(stderr,"Iboard::AllocPort %d already allocated\n",pn);
         return( NULL );
      }
      Init( pn );
      mAllocated[pn] = 1;
      return( &mPinGroup[pn] );
   }
   else{
      return( NULL );
   }
}

