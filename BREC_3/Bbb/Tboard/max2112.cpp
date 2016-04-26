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
#include "Ui2c/ui2c.h"

#include "max2112.h"

////////////////////////////////////////////////////////////////////////////////
MAX2112::MAX2112()
{
   mDbg  = 0xffffffff;

   mOscHz   = 20000000; // fixed by osc on board
   mXd      = 0; // divide osc by one, fixed by loop filter on board
   mDevAddr = 0xC0;
   mNdiv    = 90;
   mFdiv    = 0;
   mBbgDb   = 0;
   mLpfHz   = 4e6;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
MAX2112::Configure( UI2C *ui2c )
{
   mI2c = ui2c;
   return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t
MAX2112::Show()
{
   int          idx;
   uint8_t      bytes[32];
   uint32_t     err = 0;

   printf("%s:%d ----------------\n",__FILE__,__LINE__);
   for(idx=0;idx<14;idx++){
       err |= ReadReg( mDevAddr, idx, &(bytes[idx]), 1 );
       printf("reg[%02d] = 0x%02x\n",idx,bytes[idx]);
   }
   printf("FRAC   = %d\n",
                  (0x80&bytes[0])?1:0
         );
   printf("N      = %d\n", 
                  (((int)(bytes[0]&0x7f))<<8) + bytes[1] 
         );
   printf("CPMP   = %d\n", 
                  ( (bytes[2]>>6) & 0x3 ) 
         );
   printf("CPLIN  = %d\n", 
                  ( (bytes[2]>>4) & 0x3 ) 
         );
   printf("F      = %d\n", 
                  ( (((int)(bytes[2]&0x0f))<<16)+ 
                    (((int)(bytes[3]))<<8)      + 
                    bytes[4]) 
         );
   printf("XD     = %d\n", 
                  ( (bytes[5]>>5) & 0x7 ) 
         );
   printf("R      = %d\n", 
                  ( (bytes[5]   ) & 0x1f ) 
         );
   printf("PLL D24= %d\n", 
                  (0x80&bytes[6])?1:0
         );
   printf("PLL CPS= %d\n", 
                  (0x40&bytes[6])?1:0
         );
   printf("PLL ICP= %d\n", 
                  (0x20&bytes[6])?1:0
         );
   printf("VCO    = %d\n", 
                  ( (bytes[7]>>2) & 0x1f ) 
         );
   printf("VAS    = %d\n", 
                  (0x04&bytes[7])?1:0
         );
   printf("ADL    = %d\n", 
                  (0x02&bytes[7])?1:0
         );
   printf("ADE    = %d\n", 
                  (0x01&bytes[7])?1:0
         );
   printf("LPF    = %d\n", 
                  bytes[8]
         );
   printf("STBY   = %d\n", 
                  (0x80&bytes[9])?1:0
         );
   printf("PWDN   = %d\n", 
                  (0x20&bytes[9])?1:0
         );
   printf("BBG    = %d\n", 
                  (0x07&bytes[9])
         );
   printf("Shut   = %d\n", 
                  bytes[10]
         );
   printf("Test   = %d\n", 
                  bytes[11]
         );
   printf("POR    = %d\n", 
                  (0x80&bytes[12])?1:0
         );
   printf("VASA   = %d\n", 
                  (0x40&bytes[12])?1:0
         );
   printf("VASE   = %d\n", 
                  (0x20&bytes[12])?1:0
         );
   printf("LD     = %d\n", 
                  (0x10&bytes[12])?1:0
         );
   printf("VCOSBR = %d\n", 
                  ( (bytes[13]>>3) & 0x1f ) 
         );
   printf("ADC    = %d\n", 
                  ( (bytes[13]) & 0x7 ) 
         );

   printf("mDevAddr = 0x%02x\n",mDevAddr);
   printf("mNdiv    = %d\n",mNdiv);
   printf("mFdiv    = %d\n",mFdiv);
   printf("mXd      = %d\n",mXd);
   printf("mOscHz   = %d\n",mOscHz);
   printf("mFreqHz  = %f\n",mFreqHz);
   printf("mLpfHz   = %f\n",mLpfHz);
   printf("mBbgDb   = %d\n",mBbgDb);

   return( err );
} 

////////////////////////////////////////////////////////////////////////////////
void
MAX2112::ProgramDevice()
{
   uint8_t      byte;

   printf("Hz(tgt) = %f\n",mFreqHz);
   printf("N       = %d\n",mNdiv);
   printf("F       = %d\n",mFdiv);
   printf("Hz(act) = %f\n",mFreqHz);

   byte = 0x80 | ( (mNdiv>>8)&0x7f );
   WriteReg( mDevAddr, 0x00, &byte, 1 );

   byte = (mNdiv&0xff); 
   WriteReg( mDevAddr, 0x01, &byte, 1 );

   // See below for F

   byte = 0x01 | ((mXd<<5)&0xe0); 
   WriteReg( mDevAddr, 0x05, &byte, 1 );

   byte = 0x40 | ((mFreqHz>=1125e6)?0x00:0x80);
   WriteReg( mDevAddr, 0x06, &byte, 1 );

   // NOTE: With VASA the VCO value is 
   // the starting vco for auto select
   // Use the power on default value of 11001
   // and enable VAS
   byte = 0xCC;  
   WriteReg( mDevAddr, 0x07, &byte, 1 );

   // LPF cutoff freq = 4MHz + ( LPF - 12 ) * 290kHz
   byte = 0xff & (unsigned int)(12 + (mLpfHz - 4e6) / 290e3); 
   WriteReg( mDevAddr, 0x08, &byte, 1 );

   byte = (0x0f & mBbgDb); 
   WriteReg( mDevAddr, 0x09, &byte, 1 );

   byte = 0x00;  
   WriteReg( mDevAddr, 0x0A, &byte, 1 );

   byte = 0x08;  
   WriteReg( mDevAddr, 0x0B, &byte, 1 );

   /////////////////////
   byte = 0x10 | ((mFdiv>>16)&0x0f); 
   WriteReg( mDevAddr, 0x02, &byte, 1 );

   byte = ((mFdiv>>8)&0xff); 
   WriteReg( mDevAddr, 0x03, &byte, 1 );

   byte = ((mFdiv   )&0xff); 
   WriteReg( mDevAddr, 0x04, &byte, 1 );
   /////////////////////

   ReadReg( mDevAddr, 0x0C, &byte, 1 );
   printf("%s:%d:Status1 = 0x%02x\n",__FILE__,__LINE__,byte);

   ReadReg( mDevAddr, 0x0D, &byte, 1 );
   printf("%s:%d:Status2 = 0x%02x\n",__FILE__,__LINE__,byte);

}

////////////////////////////////////////////////////////////////////////////////
double
MAX2112::SetFreqHz( double freqHz )
{
   unsigned int refHz;
   double       del;

   mFreqHz = freqHz;

   refHz   = mOscHz; // NOTE: for xd = 0, div by 1
   mNdiv   = mFreqHz / refHz;
   del     = ( mFreqHz - ( mNdiv * refHz ) );
   mFdiv   = (1<<20) * del / refHz ;
   mFreqHz = ( (double)mNdiv * (double)refHz ) + 
                  ( (double)refHz * (double)mFdiv / (double)(1<<20) );

   printf("Ref(Hz) = %d\n",refHz);
   printf("del(Hz) = %f\n",del);

   ProgramDevice();
   return( mFreqHz );
}

////////////////////////////////////////////////////////////////////////////////
double
MAX2112::SetBwHz( double bwHz )
{
   if( bwHz <  3.48e6 ) bwHz =  3.48e6;
   if( bwHz > 74.47e6 ) bwHz = 74.47e6;

   mLpfHz = bwHz;

   ProgramDevice();
   return( bwHz );
}

////////////////////////////////////////////////////////////////////////////////
double
MAX2112::SetBbgDb( double gainDb )
{
   if( gainDb < 0  ) gainDb =  0;
   if( gainDb > 15 ) gainDb = 15;

   mBbgDb = ((int)(gainDb)) & 0xf;

   ProgramDevice();
   return( mBbgDb );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
MAX2112::WriteReg( 
   uint8_t  devAddr, 
   uint8_t  regAddr,  
   uint8_t *regBytes, 
   int      nBytes )
{
    int           err = 0;
    int           idx;

    err = mI2c->start_cond();
    if( err && mDbg ){
        printf("%s:%d: err start[1] 0x%08x\n",__FILE__,__LINE__,err);
        return( err );
    }

    err = mI2c->write_cycle( (devAddr&0xfe) );
    if( err && mDbg ){
        printf("%s:%d: err dev addr write cycle 0x%08x\n",
                             __FILE__,__LINE__,err);
        return( err );
    }

    err = mI2c->write_cycle( (regAddr&0xff) );
    if( err && mDbg ){
        printf("%s:%d: err reg addr write cycle 0x%08x\n",
                            __FILE__,__LINE__,err);
        return( err );
    }

    for(idx=0;idx<nBytes;idx++){
        err = mI2c->write_cycle( regBytes[idx] );
        if( err && mDbg ){
            printf("%s:%d: err value write cycle idx=%d/%d 0x%08x\n",
                            __FILE__,__LINE__,err,idx,nBytes);
        return( err );
        }
    }

    err = mI2c->stop_cond();
    if( err && mDbg ){
        printf("%s:%d: err stop_cond 0x%08x\n",__FILE__,__LINE__,err);
        return( err );
    }
 
    return( err );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
MAX2112::ReadReg( 
   uint8_t  devAddr, 
   uint8_t  regAddr, 
   uint8_t *regBytes, 
   int      nBytes )
{
    int           err = 0;
    int           idx;

// printf("MAX2112:ReadReg: dev   = 0x%02x\n", devAddr );
// printf("MAX2112:ReadReg: reg   = 0x%02x\n", regAddr );
// printf("MAX2112:ReadReg: bytes = 0x%02x\n", nBytes );

    err = mI2c->start_cond();
    if( err && mDbg ){
        printf("%s:%d: err start(1) 0x%08x\n",__FILE__,__LINE__,err);
    }

    err = mI2c->write_cycle( (devAddr&0xfe) );
    if( err && mDbg ){
        printf("%s:%d: err dev addr write cycle 0x%08x\n",
                             __FILE__,__LINE__,err);
    }

    err = mI2c->write_cycle( (regAddr&0xff) );
    if( err && mDbg ){
        printf("%s:%d: err reg addr write cycle 0x%08x\n",
                             __FILE__,__LINE__,err);
    }

us_sleep( 10000 );

    err = mI2c->start_cond();
    if( err && mDbg ){
        printf("%s:%d: err start(2) 0x%08x\n",__FILE__,__LINE__,err);
    }

    err = mI2c->write_cycle( (devAddr&0xfe) | 1 );
    if( err && mDbg ){
        printf("%s:%d: err write cycle R=1 0x%08x\n",__FILE__,__LINE__,err);
    }

    for(idx=0;idx<nBytes;idx++){
        err = mI2c->read_cycle( &(regBytes[idx]), (idx==(nBytes-1)?1:0) );
        if( err && mDbg ){
            printf("%s:%d: err read cycle idx=%d/%d 0x%08x\n",
                            __FILE__,__LINE__,err,idx,nBytes);
        }
    }

    err = mI2c->stop_cond();
    if( err && mDbg ){
        printf("%s:%d: err stop_cond 0x%08x\n",__FILE__,__LINE__,err);
    }

    return( err );
}

