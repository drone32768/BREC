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
///
.setcallreg r29.w0

.origin 0
.entrypoint MAIN1

#include "PruConst.hp"
#include "Xpru.h"


//   
//   SRAM
//           +-- rTailPtr       +--- rSrHdPtrPtr
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

//-----------------------------------------------------------------------------
//
// This pru module reads a serial word and stores the results in 
// the wrapping fifo memory from 0x0.0000 to 0x0.3fff.  The last address 
// written to within this range is stored at 0x1.0000
//
MAIN1:
    // Enable OCP master port
    LBCO      r0, CONST_PRUCFG, 4, 4
    CLR       r0, r0, 4  // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    SBCO      r0, CONST_PRUCFG, 4, 4

#define    rDbg1Ptr         r4
#define    rDbg2Ptr         r5
#define    rTmp1            r6
#define    rTmp2            r7
#define    rTailMask        r8
#define    rTailPtr         r9
#define    rSrHdPtrPtr      r10 
#define    rDrmOffsetMask   r11
#define    rDrmOffset       r12 
#define    rDrmBasePtr      r13
#define    rDrmOffsetPtrPtr r14
#define    rNextPtr         r15
#define    rHeadMask        r16
#define    rSrTlPtrPtr      r17 

    MOV       rDbg2Ptr,          (0x2000 + SRAM_OFF_DBG2)
    MOV       rTailMask,         (0x2fff)
    MOV       rTailPtr,          (0x2000) 
    MOV       rSrHdPtrPtr,       (0x2000 + SRAM_OFF_HEAD_PTR)  
    MOV       rDrmOffsetMask,    (0x0003ffff)
    MOV       rDrmOffset    ,    (0x0000)
    MOV       rDrmBasePtr   ,    (0x2000 + SRAM_OFF_DRAM_PBASE) 
    LD32      rDrmBasePtr, rDrmBasePtr
    MOV       rDrmOffsetPtrPtr,  (0x2000 + SRAM_OFF_DRAM_HEAD) 
    MOV       rNextPtr,          0x0
    MOV       rHeadMask,         (0x0fff)
    MOV       rSrTlPtrPtr,       (0x2000 + SRAM_OFF_TAIL_PTR) 

main_loop:
    // increment dbg2 every loop pass
    LD32      rTmp1, rDbg2Ptr
    ADD       rTmp1,rTmp1,1
    ST32      rTmp1, rDbg2Ptr

    // save tail pointer to sram for debugging
    ST32      rTailPtr,rSrTlPtrPtr,

    // wait if tail == head
    LD32      rTmp1,rSrHdPtrPtr          // load other pru head
    AND       rTmp1,rTmp1,rHeadMask     // mask to offset
    AND       rTmp2,rTailPtr,rHeadMask  // mask to offset
    QBEQ      main_loop, rTmp1, rTmp2    // if our tail=other head, loop

    // load other pru data from sram and advance
    LD16      rTmp1, rTailPtr             // load sample
    ADD       rTailPtr, rTailPtr, 2       // inc tail
    AND       rTailPtr,rTailPtr,rTailMask // wrap tail

    // store and advance ddr dst pointer
    SBBO      rTmp1,rDrmBasePtr,rDrmOffset, 2       // store samp in drm
    ADD       rDrmOffset,rDrmOffset,2               // inc drm dst addr 
    AND       rDrmOffset,rDrmOffset,rDrmOffsetMask  // wrap dst addr

    // save dram head in sram so cpu can access
    ST32      rDrmOffset,rDrmOffsetPtrPtr   

    JMP       main_loop         // do it all over again
