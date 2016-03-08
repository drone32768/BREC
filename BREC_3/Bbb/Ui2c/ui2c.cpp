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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "Util/mcf.h"
#include "Util/gpioutil.h"
#include "ui2c.h"

////////////////////////////////////////////////////////////////////////////////
UI2C::UI2C()
{
    mUsHold   = 100;
    mDbg      = 0x0;
    mGpioSCL  = NULL;
    mGpioSDA  = NULL;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t
UI2C::Dbg( uint32_t dbg )
{
   uint32_t pdbg;
   pdbg   = mDbg;
   mDbg   = dbg;
   return( pdbg );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
UI2C::wait_high( GpioPin *pin )
{
   int cnt;

   pin->SetDirInput( 1 );
   cnt = 10;
   while( cnt>0 ){
      if( 1==pin->Get() ) return(0);
      us_sleep( 100 );
      cnt--;
   }
   return( UI2C_ERR_HIGH_TOUT );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
UI2C::read_value( GpioPin *pin )
{
   pin->SetDirInput(1);
   return( pin->Get() );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
UI2C::pull_low( GpioPin *pin )
{
   pin->SetDirInput(0);
   pin->Set(0);
   return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t
UI2C::configure( GpioPin *scl, GpioPin *sda )
{
    // Save the two pins
    mGpioSCL = scl;
    mGpioSDA = sda;
 
    // Sanity check on init
    if( !mGpioSCL || !mGpioSDA ){
        printf("%s:%d gpios null\n",__FILE__,__LINE__);
        return( UI2C_ERR_CFG );
    }

    if( mDbg & UI2C_DBG_CFG ){
        printf("%s:%d:configure SCL=%d, SDA=%d\n",
                  __FILE__,__LINE__,
                  read_value(mGpioSCL),
                  read_value(mGpioSDA)  );
    }

    return(0);
}

////////////////////////////////////////////////////////////////////////////////
// At exit scl is high/input and sda is low/output
uint32_t
UI2C::start_cond()
{
    int err;

    if( mDbg & UI2C_DBG_START ){
        printf("%s:%d:start_cond issued \n",__FILE__,__LINE__);
    }

    // We need to do this to allow any slave's who just 
    // acked to release the bus.  This occurs if a start is issued
    // before a stop has been issued.
    pull_low( mGpioSCL ); 

    // Make sure SCL is high
    err = wait_high( mGpioSCL );
    if( err ){
        return( UI2C_ERR_START_CONDA );
    }

    // Make sure SDA is high
    err = wait_high( mGpioSDA );
    if( err ){
        return( UI2C_ERR_START_CONDB );
    }

    // Wait min time with SDA and SCL high
    us_sleep( mUsHold );

    // Pull SDA low
    pull_low( mGpioSDA );
    us_sleep( mUsHold );
   
    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
// At exit scl and sda are input/high
uint32_t
UI2C::stop_cond()
{
    int err;

    if( mDbg & UI2C_DBG_START ){
        printf("%s:%d:stop_cond issued \n",__FILE__,__LINE__);
    }
   
    // Pull clock low so we can change data
    pull_low( mGpioSCL );
    us_sleep( mUsHold );

    // Pull data low
    pull_low( mGpioSDA );
    us_sleep( mUsHold );

    // Allow clock high
    err = wait_high( mGpioSCL );
    if( err ){
        return( UI2C_ERR_STOP_COND );
    }
    us_sleep( mUsHold );

    // Allow data to transition high while clock high thus signalling stop
    err = wait_high( mGpioSDA );
    if( err ){
        return( UI2C_ERR_STOP_COND );
    }
    us_sleep( mUsHold );

    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
// At exit scl and sda are input/high
uint32_t
UI2C::write_cycle( uint8_t byte )
{
    int           err = 0;
    int           cnt;
    int           ab;
    int           bit;

    if( mDbg & UI2C_DBG_WRITE_CYCLE ){
        printf("%s:%d:write_cycle begin: val=0x%02x\n",
                  __FILE__,__LINE__,byte);
    }

    // Verify SCL is high at start
    err = wait_high( mGpioSCL );
    if( err ){
       return( UI2C_ERR_WRITE_A );
    }

    for(cnt=0;cnt<8;cnt++){

        // SCL low
        pull_low( mGpioSCL );
        us_sleep( mUsHold );

        // SDA transition
        bit = ((byte&0x80)>>7);
        if( mDbg & UI2C_DBG_WRITE_CYCLE ) printf("%d ",bit);
        if( bit ){
           wait_high( mGpioSDA );
        }
        else{
           pull_low( mGpioSDA );
        }
        us_sleep( mUsHold );

        // SCL high
        err = wait_high( mGpioSCL );
        if( err ){
            return( UI2C_ERR_WRITE_B );
        }
        us_sleep( mUsHold );

        byte = byte << 1;
    }
    if( mDbg & UI2C_DBG_WRITE_CYCLE ) printf("\n");

    // SCL is high at this point

    // Pull SCL low so slave can drive SDA
    // And turn SDA into an input (to stop driving it)
    pull_low( mGpioSCL );
    mGpioSDA->SetDirInput(1);   
    us_sleep( mUsHold );

    // SCL high
    err = wait_high( mGpioSCL );
    if( err ){
        return( err );
    }

    // Get SDA state to check slave ACK
    ab = mGpioSDA->Get();       
    if( ab ){
        return( UI2C_ERR_WRITE_C );
    }

    if( mDbg & UI2C_DBG_WRITE_CYCLE ){
        printf("%s:%d:write_cycle end  : err=%d\n",
              __FILE__,__LINE__,err);
    }

    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
// At exit scl is input/high, sda is low/output(ack) or high/input(nack)
uint32_t
UI2C::read_cycle( uint8_t *bytePtr, int nack )
{
    int           err = 0;
    int           cnt;
    int           bit;
    uint8_t       byte = 0;

    err = wait_high( mGpioSCL );
    if( err ){
       return( UI2C_DBG_READ_CYCLE );
    }

    // Start first low clock so we can release SDA
    pull_low( mGpioSCL ); 
    mGpioSDA->SetDirInput(1);   
    us_sleep( mUsHold ); 

    for(cnt=0;cnt<8;cnt++){

        err = wait_high( mGpioSCL );
        us_sleep( mUsHold );

        bit = mGpioSDA->Get();
        byte = (byte<<1) | bit; 

        pull_low( mGpioSCL ); 
        us_sleep( mUsHold );
    }

    pull_low( mGpioSCL ); 
    if( nack ){
        err = wait_high( mGpioSDA );
    }
    else{
        pull_low( mGpioSDA ); 
    }

    err = wait_high( mGpioSCL );
    us_sleep( mUsHold );

    *bytePtr = byte;
    return( err );
}

