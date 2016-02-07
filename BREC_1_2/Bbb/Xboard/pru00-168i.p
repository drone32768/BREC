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

#include "PruConst.hp"
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
//    clock out a read cmd and in the fifo sample
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
#define       r2k           r16
#define       rSampCnt      r17

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
    MOV       rSrHdPtr,          (0x0000)
    MOV       rSrHeadPtrPtr,     (0x0000 + SRAM_OFF_HEAD_PTR)
    MOV       rHeadMask,         (0x0fff)  // one 4k page of fifo
    MOV       r2k,               (0x800)   // 
    MOV       rSampCnt,          0
    // r29 = return register

main_loop: // primary loop

    // increment dbg1 every main loop pass
    LD32      rTmp1, rDbg1Ptr
    ADD       rTmp1,rTmp1,1
    ST32      rTmp1, rDbg1Ptr

    LD32      rTmp1, rCmdPtr     // Load command 
    QBEQ      wr_rd_16,rTmp1,1   // cmd = 1 goto wr_rd_16
    QBEQ      stream_fc, rTmp1,2 // cmd = 2 goto stream
    OR        rTmp2,rTmp2,rTmp2  // nop
    OR        rTmp2,rTmp2,rTmp2  // nop
    OR        rTmp2,rTmp2,rTmp2  // nop
    JMP       main_loop          // top of loop to re-check command

wr_rd_16:
    LD32      rArg0, rWr16Ptr    // load value to write
    CALL      xspi_wr_rd         // access the spi
    ST32      rArg0, rRd16Ptr    // store results
    MOV       rTmp1, 0x0         // load 0
    ST32      rTmp1,rCmdPtr      // write 0 to command
    JMP       main_loop          // goto main loop

//-----------------------------------------------------------------------------
// Uses fifo data flag indicator
#define XRDP0 0x8 // Xspi read port 0 command
#define XRDP1 0x9 // Xspi read port 1 command
#define XFRBIT 13 // R0 fifo ready bit
stream_fc:
    LD32      rTmp1, rCmdPtr     // load current pru command 
    QBNE      main_loop,rTmp1,2  // if cmd != 2 goto loop_label

    MOV       rArg0, XRDP0       // load write value (read port 0 = 0x8)
    CALL      xspi_wr_rd         // access the spi
    MOV       rArg0, XRDP0       // load write value (read port 0 = 0x8)
    CALL      xspi_wr_rd         // access the spi
    LSR       rTmp1,rArg0,XFRBIT // shift result down 
    AND       rTmp1,rTmp1,1      // mask for 1
    QBNE      stream_fc,rTmp1,1  // if the bit is not set goto loop top

read2k:
    MOV       rSampCnt,1
    MOV       rArg0, XRDP1       // load write value (read port 1 = 0x9)
    CALL      xspi_wr_rd         // access the spi (this load 1st fifo value)

loop2k:
    MOV       rArg0, XRDP1       // load write value (read port 1 = 0x9)
    CALL      xspi_wr_rd         // access the spi
    CALL      fifo_write         // store the spi read into fifo
    ADD       rSampCnt,rSampCnt,1 // inc counter
    QBNE      loop2k,r2k,rSampCnt // if < 2k samples loop

    MOV       rArg0, XRDP0       // load write value (read port 0 = 0x8)
    CALL      xspi_wr_rd         // access the spi (loads last fifo value)
    CALL      fifo_write         // store the spi read into fifo
    JMP       stream_fc          // have 2k samples, goto main loop
 
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

    AND       r30, r30, SS_LOW   // ss low
    MOV       rSO, rArg0         // setup output word
    MOV       rSI, 0             // setup input word
    MOV       rCnt,16            // initialize bit counter

clockbit:
    LSR       rTmp1,rSO,15       // 04 get so msb at bit 0
    AND       rTmp1,rTmp1,1      // 05 mask bit 0
    LSL       rTmp1,rTmp1,MOSI_B // 06 move msb to mosi bit loc
    MOV       r30,rTmp1          // 07 ** Set MOSI
    OR        rTmp2,rTmp2,rTmp2  // 08 nop
    OR        rTmp2,rTmp2,rTmp2  // 09 nop

    OR        rTmp2,rTmp2,rTmp2  // 10 nop
    OR        rTmp2,rTmp2,rTmp2  // 11 nop
    OR        rTmp2,rTmp2,rTmp2  // 12 nop
    OR        rTmp2,rTmp2,rTmp2  // 13 nop
    OR        rTmp2,rTmp2,rTmp2  // 14 nop
    OR        rTmp2,rTmp2,rTmp2  // 15 nop
    OR        rTmp2,rTmp2,rTmp2  // 16 nop
    OR        rTmp2,rTmp2,rTmp2  // 17 nop
    OR        rTmp2,rTmp2,rTmp2  // 18 nop
    OR        rTmp2,rTmp2,rTmp2  // 19 nop
    OR        rTmp2,rTmp2,rTmp2  // 20 nop
    OR        rTmp2,rTmp2,rTmp2  // 21 nop
    OR        rTmp2,rTmp2,rTmp2  // 22 nop
    OR        rTmp2,rTmp2,rTmp2  // 23 nop

    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10
    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10
    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10
    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10
    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10
    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10


    OR        r30, r30,SCLK_H    // 24 ** SCLK high

    OR        rTmp2,rTmp2,rTmp2  // 25 nop
    OR        rTmp2,rTmp2,rTmp2  // 26 nop
    OR        rTmp2,rTmp2,rTmp2  // 27 nop
    OR        rTmp2,rTmp2,rTmp2  // 28 nop
    OR        rTmp2,rTmp2,rTmp2  // 29 nop
    OR        rTmp2,rTmp2,rTmp2  // 30 nop
    OR        rTmp2,rTmp2,rTmp2  // 31 nop
    OR        rTmp2,rTmp2,rTmp2  // 32 nop
    OR        rTmp2,rTmp2,rTmp2  // 33 nop
    OR        rTmp2,rTmp2,rTmp2  // 34 nop
    OR        rTmp2,rTmp2,rTmp2  // 35 nop
    OR        rTmp2,rTmp2,rTmp2  // 36 nop
    OR        rTmp2,rTmp2,rTmp2  // 37 nop
    OR        rTmp2,rTmp2,rTmp2  // 38 nop

    OR        rTmp2,rTmp2,rTmp2  // 43 nop
    OR        rTmp2,rTmp2,rTmp2  // 44 nop
    OR        rTmp2,rTmp2,rTmp2  // 45 nop
    OR        rTmp2,rTmp2,rTmp2  // 46 nop
    OR        rTmp2,rTmp2,rTmp2  // 47 nop

    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10
    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10
    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10
    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10
    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10
    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10


    LSR       rTmp1,r31,MISO_B   // 39 ** Get MISO
    AND       rTmp1,rTmp1,1      // 40 mask of any other bits
    LSL       rSI,rSI,1          // 41 shift running input up
    OR        rSI,rSI,rTmp1      // 42 add the new bit to si 

    AND       r30, r30,SCLK_L    // 48 ** SCLK LOW

    LSL       rSO,rSO,1          // 01 shift SO to prep next bit
    SUB       rCnt,rCnt,1        // 02 dec the bit count
    QBNE      clockbit,rCnt,0    // 03 if more bits, goto top of loop

    OR        rTmp2,rTmp2,rTmp2  // nop
    OR        rTmp2,rTmp2,rTmp2  // nop
    OR        rTmp2,rTmp2,rTmp2  // nop
    OR        rTmp2,rTmp2,rTmp2  // nop

    OR        r30, r30, SS_HIGH  // ** SS HIGH
    OR        rTmp2,rTmp2,rTmp2  // 01 nops to ensure slave statemachine exec
    OR        rTmp2,rTmp2,rTmp2  // 02 nop
    OR        rTmp2,rTmp2,rTmp2  // 03 nop
    OR        rTmp2,rTmp2,rTmp2  // 04 nop
    OR        rTmp2,rTmp2,rTmp2  // 05 nop
    OR        rTmp2,rTmp2,rTmp2  // 06 nop
    OR        rTmp2,rTmp2,rTmp2  // 07 nop
    OR        rTmp2,rTmp2,rTmp2  // 08 nop
    OR        rTmp2,rTmp2,rTmp2  // 09 nop
    OR        rTmp2,rTmp2,rTmp2  // 10 nop
    OR        rTmp2,rTmp2,rTmp2  // 11 nop
    OR        rTmp2,rTmp2,rTmp2  // 12 nop
    OR        rTmp2,rTmp2,rTmp2  // 13 nop
    OR        rTmp2,rTmp2,rTmp2  // 14 nop
    OR        rTmp2,rTmp2,rTmp2  // 15 nop
    OR        rTmp2,rTmp2,rTmp2  // 16 nop
    OR        rTmp2,rTmp2,rTmp2  // 17 nop
    OR        rTmp2,rTmp2,rTmp2  // 18 nop
    OR        rTmp2,rTmp2,rTmp2  // 19 nop
    OR        rTmp2,rTmp2,rTmp2  // 21 nop
    OR        rTmp2,rTmp2,rTmp2  // 22 nop
    OR        rTmp2,rTmp2,rTmp2  // 23 nop
    OR        rTmp2,rTmp2,rTmp2  // 24 nop
    OR        rTmp2,rTmp2,rTmp2  // 25 nop
    OR        rTmp2,rTmp2,rTmp2  // 26 nop
    OR        rTmp2,rTmp2,rTmp2  // 26 nop
    OR        rTmp2,rTmp2,rTmp2  // 26 nop
    OR        rTmp2,rTmp2,rTmp2  // 26 nop
    OR        rTmp2,rTmp2,rTmp2  // 26 nop
    OR        rTmp2,rTmp2,rTmp2  // 26 nop
    OR        rTmp2,rTmp2,rTmp2  // 26 nop
    OR        rTmp2,rTmp2,rTmp2  // 26 nop
    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10
    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10
    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10
    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10
    OR        rTmp2,rTmp2,rTmp2  // +01
    OR        rTmp2,rTmp2,rTmp2  // +02
    OR        rTmp2,rTmp2,rTmp2  // +03
    OR        rTmp2,rTmp2,rTmp2  // +04
    OR        rTmp2,rTmp2,rTmp2  // +05
    OR        rTmp2,rTmp2,rTmp2  // +06
    OR        rTmp2,rTmp2,rTmp2  // +07
    OR        rTmp2,rTmp2,rTmp2  // +08
    OR        rTmp2,rTmp2,rTmp2  // +09
    OR        rTmp2,rTmp2,rTmp2  // +10

xspi_wr_rd_out:
    MOV       rArg0,rSI          // move serial input to return
    RET

//      000000000011111111112222222222333333333344444444444
//      012345678901234567890123456789012345678901234567890
//                .         .         .         .
// MOSI 0000000VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV000
//                .         .         .         .
// MISO ------------------------------S-------------
//                .         .         .         .
//      +         .         .         .         .
// SS   |         .         .         .         .
//      |         .         .         .         .
//      +------------------------------------------------------
//                .         .         .         .
//      +         .         +-------------------+
// SCLK |         .         |                   |
//      |         .         |                   |
//      +-------------------+                   +------------
//
// NOTE: 10 inst between transitions -> 200k words / sec ok
