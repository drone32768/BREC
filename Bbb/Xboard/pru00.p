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

#include "pru00.hp"
#include "Xpru.h"

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

//-----------------------------------------------------------------------------
// The basic structure is of a command word, write word and read word.
//
// The main loop does the following:
//    read command
//    if command == 1 the write and read spi 
//    if command == 2 stream
//    otherwise wait a bit
//    goto top of loop
// The spi write/read does the following:
//    get the value to write from sram
//    clock out the write value and in the read value
//    store the read value in sram
//    zero the sram command
//    return to main command loope
// The stream loop does the following:
//    clock out a 0x0 write and in the fifo sample
//    store sample in sram buffer for other pru
//    advance sample index
//    read command word
//    if command !=1 (stream) goto main loop
//    if fifo not empty return to top of loop
//
#define       rTmp1         r1
#define       rTmp2         r2
#define       rArg0         r3
#define       rCmdPtr       r4
#define       rWr16Ptr      r5
#define       rRd16Ptr      r6
#define       rSO           r7
#define       rSI           r8
#define       rCnt          r9
#define       rDbg1Ptr      r10
#define       rStkPtr       r12
#define       rSrHdPtr      r13
#define       rSrHeadPtrPtr r14
#define       rHeadMask     r15

    MOV       rTmp1,             0x0
    MOV       rTmp2,             0x0
    MOV       rArg0,             0x0
    MOV       rCmdPtr,           (0x0000 + SRAM_OFF_CMD) 
    MOV       rWr16Ptr,          (0x0000 + SRAM_OFF_WRITE_VAL) 
    MOV       rRd16Ptr,          (0x0000 + SRAM_OFF_READ_VAL) 
    MOV       rSO,               0x0
    MOV       rSI,               0x0
    MOV       rCnt,              0x0
    MOV       rDbg1Ptr,          (0x0000 + SRAM_OFF_DBG1)
    MOV       rStkPtr,           (0x0000 + SRAM_OFF_STACK)
    MOV       rSrHdPtr,          (0x0000 + SRAM_OFF_SRAM_HEAD)
    MOV       rSrHeadPtrPtr,     (0x0000 + SRAM_OFF_SHARED_PTR)
    MOV       rHeadMask,         (0x0fff)  // one 4k page of fifo

    // r29 = return register

main_loop: // primary loop
    LD32      rTmp1, rDbg1Ptr
    ADD       rTmp1,rTmp1,1
    ST32      rTmp1, rDbg1Ptr

    LD32      rTmp1, rCmdPtr     // Load command 
    QBEQ      wr_rd_16,rTmp1,1   // cmd = 1 goto wr_rd_16
    QBEQ      stream,  rTmp1,2   // cmd = 2 goto stream
    CALL      spin_wait          // no cmd, then pause a bit 
    JMP       main_loop          // top of loop to re-check command

wr_rd_16:
    LD32      rArg0, rWr16Ptr    // load value to write
    CALL      xspi_wr_rd         // access the spi
    ST32      rArg0, rRd16Ptr    // store results
    MOV       rTmp1, 0x0         // load 0
    ST32      rTmp1,rCmdPtr      // write 0 to command
    JMP       main_loop          // goto main loop

stream:
    LD32      rTmp1, rCmdPtr     // Load command 
    QBNE      main_loop,rTmp1,2  // if cmd != 2 goto loop_label
    MOV       rArg0, 0x9         // load write value (read port 1 = 0x9)
    CALL      xspi_wr_rd         // access the spi
    CALL      fifo_write         // store the spi read into fifo
    // TODO - check if fpga fifo is nearly empty (if so spin wait)
    JMP       stream;            // goto top of streaming loop
  
//-----------------------------------------------------------------------------
// This routine writes the 16 bits of rArg0 to the sram fifo
// Stack : none
//
fifo_write:
    ST16      rArg0,rSrHdPtr              // store sample at sram head
    ADD       rSrHdPtr,rSrHdPtr,2         // inc the head by 2
    AND       rSrHdPtr,rSrHdPtr,rHeadMask // wrap head
    ST32      rSrHdPtr,rSrHeadPtrPtr      // store head pt for other pru to use
    RET

//-----------------------------------------------------------------------------
// This routine spins on a register a fixed number of times.
// Stack : none.
//
spin_wait:
    MOV       rTmp1, 5         // load spin wait count
spwloop:
    SUB       rTmp1,rTmp1,1      // dec counter
    QBNE      spwloop, rTmp1, 0  // if counter not 0 loop back
    RET

//-----------------------------------------------------------------------------
// This routine clocks 16 bits of data out and in.  rArg0 is the write
// data and the read data is produced here.  
// Stack : 4 bytes
//
xspi_wr_rd:
#define MOSI_B   2
#define MISO_B   5
#define SS_HIGH  0x08            // OR  into r30 
#define SS_LOW   0xf7            // AND into r30 
#define SCLK_H   0x02            // OR  into r30
#define SCLK_L   0xfd            // AND into r30

    ST32      r29, rStkPtr       // save return pointer on stack
    ADD       rStkPtr,rStkPtr,4  // inc stack

    AND       r30, r30, SS_LOW   // ss low
    MOV       rSO, rArg0         // setup output word
    MOV       rSI, 0             // setup input word
    MOV       rCnt,16            // initialize bit counter

clockbit:
    CALL      spin_wait
    LSR       rTmp1,rSO,15       // get so msb at bit 0
    AND       rTmp1,rTmp1,1      // mask bit 0
    LSL       rTmp1,rTmp1,MOSI_B // move msb to mosi bit loc
    MOV       rTmp2,r30          // copy r30
    AND       rTmp2,rTmp2,0xb    // mask out the current value of mosi bit
    OR        rTmp2,rTmp2,rTmp1  // or in the new mosi bit 
    MOV       r30,rTmp1          // ** Set MOSI

    CALL      spin_wait          // setup time
    OR        r30, r30,SCLK_H    // ** SCLK high

    CALL      spin_wait          // hold time
    LSR       rTmp1,r31,MISO_B   // ** Get MISO

    AND       rTmp1,rTmp1,1      // mask of any other bits
    LSL       rSI,rSI,1          // shift running input up
    OR        rSI,rSI,rTmp1      // add the new bit to si 
    AND       r30, r30,SCLK_L    // ** SCLK LOW
    LSL       rSO,rSO,1          // shift to prep next bit

    SUB       rCnt,rCnt,1        // dec the bit count
    QBNE      clockbit,rCnt,0    // if more bits, goto top of loop
    OR        r30, r30, SS_HIGH  // ss high
xspi_wr_rd_out:
    MOV       rArg0,rSI          // move serial input to return
    SUB       rStkPtr,rStkPtr,4  // dec stack
    LD32      r29, rStkPtr       // fetch return pointer
    RET


