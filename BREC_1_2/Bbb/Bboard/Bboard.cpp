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
#include "Bboard.h"
#include "../Util/mcf.h"  // for us_sleep

//----------------------------------------------------------------------------
// Internal forward decls.
static int p124_init();
static int p124_spi_delay();
static int p124_spi_clockbit( int bv );
static int p124_delta_usec( struct timeval *tv2, struct timeval *tv1 );

static int p124_get_ld( int dev );
static int p124_spi_write( int dev, int log, uint32_t word );

//----------------------------------------------------------------------------
#define MAX_4351_DEV 2
static int gnInitialized        = 0;

static GpioUtil gClkIo;
static GpioUtil gTxIo;
static GpioUtil gLdIo[ MAX_4351_DEV ];
static GpioUtil gCsIo[ MAX_4351_DEV ];

//----------------------------------------------------------------------------
static int p124_init()
{
    if( gnInitialized ) return( 0 );

    gLdIo[0].Open( 45 );
    gLdIo[1].Open( 44 );

    gClkIo.Open(   66 );
    gTxIo.Open(    67 );   

    gCsIo[0].Open( 3  );
    gCsIo[1].Open( 2  );

    gClkIo.Set( 0 );
    gCsIo[0].Set( 1 );
    gCsIo[1].Set( 1 );

    gnInitialized = 1;
    return( 0 );
}

//----------------------------------------------------------------------------
/**
 * This static method takes two timeval's and returns the difference
 * in signed integer microseconds.  First arg is latest time, second is 
 * earlier.
 * TODO - use common version
 */
static int p124_delta_usec( struct timeval *tv2, struct timeval *tv1 )
{
    int usec;

    usec  = 1000000*(tv2->tv_sec - tv1->tv_sec);
    usec += (tv2->tv_usec - tv1->tv_usec);
    return(usec);
}

//----------------------------------------------------------------------------
static int p124_spi_delay()
{
    struct timeval tv2, tv1;
    int dus;
    int cnt;

    cnt =0;
    gettimeofday( &tv1, NULL );
    do{
        gettimeofday( &tv2, NULL );
        dus = p124_delta_usec( &tv2, &tv1 );
	cnt++;
    }while( dus < 1 );
    // printf("delay cnt=%d\n",cnt);
    return(0);
}

//----------------------------------------------------------------------------
static int p124_spi_clockbit( int bv )
{
    gTxIo.Set( bv );

    p124_spi_delay();

    gClkIo.Set( 1 );

    p124_spi_delay();

    gClkIo.Set( 0 );

    p124_spi_delay();
    return(0);
}

//----------------------------------------------------------------------------
static int p124_spi_write( int dev, int log, uint32_t word )
{
    int nb;

    if( log ){
        printf("p124_spi_write %d 0x%08x:\n",dev,word);
    }

    gClkIo.Set( 0 );
    gCsIo[dev].Set( 0 );

    p124_spi_delay();
    for(nb=0;nb<32;nb++){
        p124_spi_clockbit( (word&0x80000000)?1:0 );
        word = (word<<1);
    }
    p124_spi_delay();

    gCsIo[dev].Set( 1 );
    gClkIo.Set( 0 );

    return(0);
}

//----------------------------------------------------------------------------
static int p124_get_ld( int dev )
{
    return( gLdIo[dev].Get() );
}


//----------------------------------------------------------------------------
Bboard::Bboard()
{
    printf("Bboard::Bboard()\n");
    mIsOpen = false;
}

//----------------------------------------------------------------------------
int Bboard::Open()
{
    p124_init();
    return( 0 );
}

//----------------------------------------------------------------------------
Adf4351 * Bboard::GetAdf4351( int dev )
{
    Adf4351 *adf4351;

    adf4351 = new Adf4351();
    adf4351->SetDev( dev );
    adf4351->Open( p124_spi_write, p124_get_ld );

    return( adf4351 );
}
