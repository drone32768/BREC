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

#include "../Util/gpioutil.h"
#include "prussdrv.h"
#include "pruss_intc_mapping.h"

#include "Fboard.h"
#include "pruinc.h"
#include "pru_images.h"


////////////////////////////////////////////////////////////////////////////////
/// Gpio base host spi port ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static int      fspiDbg = 0;
static GpioUtil hss;
static GpioUtil hsclk;
static GpioUtil hmosi[2];
static GpioUtil hmiso[2];

static int
fspi_init( )
{
    hss.Define(      117 );
    hsclk.Define(    110 );
    hmosi[0].Define( 113 );
    hmosi[1].Define( 115 );
    hmiso[0].Define( 111 );
    hmiso[1].Define( 112 );

    hss.Export();
    hsclk.Export();
    hmosi[0].Export();
    hmosi[1].Export();
    hmiso[0].Export();
    hmiso[1].Export();

    hss.SetDirInput(      0 );
    hsclk.SetDirInput(    0 );
    hmosi[0].SetDirInput( 0 );
    hmosi[1].SetDirInput( 0 );
    hmiso[0].SetDirInput( 1 );
    hmiso[1].SetDirInput( 1 );

    hss.Open();
    hsclk.Open();
    hmosi[0].Open();
    hmosi[1].Open();
    hmiso[0].Open();
    hmiso[1].Open();

    hss.Set(1);
    hsclk.Set(0);

    return(0);
}

static int 
fspi_select()
{
    hss.Set( 0 );
}

static int 
fspi_deselect()
{
    hss.Set( 1 );
}

static int 
fspi_xfer_byte( int wval )
{
    int rval;
    int obit,ibit,idx;

    if( fspiDbg ){
       printf("fspi_write: wval = 0x%04x\n",wval);
    }

    rval    = 0;

    // expecting: sclk=0, ss=0

    for( idx=7; idx>=0; idx-- ){
        if( fspiDbg ){
            printf("idx[%d]\n",idx);
        }

        if( wval&0x80 ) obit = 1;
        else            obit = 0;

        if( fspiDbg ){
            printf("   obit = %d\n",obit);
        }
        hmosi[0].Set( obit );
        hsclk.Set( 1 );
        ibit = hmiso[0].Get( );
        if( fspiDbg ){
            printf("   ibit = %d\n",ibit);
        }

        hsclk.Set( 0 );

        rval = (rval<<1) | ibit;
        wval = (wval<<1);
    }

    // expecting: sclk=0, ss=1

    return(rval);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define GetSramWord( off )    \
      (*(volatile unsigned int*  )((mPtrPruSram + (off))) )
#define SetSramWord( v,off )  \
      (*(volatile unsigned int*  )((mPtrPruSram + (off))) = (v) )
#define GetSramShort( off )   \
      (*(volatile unsigned short*)((mPtrPruSram + (off))) )
#define SetSramShort( v,off ) \
      (*(volatile unsigned short*)((mPtrPruSram + (off))) = (v) )
#define GetSramByte( off )    \
      (*(volatile unsigned char*)((mPtrPruSram + (off))) )
#define SetSramByte( v,off )  \
      (*(volatile unsigned char*)((mPtrPruSram + (off))) = (v) )

int
Fboard::StartPru()
{
    int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    printf("Fboard:StartPrus Enter\n");

    // Open pruss driver
    prussdrv_init();
    ret = prussdrv_open(PRU_EVTOUT_0);
    if( ret ){
        fprintf(stderr,"prussdrv_open failed\n");
        return(-1);
    }

    // Get pointers to dram and sram
    prussdrv_map_extmem( (void**)( &mPtrPruSamples ) );
    prussdrv_map_prumem( PRUSS0_PRU0_DATARAM, (void**)(&mPtrPruSram) );

    // Stop the prus
    prussdrv_pru_disable(0);

    // Init pru data
    prussdrv_pruintc_init(&pruss_intc_initdata);

    // Clear pru sram
    memset( (void*)mPtrPruSram, 0x0, 8192 );

    // Set specific required values
    SetSramWord( 0, SRAM_OFF_CMD1 );
    SetSramWord( 0, SRAM_OFF_CMD2 );

    // Write the instructions
    prussdrv_pru_write_memory(PRUSS0_PRU0_IRAM,0,
                             (unsigned int*)pru_image0,sizeof(pru_image0) );

    // Run/Enable prus
    prussdrv_pru_enable(0);

    printf("Xboard:StartPrus Exit\n");
    return( 0 );
}
////////////////////////////////////////////////////////////////////////////////
/// External Methods ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
Fboard::Fboard()
{
    mDbg    = 0xffffffff;
    mUsePru = 0;
}

//------------------------------------------------------------------------------
int
Fboard::Open()
{
    printf("Fboard:Open enter\n");

    // NOTE: we do not access jtag pins in this sw

    // Setup done
    mDoneGpio.Define( 31 /* gpio0_31 */ );
    mDoneGpio.Export();
    mDoneGpio.SetDirInput( 1 );
    mDoneGpio.Open();

    // Setup initb
    mInitbGpio.Define( 30 /* gpio0_30 */ );
    mInitbGpio.Export();
    mInitbGpio.SetDirInput( 1 );
    mInitbGpio.Open();

    // Figure out how we should access spi 
    if( FindCapeByName( "brecFpru" ) ) {
        mUsePru = 1;
        StartPru();
    }
    else{ 
        mUsePru = 0;
        fspi_init();
    }

    printf("Fboard:Open exit\n");

    return(0);
}

//------------------------------------------------------------------------------
void
Fboard::Close()
{
    printf("Fboard:Close enter\n");

    mDoneGpio.Close();
    mInitbGpio.Close();

    printf("Fboard:Close exit\n");
}

//------------------------------------------------------------------------------
void
Fboard::Reset()
{
    // 
    // NOTE: 
    // program_b is treated special since opening it
    // seems to pulse it low.
    // 

    printf("Fboard:Reset enter\n");

    mProgbGpio.Define( 50 /* gpio1_18 */ );
    mProgbGpio.Export();
    mProgbGpio.SetDirInput( 0 );
    mProgbGpio.Open();

    mProgbGpio.Set( 0 );
    us_sleep( 10*1000 );
    mProgbGpio.Set( 1 );

    mProgbGpio.Close();

    printf("Fboard:Reset exit\n");
}

//------------------------------------------------------------------------------
void
Fboard::Show()
{
    printf("Fboard:\n");
    printf("    LX9 done      = %d\n",mDoneGpio.Get() );
    printf("    LX9 init_b    = %d\n",mInitbGpio.Get() );
    printf("    brecFjtag DT  = %d\n",FindCapeByName( "brecFjtag" ) );
    printf("    brecFpru  DT  = %d\n",FindCapeByName( "brecFpru" ) );
    if( mUsePru ){
        printf("    PRU0 dbg1       0x%08x\n",GetSramWord( SRAM_OFF_DBG1 ) );
        printf("    PRU0 dbg2       0x%08x\n",GetSramWord( SRAM_OFF_DBG2 ) );
        printf("    PRU0 cmd1       0x%08x\n",GetSramWord( SRAM_OFF_CMD1 ) );
        printf("    PRU0 cmd1+4     0x%08x\n",GetSramWord( SRAM_OFF_CMD1+4 ) );
        printf("    PRU0 cmd2       0x%08x\n",GetSramWord( SRAM_OFF_CMD2 ) );
        printf("    PRU0 cmd2+4     0x%08x\n",GetSramWord( SRAM_OFF_CMD2+4 ) );
    }
}

//------------------------------------------------------------------------------
void
Fboard::SpiXfer8( unsigned char *bf, int bfCount )
{

   // GPIO based xfer
   if( !mUsePru ){
      int idx;

      fspi_select();
      for( idx=0; idx<bfCount; idx++ ){
          bf[idx] = fspi_xfer_byte( bf[idx] );
      }
      fspi_deselect();
   }

   // PRU based xfer
   else{
      int idx,cnt,limit;

      // Copy bytes in and issue byte count as command
      for(idx=0;idx<bfCount;idx++){
          SetSramByte( bf[idx], SRAM_OFF_CMD1+4+idx );
      }
      SetSramWord( bfCount, SRAM_OFF_CMD1 );

      // Wait for the command to complete
      cnt   = 0;
      limit = 1000; // wait for this many times nominal expected xfer time
      while( 0!=GetSramWord(SRAM_OFF_CMD1) && (cnt<1000) ){
          us_sleep(1 * bfCount);  // assume nominal 1us/byte
          cnt++;
      }
      if( cnt>=limit ){
         printf("%s:%d SpiXfer8 pru timeout\n",__FILE__,__LINE__);
      }

      // Copy out the results
      for(idx=0;idx<bfCount;idx++){
          bf[idx] = GetSramByte( SRAM_OFF_CMD1+4+idx );
      }

   }
}

