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
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include "../Util/gpioutil.h"
#include "Xboard.h"

#include "Xpru.h"
#include "prussdrv.h"
#include "pruss_intc_mapping.h"
#include "pru_images.h"

//------------------------------------------------------------------------------
#define XSPI_FIFO_EN     0x8000
#define XSPI_FIFO_RST    0x4000

#define XSPI_LED0        0x0100
#define XSPI_LED1        0x0200
#define XSPI_LED2        0x0400
#define XSPI_LED3        0x0800

#define XSPI_SEL_P1_CNTR 0x0000
#define XSPI_SEL_P1_FIFO 0x0010

#define XSPI_READ        0x0008
#define XSPI_WRITE       0x0004
#define XSPI_PORT1       0x0001
#define XSPI_PORT0       0x0000

#define XSPI_WP0   (XSPI_WRITE | XSPI_PORT0)
#define XSPI_RP1   (XSPI_READ  | XSPI_PORT1)

#define XSPI_STOP      (               XSPI_SEL_P1_FIFO | XSPI_LED0 | XSPI_WP0 )
#define XSPI_FLUSH     (XSPI_FIFO_RST| XSPI_SEL_P1_FIFO | XSPI_LED0 | XSPI_WP0 )
#define XSPI_START     (XSPI_FIFO_EN | XSPI_SEL_P1_FIFO | XSPI_LED1 | XSPI_WP0 )
#define XSPI_READ_SAMPLE   (  XSPI_RP1 )

//------------------------------------------------------------------------------
Xboard::Xboard()
{
   mCSPS = 0;
}

//------------------------------------------------------------------------------
int
Xboard::SetGain( int gn )
{
   if( gn!=0 ){
       fprintf(stderr,"Xboard:SetGain invoked w/o 0 - err \n");
   }
   return( 0 );
}

//------------------------------------------------------------------------------
int
Xboard::Open()
{
    int ret;

    printf("Xboard:Open enter\n");

    prussdrv_init();
    ret = prussdrv_open(PRU_EVTOUT_0);
    if( ret ){
        fprintf(stderr,"prussdrv_open failed\n");
        return(-1);
    }
    prussdrv_map_extmem( (void**)( &mPtrPruSamples ) ); 
    prussdrv_map_prumem( PRUSS0_PRU0_DATARAM, (void**)(&mPtrPruSram) );
    mPtrHead = (unsigned int*)(mPtrPruSram + PRU0_OFFSET_DRAM_HEAD);

    printf("Xboard:Open exit\n");

    return(0);
}

//------------------------------------------------------------------------------
int
Xboard::Flush()
{
    mPidx = mPtrHead[0]/2;
    return(0);
}

//------------------------------------------------------------------------------
int 
Xboard::FlushSamples()
{
    Flush();
    return(0);
}

//------------------------------------------------------------------------------
// NOTE this is only for test purposes, it will not support high sample rates
int
Xboard::GetSamplePair( short *eo )
{
   *eo = 0;
   return( 0 );
}

//------------------------------------------------------------------------------
int
Xboard::Get2kSamples( short *bf )
{
    return(0);
}

//------------------------------------------------------------------------------
int
Xboard::StartPrus()
{
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    printf("Xboard:StartPrus Enter\n");

    // Stop the prus
    prussdrv_pru_disable(0);
    prussdrv_pru_disable(1);

    // Init pru data
    prussdrv_pruintc_init(&pruss_intc_initdata);

    // Initialize required ram values
    *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_DRAM_PBASE) ) = 
                                prussdrv_get_phys_addr((void*)mPtrPruSamples);
    *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_CMD) ) = 0;

    // Write the instructions
    prussdrv_pru_write_memory(PRUSS0_PRU0_IRAM,0,
                             (unsigned int*)pru_image0,sizeof(pru_image0) );
    prussdrv_pru_write_memory(PRUSS0_PRU1_IRAM,0,
                             (unsigned int*)pru_image1,sizeof(pru_image1) );

    // Run/Enable prus
    prussdrv_pru_enable(0);
    prussdrv_pru_enable(1);

    printf("Xboard:StartPrus Exit\n");
    return( 0 );
}

//------------------------------------------------------------------------------
int
Xboard::SetSim( int sim )
{
    return( 0 );
}

//------------------------------------------------------------------------------
int
Xboard::SetComplexSampleRate( int complexSamplesPerSecond )
{
    mCSPS = complexSamplesPerSecond;
    printf("Xboard:SetComplexSampleRate %d CSPS\n",mCSPS);

    return(0);
}

//------------------------------------------------------------------------------
int
Xboard::GetComplexSampleRate()
{
    return( mCSPS );
}

//------------------------------------------------------------------------------
int Xboard::GetRms( int nSamples, short *aSamples, double *rrms )
{
    return( 0 );
}

//------------------------------------------------------------------------------
int
Xboard::XspiWrite( int wval )
{
    int rval,bsy,cnt;
    int dbg1,dbg2;

    if( mXspiDbg ){
       printf("Xboard::XspiWrite: wval = 0x%04x\n",wval);
    }

    dbg1 = *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_DBG1) );
    dbg2 = *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_DBG1) );
    printf("Xboard::XspiWrite:Pre  dbg1=0x%08x, dbg2=0x%08x\n",dbg1,dbg2);

    *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_WRITE_VAL) ) = wval;
    *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_CMD) ) = 1;

    bsy  = 1;
    cnt  = 100;
    while( bsy && 0!=cnt ){
        bsy = *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_CMD) );
        if( bsy ){
            us_sleep( 1 );
            cnt--;
        }
    }

    if( 0!=cnt ){
        rval = *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_READ_VAL) );
    }
    else{
        rval = -1;
    }

    dbg1 = *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_DBG1) );
    dbg2 = *( (unsigned int*)(mPtrPruSram + PRU0_OFFSET_DBG1) );
    printf("Xboard::XspiWrite:Post dbg1=0x%08x, dbg2=0x%08x\n",dbg1,dbg2);

    if( mXspiDbg ){
        printf("Xboard::XspiWrite: rval = 0x%04x\n",rval);
    }

    return(rval);
}
