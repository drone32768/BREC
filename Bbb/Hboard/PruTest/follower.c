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
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include "pructl.h"

#include "../../Util/mcf.h"

//----------------------------------------------------------------------------
#define MAX_PATTERN_ERRS 10
static void *ddrMem;
static void *pru0DataMem;

//----------------------------------------------------------------------------
/**
 * This method copies 512 bytes from the source location to the destination
 * location using ldmi/stmia instructions to move 32 bytes at a time.
 */
void
copy512( unsigned short *src, unsigned short *dst )
{
    // NOTE: By specifying the clobber set, gcc will emit the
    // asm(" stmfd	sp!, { r3, r4, r5, r6, r7, r8, r9, r10 } ");
    // asm(" ldmfd	sp!, { r3, r4, r5, r6, r7, r8, r9, r10 } ");
    // surrounding the operation so we do not have to

    asm volatile (
      " ldmia	%[src]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t"    // 32
      " stmia	%[dst]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t" 
      " ldmia	%[src]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t"    // 64
      " stmia	%[dst]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t" 
      " ldmia	%[src]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t"    // 96
      " stmia	%[dst]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t" 
      " ldmia	%[src]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t"    // 128
      " stmia	%[dst]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t" 
      " ldmia	%[src]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t"    // 160
      " stmia	%[dst]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t" 
      " ldmia	%[src]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t"    // 192
      " stmia	%[dst]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t" 
      " ldmia	%[src]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t"    // 224
      " stmia	%[dst]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t" 
      " ldmia	%[src]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t"    // 256
      " stmia	%[dst]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t" 
      " ldmia	%[src]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t"    // 288
      " stmia	%[dst]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t" 
      " ldmia	%[src]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t"    // 320
      " stmia	%[dst]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t" 
      " ldmia	%[src]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t"    // 352
      " stmia	%[dst]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t" 
      " ldmia	%[src]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t"    // 384
      " stmia	%[dst]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t" 
      " ldmia	%[src]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t"    // 416
      " stmia	%[dst]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t" 
      " ldmia	%[src]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t"    // 448
      " stmia	%[dst]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t" 
      " ldmia	%[src]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t"    // 480
      " stmia	%[dst]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t" 
      " ldmia	%[src]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t"    // 512
      " stmia	%[dst]!, { r3, r4, r5, r6, r7, r8, r9, r10 } \n\t" 
      : [src] "+r" (src), [dst] "+r" (dst)           
      :
      : "r3","r4","r5","r6","r7","r8","r9","r10"   );
}

//----------------------------------------------------------------------------
/**
  * This method reads a number of samples from the head into the 
  * buffer provided.  The tailOffset value should be retained 
  * across invokations to maintain a stream or set to current value
  * of headOffset to flush the stream and obtain the most
  * current samples.
  *
  * The number of samples is returned.
  * The buffers should be at least page aligned.
  */
int get_sample_block( 
    volatile unsigned int *headOffsetPtr,
    unsigned int          *tailOffsetPtr, 
    unsigned short        *dst )
{
    unsigned long long *llsrc,*lldst;
    unsigned int        mask;
    unsigned int        hoff,toff;
    int                 bytes_in_block = 4096;

    // Create a mask for the number of bytes to be read
    mask = ~(bytes_in_block-1);

    // Wait until head != tail
    do{
       hoff = (*headOffsetPtr)&mask;
       toff = (*tailOffsetPtr)&mask;
    }while( hoff==toff );
    // printf("hoff=0x%08x toff=%08x mask=0x%08x\n",hoff,toff,mask);

    // Copy from current tail to provided destination
    lldst = (unsigned long long*)dst;
    llsrc = (unsigned long long*)( (unsigned char*)ddrMem + toff );

#   if 0
    // copy using 2 byte copies
    int idx;
    unsigned short *ssrc,*sdst;
    for(idx=0;idx<(bytes_in_block/2);idx++){
       *sdst++ = *ssrc++;
    }
#   endif

#   if 0
    // copy using 8 byte copies
    int idx;
    for(idx=0;idx<(bytes_in_block/8);idx++){
       *lldst++ = *llsrc++;
    }
#   endif

#   if 1 
    // copy using 32 byte copies
    int idx;
    for(idx=0;idx<(bytes_in_block/512);idx++){
      copy512( (unsigned short*)llsrc, (unsigned short*)lldst);
      llsrc+=64;
      lldst+=64;
    }
#   endif

    // Move tail forward
    *tailOffsetPtr=((*tailOffsetPtr)+bytes_in_block )%(2*PRU_MAX_SHORT_SAMPLES);

    return( bytes_in_block/sizeof(short) );
}
//----------------------------------------------------------------------------
/*
 * This method is provided a pointer to a buffer and the number of samples
 * in the buffer.  It verifies that the sequence of samples within the
 * buffer matches x[n] = x[n-1] + x[n-2]
 *
 * Returns: 0 on succes, 1 on patter error
 */
int verify_sequence( unsigned short *bf, int nSamp )
{
    unsigned short         expected;
    int                    idx;
 
    for(idx=2;idx<nSamp;idx++){
       expected = (bf[idx-1] + bf[idx-2])&0xffff;
       if( expected != bf[idx] ){
           printf("%d + %d -> %d but have %d at idx=%d\n",
              bf[idx-1],bf[idx-2], expected, bf[idx],idx);
           return( 1 );
       }
    }
    return( 0 );
}

//----------------------------------------------------------------------------
/*
 * This method attempts to follow the sample buffer in increments.
 * A block of samples is extracted, verified, and then another block
 * of samples is processed.  It does not have to maintain real-time
 * processing with the stream.
 *
 * Any detected errors are displayed on stdout.  
 * It returns once MAX_PATTERN_ERRS errors have been encountered.
 *
 */
void follow_sampled( )
{
    volatile unsigned int *headByteOffsetPtr;
    unsigned int           tailByteOffset;
    unsigned short         sample_bf[131072];
    unsigned short        *dst,*src;
    int                    curSamps;
    int                    errCount;
    int                    err,idx;
    int                    snapCount;
    int                    ns;
    unsigned long long    *llsrc,*lldst;

    // Setup basic operating variables
    dst            = sample_bf;
    curSamps       = 0;
    snapCount      = 0;
    errCount       = 0;
    headByteOffsetPtr = (unsigned int*)( 
                            (unsigned char*)pru0DataMem+PRU0_OFFSET_DRAM_HEAD );
    tailByteOffset = *headByteOffsetPtr;
    while( errCount < MAX_PATTERN_ERRS ){

      // Get a block of samples
      ns = get_sample_block( headByteOffsetPtr, &tailByteOffset, dst );
      curSamps += ns; 
      dst      += ns;

      // See if we have captured enough samples to verify
      if( curSamps >= 32768 ){ // 32768 ){
         err = verify_sequence( sample_bf, curSamps );
         if( err ){
            printf("snap error %d\n", snapCount);
            errCount++;
         }
         dst = sample_bf;
         curSamps = 0;
         snapCount++;

         tailByteOffset = *headByteOffsetPtr;
         dst = sample_bf;
      }
  
    } // End of main loop 

}

//----------------------------------------------------------------------------
/**
  * This method continuously follows the stream of PRU data checking
  * the pattern.
  *
  * It returns in once MAX_PATTERN_ERRS pattern errors have been encountered.
  */
void follow_stream( unsigned int usDelay )
{
    volatile unsigned int *headByteOffsetPtr;
    unsigned int           tailByteOffset;

    unsigned short         sample_bf[131072];
    unsigned short         r0,r1,r2;
    unsigned short         actual;
    
    int                    ns;
    int                    errCount;
    int                    idx;
    int                    synchronized;

    // Setup basic operating variables
    errCount       = 0;
    synchronized   = 0;
    headByteOffsetPtr = (unsigned int*)( 
                           (unsigned char*)pru0DataMem+PRU0_OFFSET_DRAM_HEAD );
    tailByteOffset = *headByteOffsetPtr;
    while( errCount < MAX_PATTERN_ERRS ){

        // Delay if specified
        if( usDelay!=0 ) us_sleep( usDelay );

        // Get a block of samples
        ns=get_sample_block( headByteOffsetPtr, &tailByteOffset, sample_bf );

        // Loop over all of the samples available in this block and verify
        for( idx=0;idx<ns;idx++ ){

            // Get the actual sample data
            actual = sample_bf[idx];

            // Calculate the expected sample
            r0 = (r1 + r2)&0xffff;

            // See if actual sample matches expected
            if( actual!=r0 ){
                if( 3==synchronized ){
                    printf("expected 0x%04x, read 0x%04x\n", r0, actual );
                    errCount++;
                    synchronized = 0;
                }
                else{
                    synchronized++;
                }
            }

            // Save off history for next expected computation
            r1 = r2;
            r2 = actual;

        }// End of loop over sample block
  
    } // End of main loop 

}

//----------------------------------------------------------------------------
/**
  * This method continuously checks a memory pattern
  */
void follow_ro( )
{
    unsigned short        *sample_bf;
    int                    err,idx;
    int                    bf_samps,off;
    long                   db,dt;
    int                    loopCnt;
    struct timeval         tv1,tv2;

    // Allocate and fill in large memory pattern
    bf_samps     = 8*1024*1024;
    sample_bf    = (unsigned short*)malloc( 2*bf_samps );
    if( !sample_bf ) return;
    sample_bf[0] = 0;
    sample_bf[1] = 1;
    for( idx=2; idx< bf_samps; idx++ ){
       sample_bf[idx] = sample_bf[idx-1] + sample_bf[idx-2];
    }

    // Loop forever
    loopCnt=0;
    db = 0;
    gettimeofday( &tv1, NULL );
    while(  1 ){

      // Conduct a pattern check on the memory
      err = verify_sequence( sample_bf+off, 32768 );
      if( err ){
         printf("loop error %d\n", loopCnt);
         return;
      }
      db  += 2*32768;
      off = (off+32768)%bf_samps;

      // Capture the current time
      gettimeofday( &tv2, NULL );

      // Calculate the time delta in microseconds
      dt = tv_delta_useconds(&tv2, &tv1);

      // If delta has exceeded a threshold, compute throughput
      if( dt > 1000000 ){

        // Show the results
        printf("ro MB/s = %f\n", (double)db/(double)dt);

        loopCnt++;

        // Reset starting size and time
        db=0;
        gettimeofday( &tv1, NULL );
      }

    } // End of main loop 
}

//----------------------------------------------------------------------------
/**
  * This method monitors the dram head pointer movement rate and prints
  * out the estimated throughput periodically. 
  *
  * It does not return
  */
void measure( )
{
    volatile unsigned int *hoff;
    unsigned int h1, h2;
    unsigned int db, dt;
    unsigned int count;
    struct timeval tv1,tv2;

    hoff  = (unsigned int*)((unsigned char*)pru0DataMem+PRU0_OFFSET_DRAM_HEAD);
    db    = 0;
    dt    = 1;
    count = 0;
    while( 1 ){

        // Capture the starting head location and time
        h1 = *hoff;
        gettimeofday( &tv1, NULL );

        // Wait or a bit
        us_sleep( 1000 );

        // Capture the starting ending location and time
        h2 = *hoff;
        gettimeofday( &tv2, NULL );

        // Calculate the head delta in bytes
        if( h2< h1 ){
           db = h2 + (h1 - 2*PRU_MAX_SHORT_SAMPLES);
        }
        else{
           db = h2 - h1;
        }

        // Calculate the time delta in microseconds
        dt = tv_delta_useconds(&tv2, &tv1);

        // Show the results
        printf("MB/s = %f\r", (double)db/(double)dt);
        if( 0==(count%1000) ) printf("\n");
        count++;
    }
}

//----------------------------------------------------------------------------
int main(int argc, char *argv[]  )
{
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
    void *vptr;	     
    void *pptr;	     

    int idx;
    int bMeasure = 0;
    int bStream  = 0;
    int bSample  = 0;
    int bMem     = 0;
    int usDelay  = 0;

    idx=1;
    while( idx< argc ){
        if( 0==strcmp(argv[idx],"-measure") ){
            bMeasure = 1;
        }

        if( 0==strcmp(argv[idx],"-stream") ){
            bStream = 1;
        }

        if( 0==strcmp(argv[idx],"-sample") ){
            bSample = 1;
        }

        if( 0==strcmp(argv[idx],"-mem") ){
            bMem = 1;
        }

        if( 0==strcmp(argv[idx],"-delay") && (idx+1)<argc ){
            usDelay = atoi( argv[idx+1] );
            idx++;
        }

        idx++;
    }	      
    printf("bMeasure= %d\n",bMeasure);
    printf("bStream = %d\n",bStream);
    printf("bSample = %d\n",bSample);
    printf("bMem    = %d\n",bMem);
    printf("usDelay = %d\n",usDelay);

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

    /* Setup DRAM memory */
    printf("\tINFO: setup mem \n");
    // line 215 extram_phys_base is phys and read from sysfs
    // line 231 extram_base      is virt and mmapped
	
    prussdrv_map_extmem( &vptr ); 
    printf("V extram_base = 0x%08x\n",(unsigned int) vptr);
    ddrMem = vptr;

    // Execute the test specified
    if( bMeasure ) measure();
    if( bSample )  follow_sampled();
    if( bStream )  follow_stream( usDelay );
    if( bMem    )  follow_ro();

    return(0);

}
