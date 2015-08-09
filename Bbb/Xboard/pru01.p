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

#include "pru00.hp"
#include "Xpru.h"

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

    // r1 = tmp

    // r2 = tmp

    // r3 = sample value

    // r4 = sram data page base
    MOV       r4, 0x2000

    // r5 = sram fifo pointer 
    MOV       r5, (0x2000 + SRAM_OFF_SRAM_HEAD) // 0x3000

    // r6 = sram mask
    MOV       r6, 0xfff

    // r7 =  0x3010 sram
    MOV       r7, (0x2000 + SRAM_OFF_SRAM_TAIL) // 0x3010

    // r8 =  0x3014 sram
    MOV       r8, (0x2000 + SRAM_OFF_DRAM_HEAD) //  0x3014

    // r9  = pru1 reading sram
    MOV       r9, 0x00

    // r10 = ddr fifor offset addr
    MOV       r10, 0x00

    // r14 = ddr fifo base pointer (loaded from 0x3004 SRAM)
    MOV       r14, (0x2000 + SRAM_OFF_DRAM_PBASE) // 0x3004
    LD32      r14, r14

    // r15 = ddr mask (fixed internally at 256kB or 128k samples))
    MOV       r15, 0x0003ffff

#define   rTmp1     r1
#define   rDbg2Ptr  r8
    MOV       rTmp1, 0
    MOV       rDbg2Ptr,          (0x2000 + SRAM_OFF_DBG2)

main_loop:
    LD32      rTmp1, rDbg2Ptr
    ADD       rTmp1,rTmp1,1
    ST32      rTmp1, rDbg2Ptr
    JMP       main_loop


    // wait if we are at pru0 spot
    LD32      r2, r5              // load pru0's sram postion into r2
    ADD       r1, r1, 1           // NOP
    QBEQ      main_loop, r2, r9  // if pru1's sram position == pru0's loop

    // load pru0 data from sram and advance
    ADD       r9, r9, r4          // add sram base to sram offset
    LD16      r3, r9              // load from pru1's head to r3
    ADD       r9, r9, 2           // advance pru1 sram offset
    AND       r9, r9, r6          // wrap pru1 sram offset

//MOV r3, 0x33

    // Store and advance ddr dst pointer
    SBBO      r3,  r14, r10, 2   // Store results at ddr  dst addr
    ADD       r10, r10, 2        // Inc ddr dst addr 
    AND       r10, r10, r15      // Wrap ddr dst addr

    ST32      r9,  r7            // store sram reading location
    ST32      r10, r8            // store dram addr ofset into sram

    JMP       main_loop         // Do it all over again
