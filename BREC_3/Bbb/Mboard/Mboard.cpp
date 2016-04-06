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
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/time.h>

#include "Mboard.h"

#include "../Util/gpioutil.h"
#include "../Util/mcf.h"  // for us_sleep

#include "Iboard/Iboard.h"
#include "Bdc/Bdc.h"


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// The Adf uses a static C method with a device number as an spi interface.  
// That library can support multiple instances of devices on the same
// spi with board.  The library is provided call out methods to write 
// a 32 bit spi word and to detect lock status.
//
static GpioPin *gMbrdMosi[2];     // miso pins for both units
static GpioPin *gMbrdSclk[2];     // serial clock for both units
static GpioPin *gMbrdStat[2];     // lock detect for both units
static GpioPin *gMbrdSsel[2];     // serial slave select for units
static int      gMbrdBitUs = 100; // us to sleep between bits

//----------------------------------------------------------------------------
static int MbrdSpiWrite( int pn, int log, uint32_t word )
{
    int nb;

    if( log ){
        printf("MbrdSpiWrite %d 0x%08x:\n",pn,word);
    }

    gMbrdSsel[pn]->Set(0);
    us_sleep( gMbrdBitUs );

    for(nb=0;nb<32;nb++){

        gMbrdMosi[pn]->Set( (word&0x80000000)?1:0 );
        us_sleep( gMbrdBitUs );

        gMbrdSclk[pn]->Set(1);
        us_sleep( gMbrdBitUs );

        gMbrdSclk[pn]->Set(0);
        us_sleep( gMbrdBitUs );

        word = (word<<1);
    }

    us_sleep( gMbrdBitUs );
    gMbrdSsel[pn]->Set(1);

    return(0);
}

//----------------------------------------------------------------------------
static int MbrdLockDetect( int pn )
{
    int v;
    v = gMbrdStat[pn]->Get();
    return(v);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
Mboard::Mboard()
{
    mIsOpen = false;
    mLog    = 0xffffffff;
}

//----------------------------------------------------------------------------
int Mboard::Attach(  void *lvl0, void *lvl1 )
{
    int portN;

    // Port to use is second level designator
    portN = (unsigned int)lvl1;

    printf("Mboard:Attach Enter ( port = %d)\n",portN);

    if( FindCapeByName( "brecFpru" ) || FindCapeByName( "brecFjtag" ) ){
       printf("Mboard:Open F board\n");

       Bdc     *bdc;

       bdc  = (Bdc*)lvl0;

       // subtract 3 from pin number (one for 0 based, 2 more for first
       // power and ground pin which are not gpios)

       gMbrdMosi[0] = bdc->GetGpioPin(portN,0); // p3 - 3 = 0
       gMbrdSclk[0] = bdc->GetGpioPin(portN,2); // p5 - 3 = 2
       gMbrdStat[0] = bdc->GetGpioPin(portN,5); // p8 - 3 = 5
       gMbrdSsel[0] = bdc->GetGpioPin(portN,3); // p6 - 3 = 3

    }
    else{
       printf("Mboard:Open I board\n");

       Iboard        *ibrd;
       Gpio6PinGroup *g6pg;

       ibrd = (Iboard*)lvl0;

       g6pg = ibrd->AllocPort( portN ); 
       ibrd->EnablePort( portN, 1 );

       gMbrdMosi[0] = g6pg->GetMoSi(); 
       gMbrdSclk[0] = g6pg->GetSclk();
       gMbrdStat[0] = g6pg->GetSs1();
       gMbrdSsel[0] = g6pg->GetSs1();

    }

    // Configure the pin directions
    gMbrdMosi[0]->SetDirInput(0);
    gMbrdSclk[0]->SetDirInput(0);
    gMbrdStat[0]->SetDirInput(1);
    gMbrdSsel[0]->SetDirInput(0);
   
    // Initialize the synthesizer device object
    mAdf4351.SetDev(0); // Single device on this board
    mAdf4351.SetLog( 0xffffffff );
    mAdf4351.Open( MbrdSpiWrite, MbrdLockDetect );

    return( 0 );
}
