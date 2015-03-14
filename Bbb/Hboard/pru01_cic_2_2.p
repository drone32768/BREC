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
.entrypoint MAIN1

#include "pru.hp"
#include "Hpru.h"

//-----------------------------------------------------------------------------
MAIN1:
    // Enable OCP master port
    LBCO      r0, CONST_PRUCFG, 4, 4
    CLR       r0, r0, 4  // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    SBCO      r0, CONST_PRUCFG, 4, 4

//   
//   SRAM
//           +-- rHeadPtr       +--- rHeadPtrPtr
//           |                  |  + -- rSpinPtr
//           |                  |  |
//           |                  |  |
//           V                  V  V
//   +-------------------------+-----------------------+  
//   |                         |                       |
//   +-------------------------+-----------------------+  
//    <---- rHeadMask -------->
//

#define rHeadPtr     r3
    MOV       rHeadPtr,     (0x2000)

#define rHeadMask    r4
    MOV       rHeadMask,    0x2fff

#define rHeadPtrPtr  r5
    MOV       rHeadPtrPtr,  (0x2000 + PRU0_OFFSET_SRAM_HEAD)    // 0x3000

#define rSpinPtr     r6
    MOV       rSpinPtr,     (0x2000 + PRU0_OFFSET_SPIN_COUNT)   // 0x3008

#define rSpinCnt     r7
    LD32      rSpinCnt, rSpinPtr

#define rSampMask    r8
    MOV       rSampMask, 0xfff
    
#define rSignExt     r9
    MOV       rSignExt,  0xfffff000

#define rSamp        r10
    MOV       rSamp, 0x0

#define rClkMask     r11
    MOV       rClkMask, 0x10000

#define rIntA        r12
    MOV       rIntA, 0x0

#define rIntB        r13
    MOV       rIntB, 0x0
    

    // Wait for sample clock
wait_high:  
    QBBC        wait_high, r31.t16               // 1

wait_low:  
    QBBS      wait_low, r31.t16                  // 1

    // Read the sample
    AND       rSamp, r31, rSampMask               // 1

    // Convert to 32 bit signed 
    QBBC      positive_label, rSamp.t11           // 1
    OR        rSamp, rSignExt, rSamp              // 1
positive_label:

    // Apply two stage integrator
    ADD       rIntA, rIntA, rSamp                 // 1
    ADD       rIntB, rIntB, rIntA                 // 1

    // Store the sample
    ST32      rIntB,  rHeadPtr                    // 2

    // Advance head
    ADD       rHeadPtr, rHeadPtr, 4               // 1
    AND       rHeadPtr, rHeadPtr, rHeadMask       // 1

    // Update sram with new head
    ST32      rHeadPtr, rHeadPtrPtr               // 2

    // Goto waiting for sample clock
    JMP       wait_high                           // 1

//
//            
//             +----------+          +----------+
//             |          |          |          |
//             |          |          |          |
//             |          |          |          |
//        -----+          +----------+          +----------+
//             ^          ^                     ^
//             |          |                     |
//         ADC samples    |<------ 100 nS------>|
//                        |     20 inst clk
//                        |     
//                    PRU read
//             
