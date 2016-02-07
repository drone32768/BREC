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
.entrypoint MAIN

#include "pru00.hp"
#include "pructl.h"

//-----------------------------------------------------------------------------
//
// This pru module reads a serial word and stores the results in 
// the wrapping fifo memory from 0x0.0000 to 0x0.3fff.  The last address 
// written to within this range is stored at 0x1.0000
//
MAIN:
    // Enable OCP master port
    // LBCO      r0, CONST_PRUCFG, 4, 4
    // CLR    r0, r0, 4  // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    // SBCO      r0, CONST_PRUCFG, 4, 4

    // C24 - Local memory
    // Configure the block index register for PRU0 by setting 
    // c24_blk_index[7:0] and c25_blk_index[7:0] field to 0x00 and 0x00, 
    // respectively.  This will make C24 point to 0x00000000 (PRU0 DRAM) and 
    // C25 point to 0x00002000 (PRU1 DRAM).
    MOV       r0, 0x00000000
    MOV       r1, CTBIR_0
    ST32      r0, r1

    // r1 = tmp
    MOV       r1, 0x0

    // r2 = tmp
    MOV       r2, 0x0

    // r3 = call results
    MOV       r3, 0x0

    // r4 = UNUSED 
    MOV       r4, 0x0

    // r5 = sram fifo addr mask
    MOV       r5, 0x0fff    

    // r6 = UNUSED 
    MOV       r6, 0x0

    // r7 = UNUSED 
    MOV       r7, 0x0

    // r8 = UNUSED 
    MOV       r8, 0x0

    // r9  = sram location of spin count
    MOV       r9, PRU0_OFFSET_SPIN_COUNT // 0x1008

    // r10 = pointer to head location (where sample will be stored)
    MOV       r10, 0x0000

    // r11 = sram location of head index
    MOV       r11, 0x1000

    // r12 = UNUSED
    MOV       r12, 0x00

    // r13 = UNUSED
    MOV       r13, 0x0 

    // r14 = UNUSED
    MOV       r14, 0x00

    // r15 = UNUSED
    MOV       r15, 0x00

    // r16 = bit 16 mask
    MOV       r16, 1
    LSL       r16, r16, 16

    // r29 = return register

    // Main loop
loop_label:
    ST32      r10, r11           // r10 (head) in r11 (sram head)

    // Wait until cs is low
wait_cs_low:
    AND       r2, r31,0x04       // read and mask r31.2
    QBNE      wait_cs_low, r2, 0 // wait for r31.2 == 0

    // Read ADC word 
    // INSTR: 83 = 82 instructions + 1 instr for call 
    CALL      read_word_fast     // Read serial word into r3

    // Hold cs low specified spin count 
    // INSTR: 1 + 2*N instructions; N>=1 
    LD32      r2, r9             // Load spin count from sram
hold_cs_low:
    SUB       r2, r2, 1          // Decrement spin count
    QBNE      hold_cs_low, r2, 0 // Loop if count is not 0

    // Release our hold on cs 
    // NOTE: Need propogation delay before we re-read cs
    MOV       r30, 0x000000A0    // CSmask=1 (leave Clk=1)

    // Store and advance fifo head 
    // INSTR: 3 instructions
    ST16      r3,  r10           // store results at sram fifo head
    ADD       r10, r10, 2        // Inc ddr dst addr 
    AND       r10, r10, r5       // Wrap ddr dst addr

    // Wait until cs is high
wait_cs_high:
    AND       r2, r31,0x04       // read and mask r31.2
    QBNE      wait_cs_high, r2, 0x04 // wait for r31.2 != 0

    // Repeat loop
    JMP       loop_label         // Do it all over again


////////////////////////////////////////////////////////////////////////////////
// This method clocks out a 16 bit serial data word.
// It takes 16x5 + 2 instructions = 82 instructions = 400 nS. 
// return is counted in this but not the call instruction.
read_word_fast:
    MOV       r3, 0           // r3=result.  zero the value to start

    AND       r2, r31, r16    // Read SDAT
    MOV       r30, 0x00000000 // CLK=0
AND r1,r1,r1
    LSR       r2, r2, 1       // bit 16 -> bit 15
    MOV       r30, 0x00000020 // CLK=1
    OR        r3, r3, r2      // OR the bit into final position

    AND       r2, r31, r16    // Read SDAT
    MOV       r30, 0x00000000 // CLK=0
AND r1,r1,r1
    LSR       r2, r2, 2       // bit 16 -> bit 14
    MOV       r30, 0x00000020 // CLK=1
    OR        r3, r3, r2      // OR the bit into final position

    AND       r2, r31, r16    // Read SDAT
    MOV       r30, 0x00000000 // CLK=0
AND r1,r1,r1
    LSR       r2, r2, 3       // bit 16 -> bit 13
    MOV       r30, 0x00000020 // CLK=1
    OR        r3, r3, r2      // OR the bit into final position

    AND       r2, r31, r16    // Read SDAT
    MOV       r30, 0x00000000 // CLK=0
AND r1,r1,r1
    LSR       r2, r2, 4       // bit 16 -> bit 12
    MOV       r30, 0x00000020 // CLK=1
    OR        r3, r3, r2      // OR the bit into final position

    AND       r2, r31, r16    // Read SDAT
    MOV       r30, 0x00000000 // CLK=0
AND r1,r1,r1
    LSR       r2, r2, 5       // bit 16 -> bit 11
    MOV       r30, 0x00000020 // CLK=1
    OR        r3, r3, r2      // OR the bit into final position

    AND       r2, r31, r16    // Read SDAT
    MOV       r30, 0x00000000 // CLK=0
AND r1,r1,r1
    LSR       r2, r2, 6       // bit 16 -> bit 10
    MOV       r30, 0x00000020 // CLK=1
    OR        r3, r3, r2      // OR the bit into final position

    AND       r2, r31, r16    // Read SDAT
    MOV       r30, 0x00000000 // CLK=0
AND r1,r1,r1
    LSR       r2, r2, 7       // bit 16 -> bit 9
    MOV       r30, 0x00000020 // CLK=1
    OR        r3, r3, r2      // OR the bit into final position

    AND       r2, r31, r16    // Read SDAT
    MOV       r30, 0x00000000 // CLK=0
AND r1,r1,r1
    LSR       r2, r2, 8       // bit 16 -> bit 8
    MOV       r30, 0x00000020 // CLK=1
    OR        r3, r3, r2      // OR the bit into final position

    AND       r2, r31, r16    // Read SDAT
    MOV       r30, 0x00000000 // CLK=0
AND r1,r1,r1
    LSR       r2, r2, 9       // bit 16 -> bit 7
    MOV       r30, 0x00000020 // CLK=1
    OR        r3, r3, r2      // OR the bit into final position

    AND       r2, r31, r16    // Read SDAT
    MOV       r30, 0x00000000 // CLK=0
AND r1,r1,r1
    LSR       r2, r2, 10       // bit 16 -> bit 6
    MOV       r30, 0x00000020 // CLK=1
    OR        r3, r3, r2      // OR the bit into final position

    AND       r2, r31, r16    // Read SDAT
    MOV       r30, 0x00000000 // CLK=0
AND r1,r1,r1
    LSR       r2, r2, 11       // bit 16 -> bit 5
    MOV       r30, 0x00000020 // CLK=1
    OR        r3, r3, r2      // OR the bit into final position

    AND       r2, r31, r16    // Read SDAT
    MOV       r30, 0x00000000 // CLK=0
AND r1,r1,r1
    LSR       r2, r2, 12       // bit 16 -> bit 4
    MOV       r30, 0x00000020 // CLK=1
    OR        r3, r3, r2      // OR the bit into final position

    AND       r2, r31, r16    // Read SDAT
    MOV       r30, 0x00000000 // CLK=0
AND r1,r1,r1
    LSR       r2, r2, 13       // bit 16 -> bit 3
    MOV       r30, 0x00000020 // CLK=1
    OR        r3, r3, r2      // OR the bit into final position

    AND       r2, r31, r16    // Read SDAT
    MOV       r30, 0x00000000 // CLK=0
AND r1,r1,r1
    LSR       r2, r2, 12       // bit 16 -> bit 2
    MOV       r30, 0x00000020 // CLK=1
    OR        r3, r3, r2      // OR the bit into final position

    AND       r2, r31, r16    // Read SDAT
    MOV       r30, 0x00000000 // CLK=0
AND r1,r1,r1
    LSR       r2, r2, 13       // bit 16 -> bit 1
    MOV       r30, 0x00000020 // CLK=1
    OR        r3, r3, r2      // OR the bit into final position

    AND       r2, r31, r16    // Read SDAT
    MOV       r30, 0x00000000 // CLK=0
AND r1,r1,r1
    LSR       r2, r2, 14       // bit 16 -> bit 0
    MOV       r30, 0x00000020  // CLK=1 
    OR        r3, r3, r2       // OR the bit into final position
 
    RET       
