/*
 *
 * This source code is available under the "Simplified BSD license".
 *
 * Copyright (c) 2013, J. Kleiner
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the original author nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>

#include "prussdrv.h"

volatile unsigned char *gDdrMem;
volatile unsigned char *gSramMem[2];

extern void SignalSimStart( void );

//----------------------------------------------------------------------------
int prussdrv_init( void )
{
    gSramMem[0] = (unsigned char*)malloc( 8*1024 );
    if(!gSramMem[0]) return(-1);

    gSramMem[1] = (unsigned char*)malloc( 8*1024 );
    if(!gSramMem[1]) return(-1);

    gDdrMem  = (unsigned char*)malloc( 8*1024*1024 );
    if(!gDdrMem) return(-1);

    SignalSimStart();

    return( 0 );
}

//----------------------------------------------------------------------------
int prussdrv_open( unsigned int evts )
{
    return(0);
}

//----------------------------------------------------------------------------
int prussdrv_pru_disable( unsigned int prunum )
{
    return(0);
}

//----------------------------------------------------------------------------
int prussdrv_pru_enable( unsigned int prunum )
{
    return(0);
}

//----------------------------------------------------------------------------
int prussdrv_pruintc_init( tpruss_intc_initdata * prussintc_init_data )
{
    return(0);
}

//----------------------------------------------------------------------------
int prussdrv_pru_write_memory(unsigned int pru_ram_id,
                                  unsigned int wordoffset,
                                  unsigned int *memarea,
                                  unsigned int bytelength)
{
    printf("pruss_emul: write memory %d 0x%08x\n",pru_ram_id,wordoffset);
    return(0);
}

//----------------------------------------------------------------------------
unsigned int prussdrv_get_phys_addr(void *address)
{
    return(0);
}


//----------------------------------------------------------------------------
int prussdrv_map_extmem( void ** addr )
{
   *addr = (void*)(gDdrMem);
    return( 0 );
}

//----------------------------------------------------------------------------
int prussdrv_map_prumem( unsigned int which, void **addr )
{
   *addr = (void*)(gSramMem[which]);
    return( 0 );
}

