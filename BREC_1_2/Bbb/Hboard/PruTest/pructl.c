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
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include "pructl.h"

//----------------------------------------------------------------------------
static void *ddrMem;
static void *pru0DataMem;
static void *pru1DataMem;

//----------------------------------------------------------------------------
void show_hex( unsigned char *bf, int bytes )
{
   int idx;
   unsigned short w;
   volatile unsigned char *vbf;
   
   vbf = bf;

   idx = 0;
   while( idx < bytes ){
       if( 0==(idx%16) ) printf("\n0x%08x ",idx);
       printf("%02x%02x ",vbf[idx+1],vbf[idx]);
       idx+=2;
   }
   printf("\n");
}

//----------------------------------------------------------------------------
void show_csv( unsigned char *bf, int bytes )
{
   int idx;
   unsigned short w;
   volatile unsigned char *vbf;

   idx = 0;
   while( idx < bytes ){
       w = ( (unsigned short)(vbf[idx+1])<<8 ) |  (unsigned short)(vbf[idx]);
       // w = w >> 2;
       printf("%d, %d\n",idx,w);
       idx+=2;
   }
   printf("\n");
}

//----------------------------------------------------------------------------
unsigned int getu32( void *ptr, int offset )
{
    unsigned int v;

    v = *( (unsigned int*)( ((volatile unsigned char*)ptr)+offset ));

    return( v );
}

//----------------------------------------------------------------------------
void setu32( void *ptr, int offset, unsigned int v )
{
    *( (unsigned int*)( ((volatile unsigned char*)ptr)+offset )) = v;
}

//----------------------------------------------------------------------------
int main(int argc, char *argv[]  )
{
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
    void *vptr;	     
    void *pptr;	     

    int idx;
    int bLoad0    = 0;
    int bLoad1    = 0;
    int bShow     = 0;
    int bStop     = 0;
    int bSnap     = 0;
    int bClean    = 0;
    int bCsv      = 0;
    int bShowSram = 0;
    int bSetSpin  = 0;
    int offset    = 0;
    int spinCount = 100;

    idx=1;
    while( idx< argc ){
        if( 0==strcmp(argv[idx],"-load0") ){
            bLoad0 = 1;
        }

        if( 0==strcmp(argv[idx],"-load1") ){
            bLoad1 = 1;
        }

        if( 0==strcmp(argv[idx],"-show") ){
            bShow = 1;
        }
        if( 0==strcmp(argv[idx],"-stop") ){
            bStop = 1;
        }
        if( 0==strcmp(argv[idx],"-snap") ){
            bSnap = 1;
        }
        if( 0==strcmp(argv[idx],"-clean") ){
            bClean = 1;
        }
        if( 0==strcmp(argv[idx],"-csv") ){
            bCsv = 1;
        }
        if( 0==strcmp(argv[idx],"-sram") ){
            char ch;
            bShowSram = 1;
            offset    = strtol( argv[idx+1], &ch, 0 );
        }
        if( 0==strcmp(argv[idx],"-spin") ){
            char ch;
            bSetSpin = 1;
            spinCount= strtol( argv[idx+1], &ch, 0 );
        }
        idx++;
    }	      
    printf("load0 = %d\n",bLoad0);
    printf("load1 = %d\n",bLoad1);
    printf("show  = %d\n",bShow);
    printf("stop  = %d\n",bStop);
    printf("show sram  = %d,%d\n",bShowSram,offset);
    printf("spin  = %d,%d\n",bSetSpin,spinCount);

    /* Initialize the PRU */
    printf("\tINFO: pruss init\n");
    prussdrv_init ();           
    
    /* Open PRU Interrupt */
    printf("\tINFO: pruss open\n");
    ret = prussdrv_open(PRU_EVTOUT_0);
    if (ret)
    {
        printf("prussdrv_open open failed\n");
        return (ret);
    }

    /* Get the interrupt initialized */
    printf("\tINFO: pruss intc init\n");
    prussdrv_pruintc_init(&pruss_intc_initdata);

    /* Initialize example */
    printf("\tINFO: mapping memory \n");
    prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pru0DataMem);
    prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, &pru1DataMem);

    /* Setup DRAM memory */
    printf("\tINFO: setup mem \n");
    // line 215 extram_phys_base is phys and read from sysfs
    // line 231 extram_base      is virt and mmapped
	
    prussdrv_map_extmem( &vptr ); 
    printf("V extram_base = 0x%08x\n",(unsigned int) vptr);
    ddrMem = vptr;

    pptr = (void*)prussdrv_get_phys_addr( vptr );
    printf("P extram_base = 0x%08x\n",(unsigned int) pptr);

    if( bClean ){
	memset( ddrMem, 0xa5, 0x100 );
    }	     

    if( bLoad0 ){

        // Set the number of bytes to capture
        setu32(pru0DataMem,PRU0_OFFSET_SRAM_HEAD,0);

        // Give the pru dram/ddr memory base
        setu32(pru0DataMem,PRU0_OFFSET_DRAM_PBASE,
                           prussdrv_get_phys_addr(ddrMem) );

	// Set spin count based on specified or defaul
        setu32(pru0DataMem, PRU0_OFFSET_SPIN_COUNT, spinCount );

        // Place marker s
        setu32(pru0DataMem, PRU0_OFFSET_RES1,      0xdeadbeef );
        setu32(pru0DataMem, PRU0_OFFSET_SRAM_TAIL, 0x0 );
        setu32(pru0DataMem, PRU0_OFFSET_DRAM_HEAD, 0x0 );
        setu32(pru0DataMem, PRU0_OFFSET_RES2,      0xbabedead );
        setu32(pru0DataMem, PRU0_OFFSET_RES3,      0xbeefcafe );

        show_hex( ((unsigned char*)pru0DataMem)+0x1000, 32 );

        printf("\tINFO: loading pru code \n");
        prussdrv_exec_program (0, "arm/pru00.bin");
    }

    if( bLoad1 ){
        printf("\tINFO: loading pru code \n");
        prussdrv_exec_program (1, "arm/pru01.bin");
    }	     
    
    if( bCsv ){
        show_csv( (unsigned char*)ddrMem, 8192 );
    }

    if( bSnap ){
        show_hex( ((unsigned char*)pru0DataMem)+0x1000, 32 );
        show_hex( ((unsigned char*)pru0DataMem)+0x0000, 0x100 );
        show_hex( (unsigned char*)ddrMem,               0x100 );
    }	     

    if( bStop ){
        /* Disable PRU and close memory mapping*/
        printf("\tINFO: closing pru \n");
        prussdrv_pru_disable (0);
        prussdrv_pru_disable (1);
        prussdrv_exit ();
    }

    if( bShow ){
	unsigned int v;

	printf("\n");
	printf("PRU  head   : 0x%08x\n",
                              getu32(pru0DataMem,PRU0_OFFSET_SRAM_HEAD) );
	printf("DRAM paddr  : 0x%08x\n",
                              getu32(pru0DataMem,PRU0_OFFSET_DRAM_PBASE) );
	printf("PRU spin    : 0x%08x\n",
                              getu32(pru0DataMem,PRU0_OFFSET_SPIN_COUNT) );
	printf("Res1        : 0x%08x\n",
                              getu32(pru0DataMem,PRU0_OFFSET_RES1) );
	printf("PRU tail    : 0x%08x\n",
                              getu32(pru0DataMem,PRU0_OFFSET_SRAM_TAIL) );
	printf("PRU->dram   : 0x%08x\n",
                              getu32(pru0DataMem,PRU0_OFFSET_DRAM_HEAD) );
	printf("Res2        : 0x%08x\n",
                              getu32(pru0DataMem,PRU0_OFFSET_RES2) );
	printf("Res3        : 0x%08x\n",
                              getu32(pru0DataMem,PRU0_OFFSET_RES3) );

	printf("SRAM excerpt");
        show_hex( ((unsigned char*)pru0DataMem)+0x0000, 0x20 );

	printf("DRAM excerpt");
        show_hex( (unsigned char*)ddrMem,               0x20 );
    }	      

    if( bShowSram ){
        printf("Relative to 0x%08x",offset);
        show_hex( ((unsigned char*)pru0DataMem)+offset, 0x100 );
    }

    if( bSetSpin ){
        setu32(pru0DataMem, PRU0_OFFSET_SPIN_COUNT, spinCount );
    }

    return(0);

}
