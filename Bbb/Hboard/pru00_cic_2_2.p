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

.setcallreg r29.w0

.origin 0
.entrypoint MAIN

#include "pru.hp"
#include "Hpru.h"

//-----------------------------------------------------------------------------
MAIN:

    // C24 - Local memory
    // Configure the block index register for PRU0 by setting 
    // c24_blk_index[7:0] and c25_blk_index[7:0] field to 0x00 and 0x00, 
    // respectively.  This will make C24 point to 0x00000000 (PRU0 DRAM) and 
    // C25 point to 0x00002000 (PRU1 DRAM).
    MOV       r0, 0x00000000
    MOV       r1, CTBIR_0
    ST32      r0, r1

//   
//   SRAM
//           +-- rTailPtr       +--- rHeadPtrPtr
//           |                  |  + -- rDrmOffsetPtrPtr
//           |                  |  |
//           |                  |  |
//           V                  V  V
//   +-------------------------+-----------------------+  
//   |                         |                       |
//   +-------------------------+-----------------------+  
//    <---- rTailMask -------->
//
//   DRAM
// 
//   +-- rDrmBasePtr
//   |
//   |<----- rDrmOffset -----|
//   |                       |
//   V                       V
//   +------------------------------------------------------------+
//   |                                                            |
//   +------------------------------------------------------------+
//    <----------------- rDrmOffsetMask ------------------------->
// 

#define    rTailMask    r8
    MOV       rTailMask, 0x0ffc

#define    rTailPtr     r9
    MOV       rTailPtr, 0x0

#define    rHeadPtrPtr  r10 
    MOV       rHeadPtrPtr, (0x0000 + PRU0_OFFSET_SRAM_HEAD)  // 0x1000

#define    rDrmOffsetMask r11
    MOV       rDrmOffsetMask, 0x0003ffff

#define    rDrmOffset     r12 
    MOV       rDrmOffset, 0x0000

#define    rDrmBasePtr    r13
    MOV       rDrmBasePtr, (0x0000 + PRU0_OFFSET_DRAM_PBASE) 
    LD32      rDrmBasePtr, rDrmBasePtr

#define    rDrmOffsetPtrPtr r14
    MOV       rDrmOffsetPtrPtr, (0x0000 + PRU0_OFFSET_DRAM_HEAD) 

#define    rNextPtr     r15
    MOV       rNextPtr, 0x0

#define    rIntB        r16
    MOV       rIntB , 0x00

#define    rCombA       r17
#define    rCombAn1     r18
#define    rCombAn2     r19

#define    rCombB       r20
#define    rCombBn1     r21
#define    rCombBn2     r22

#define    rDecVal      r23
    MOV       rDecVal,      (0x0000 + PRU0_OFFSET_DEC)          // 0x300C
    LD32      rDecVal, rDecVal
//    MOV       rDecVal, 4

#define    rDecCnt      r24
    MOV       rDecCnt, rDecVal

#define    rBias        r25
    MOV       rBias, 32768

#define    rSamp        r26

loop_label:

    // Wait here until head is in same block as our next ptr
    // NOTE: need to mask rTailMask since it include other pru mem high bits
    LD32      r2, rHeadPtrPtr                             // 2
    AND       r2, r2, rTailMask                           // 1
    QBEQ      loop_label, r2, rNextPtr                    // 1

    // Load 32 bit pru data from sram tail and advance
    LD32      rIntB , rTailPtr                            // 2
    ADD       rTailPtr, rTailPtr, 4                       // 1
    AND       rTailPtr, rTailPtr, rTailMask               // 1

    // Move forward to next input sample.
    // Set next to next block after tail
    ADD       rNextPtr, rTailPtr, 4                       // 1
    AND       rNextPtr, rNextPtr, rTailMask               // 1

    // Decimate
    SUB       rDecCnt, rDecCnt, 1                         // 1
    QBNE      loop_label, rDecCnt, 0                      // 1
    MOV       rDecCnt, rDecVal                            // 1

    // We only continue from here with decimated samples

    // Update first comb
    SUB       rCombA, rIntB , rCombAn2                    // 1
    MOV       rCombAn2, rCombAn1                          // 1
    MOV       rCombAn1, rIntB                             // 1

    // Update second comb
    SUB       rCombB, rCombA, rCombBn2                    // 1
    MOV       rCombBn2, rCombBn1                          // 1
    MOV       rCombBn1, rCombA                            // 1

    // Convert to 16 bit signed
    ADD       rSamp, rCombB, rBias                        // 1

    // Store results at ddr dst addr
    SBBO      rSamp, rDrmBasePtr, rDrmOffset, 2           // 1+? = 9 + latency
    ADD       rDrmOffset, rDrmOffset, 2                   // 1
    AND       rDrmOffset, rDrmOffset, rDrmOffsetMask      // 1

    // Store DDR fifo offset into sram
    ST32      rDrmOffset, rDrmOffsetPtrPtr                // 2

    // Do it all over again
    JMP       loop_label                                  // 1
