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

#include "../Util/gpioutil.h"
#include "Mboard.h"
#include "../Util/mcf.h"  // for us_sleep

/*
This is a bit convoluted and not as would be with a full design 
contemplating all of the board types.  The Adf uses a static C method
with a device number as an spi interface.  Rather than redoing this
interface (initially) we will use the gpio port number as the device,
save the device number and then use that to access the different instances
of the M boards via the I board.
TODO: the Adf4351 software should use a different interface to allow
the port group to be saved directly.
*/

//----------------------------------------------------------------------------
// Internal forward decls.
static Gpio6PinGroup *gPinGrps[6];
static int p131_spi_delay();
static int p131_spi_clockbit( int pn, int bv );
static int p131_get_ld( int pn );
static int p131_spi_write( int pn, int log, uint32_t word );

//----------------------------------------------------------------------------
static int p131_spi_delay()
{
    struct timeval tv2, tv1;
    int dus;
    int cnt;

    cnt =0;
    gettimeofday( &tv1, NULL );
    do{
        gettimeofday( &tv2, NULL );
        dus = tv_delta_useconds( &tv2, &tv1 );
	cnt++;
    }while( dus < 1 );
    // printf("delay cnt=%d\n",cnt);
    return(0);
}

//----------------------------------------------------------------------------
static int p131_spi_clockbit( int pn, int bv )
{
    gPinGrps[pn]->GetMoSi()->Set( bv );
    p131_spi_delay();
    gPinGrps[pn]->GetSclk()->Set( 1 );
    p131_spi_delay();
    gPinGrps[pn]->GetSclk()->Set( 0 );
    p131_spi_delay();
    return(0);
}

//----------------------------------------------------------------------------
static int p131_spi_write( int pn, int log, uint32_t word )
{
    int nb;

    if( log ){
        printf("p131_spi_write %d 0x%08x:\n",pn,word);
    }

    gPinGrps[pn]->GetSclk()->Set( 0 );
    gPinGrps[pn]->GetSs1 ()->Set( 0 );

    p131_spi_delay();
    for(nb=0;nb<32;nb++){
        p131_spi_clockbit( pn, (word&0x80000000)?1:0 );
        word = (word<<1);
    }
    p131_spi_delay();

    gPinGrps[pn]->GetSs1 ()->Set( 1 );
    gPinGrps[pn]->GetSclk()->Set( 0 );

    return(0);
}

//----------------------------------------------------------------------------
static int p131_get_ld( int pn )
{
    int v;
    v = gPinGrps[pn]->GetStat()->Get();
    return(v);
}

//----------------------------------------------------------------------------
Mboard::Mboard()
{
    printf("Mboard::Mboard()\n");
    mIsOpen = false;
    mG6pg   = NULL;
}

//----------------------------------------------------------------------------
int Mboard::Open( Gpio6PinGroup *g6pg )
{
    int  pn;
    mG6pg = g6pg;

    pn    = mG6pg->GetGroupId();
    gPinGrps[ pn ] = g6pg;

    gPinGrps[pn]->GetSs1()->Set(  1 );
    gPinGrps[pn]->GetMoSi()->Set( 0 );
    gPinGrps[pn]->GetSclk()->Set( 0 );

    return( 0 );
}

//----------------------------------------------------------------------------
Adf4351 * Mboard::GetAdf4351( int dev )
{
    Adf4351 *adf4351;

    adf4351 = new Adf4351();
    adf4351->SetDev( mG6pg->GetGroupId() );
    adf4351->SetLog( 0xffffffff );
    adf4351->Open( p131_spi_write, p131_get_ld );

    return( adf4351 );
}

//----------------------------------------------------------------------------
unsigned int Mboard::SetLog( unsigned int lg )
{
   unsigned int prev;
   prev = mLog;
   mLog = lg;
   return( prev );
}
