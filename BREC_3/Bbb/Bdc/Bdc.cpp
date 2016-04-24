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

#include "Fboard/Fboard.h"
#include "Bdc.h"

#include "prussdrv.h"
#include "pruss_intc_mapping.h"

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
    int            ret;
    unsigned short fwVer,fwCom;
    int            pin,port;

    printf("Bdc:Open enter\n");

    // Open the lower level device
    mFbrd.Open();

    // Since we may just have powered on fpga let it load
    us_sleep( 100000 );

    // Show the underlying hw driver state
    mFbrd.Show();

    // Check hw version
    fwCom = GetFwCompat();
    printf("Bdc::Open::fw compatibility = 0x%08x\n",fwCom);

    fwVer = GetFwVersion();
    printf("Bdc::Open::fw version       = 0x%08x\n",fwVer);


    // Setup the gpio port constructs
    BdcGpio *gpio;
    for(port=0;port<BDC_GPIO_PORTS;port++){
        for(pin=0;pin<BDC_GPIO_PINS_PER_PORT;pin++){
            gpio = (BdcGpio*)( mGpioGroups[port].GetGpioPin(pin) );
            gpio->Init(this,port,pin);
        }
    }

    printf("Bdc:Open exit\n");

    return(0);
}

//------------------------------------------------------------------------------
int
Bdc::GetFwVersion()
{
    int ver;
    SpiRW16( BDC_REG_RD | BDC_REG_R1 );
    ver = SpiRW16(0);
    return(ver);
}

//------------------------------------------------------------------------------
int
Bdc::GetFwCompat()
{
    int ver;
    SpiRW16( BDC_REG_RD | BDC_REG_R0 );
    ver = SpiRW16(0);
    return(ver);
}

//------------------------------------------------------------------------------
volatile unsigned char *
Bdc::PruGetSramPtr()
{
    return( mFbrd.PruGetSramPtr() );
}

//------------------------------------------------------------------------------
volatile unsigned short *
Bdc::PruGetDramPtr()
{
    return( mFbrd.PruGetDramPtr() );
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
GpioGroup *
Bdc::GetPinGroup( int grp )
{
   return( &(mGpioGroups[grp]) );
}

////////////////////////////////////////////////////////////////////////////////
/// BdcGpioGroup ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
GpioPin*
BdcGpioGroup::GetGpioPin( int pin )
{
   return( &(mGpios[pin]) );
}

////////////////////////////////////////////////////////////////////////////////
/// BdcGpio ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void 
BdcGpio::Init( Bdc *bdc, int port, int pin )
{
    mBdc = bdc;
    mPort= port;
    mPin = pin;
}

//------------------------------------------------------------------------------
int
BdcGpio::Open()
{
    // Nothing to do
    printf("BdcGpio:Open %d:%d\n",mPort,mPin);
}

//------------------------------------------------------------------------------
int
BdcGpio::Close()
{
   // Nothing to do
}

//------------------------------------------------------------------------------
int
BdcGpio::SetDirInput( int isInput )
{
   int val;
   int rcmd,wcmd;

   // Setup r/w commands based on port
   if( 0==mPort ){
       wcmd = BDC_GPIO0_DIR_WR;
       rcmd = BDC_GPIO0_DIR_RD;
   }else{ 
       wcmd = BDC_GPIO1_DIR_WR;
       rcmd = BDC_GPIO1_DIR_RD;
   }

   // Read current value
   mBdc->SpiRW16( rcmd  );
   val = mBdc->SpiRW16( 0  );

   // Clear and reset bit
   val = (val & ~(1<<mPin) ) | ( (isInput?0:1)<<mPin);

   // Write new value
   mBdc->SpiRW16( wcmd | val  );

   return( 0 );
}

//------------------------------------------------------------------------------
int 
BdcGpio::Set( int v )
{
   int val;
   int rcmd,wcmd;

   // Setup r/w commands based on port
   if( 0==mPort ){
       wcmd = BDC_GPIO0_OUT_WR;
       rcmd = BDC_GPIO0_OUT_RD;
   }else{ 
       wcmd = BDC_GPIO1_OUT_WR;
       rcmd = BDC_GPIO1_OUT_RD;
   }

   // Read current value
   mBdc->SpiRW16( rcmd  );
   val = mBdc->SpiRW16( 0  );

   // Clear and reset bit
   val = (val & ~(1<<mPin) ) | ( (v&1)<<mPin);

   // Write new value
   mBdc->SpiRW16( wcmd | val  );

   return( 0 );
}

//------------------------------------------------------------------------------
int 
BdcGpio::Get( )
{
   int val;
   int rcmd;

   // Setup r/w commands based on port
   if( 0==mPort ){
       rcmd = BDC_GPIO0_INP_RD;
   }else{ 
       rcmd = BDC_GPIO1_INP_RD;
   }

   // Read current value
   mBdc->SpiRW16( rcmd  );
   val = mBdc->SpiRW16( 0  );

   // Select the specified bit
   val = (val>>mPin) & 0x1;

   return(val);
}
