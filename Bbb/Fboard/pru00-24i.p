//
//
// This source code is available under the "Simplified BSD license".
//
// Copyright (c) 2016, J. Kleiner
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

#include "pruconst.hp"
#include "pruinc.h"

//-----------------------------------------------------------------------------
//
// Bit defintions for later use
//
#define MOSI_B   3               // bit pos of MOSI
#define MISO_B   1               // bit pos of MISO
#define SCLK_H   0x01            // OR  into r30 to set high
#define SCLK_L   0xfe            // AND into r30 to set low
#define SS_H     0x80            // OR  into r30 to set high
#define SS_L     0x7f            // AND into r30 to set low

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
#define       rDbg1Ptr           r1
#define       rDbg2Ptr           r2
#define       rCmd1Ptr           r3
#define       rCmd2Ptr           r4

#define       rPtr               r5
#define       rCnt               r6
#define       rBytePtr           r7

#define       rArg0              r8
#define       rSO                r9
#define       rSI                r10
#define       rBc                r11

#define       rTmp1              r25
#define       rTmp2              r26

//  RESERVED                     r29 = return register

    MOV       rDbg1Ptr,          (0x0000 + SRAM_OFF_DBG1)
    MOV       rDbg2Ptr,          (0x0000 + SRAM_OFF_DBG2)
    MOV       rCmd1Ptr,          (0x0000 + SRAM_OFF_CMD1) 
    MOV       rCmd2Ptr,          (0x0000 + SRAM_OFF_CMD2) 

    MOV       rPtr,              0x0
    MOV       rCnt,              0x0
    MOV       rBytePtr,          0x0

    MOV       rArg0,             0x0
    MOV       rSO,               0x0
    MOV       rSI,               0x0
    MOV       rBc,               0x0

    MOV       rTmp1,             0x0
    MOV       rTmp2,             0x0

////////////////////////////////////////////////////////////////////////////////
// Main loop
// ARG: rCm1Ptr = pointer to command 1
// ARG: rCm2Ptr = pointer to command 1
// ARG: rDbg1Ptr= pointer to debug 1 word
// ARG: rDbg2Ptr= pointer to debug 2 word 
//
main_loop:

    // increment dbg1 every main loop pass
    LD32      rTmp1, rDbg1Ptr
    ADD       rTmp1, rTmp1,1
    ST32      rTmp1, rDbg1Ptr

    // check and process cmd1
    LD32      rCnt,  rCmd1Ptr   
    MOV       rPtr,  rCmd1Ptr
    QBNE      do_cmd,rCnt,0    

    // check and process cmd2
    LD32      rCnt,  rCmd2Ptr  
    MOV       rPtr,  rCmd2Ptr
    QBNE      do_cmd,rCnt,0  

    JMP       main_loop          // top of loop to re-check command

////////////////////////////////////////////////////////////////////////////////
// Execute a single command transfer
// ARG:   rPtr     = poiner to cmd (bytes start 4 after this)
// ARG:   rCnt     = number of bytes
// LOCAL: rBytePtr = pointer to current byte
//
do_cmd: 
    LD32      rTmp1, rDbg2Ptr
    ADD       rTmp1, rTmp1,1
    ST32      rTmp1, rDbg2Ptr

    ADD       rBytePtr, rPtr, 4   // bytes start 4 bytes after count
    AND       r30, r30, SS_L      // ss low

xfer_byte:
    LD8       rArg0, rBytePtr     // load the next byte to send
    CALL      shift8              // shift out/in a byte
    ST8       rArg0, rBytePtr     // save the received byte
    ADD       rBytePtr,rBytePtr,1 // inc the xfer byte pointer
    SUB       rCnt,rCnt,1         // dec the xfer byte count
    QBNE      xfer_byte,rCnt,0    // loop if there are more bytes to xfer

    OR        r30, r30, SS_H      // ss high
    MOV       rTmp1,0
    ST32      rTmp1, rPtr         // clear command bytes
    JMP       main_loop           // return to main loop 

//-----------------------------------------------------------------------------
// Shift out and in 8 bits
// ARG   : rArg0 = byte to shift out (in entry), byte shifted in (on exit)
// LOCAL : rSO
// LOCAL : rSI
// LOCAL : rBc
shift8:

    MOV       rSO, rArg0         // setup output word
    MOV       rSI, 0             // setup input word
    MOV       rBc,8              // initialize bit counter

clockbit:
    LSR       rTmp1,rSO,7        // 04 get so msb at bit 0
    AND       rTmp1,rTmp1,1      // 05 mask bit 0
    LSL       rTmp1,rTmp1,MOSI_B // 06 move msb to mosi bit loc
    MOV       r30,rTmp1          // 07 ** Set MOSI
    OR        rTmp2,rTmp2,rTmp2  // 08 nop
    OR        rTmp2,rTmp2,rTmp2  // 09 nop

    OR        rTmp2,rTmp2,rTmp2  // 10 nop
    OR        rTmp2,rTmp2,rTmp2  // 11 nop
    OR        r30, r30,SCLK_H    // 12 ** SCLK high

    OR        rTmp2,rTmp2,rTmp2  // 13 nop
    OR        rTmp2,rTmp2,rTmp2  // 14 nop
    OR        rTmp2,rTmp2,rTmp2  // 15 nop
    OR        rTmp2,rTmp2,rTmp2  // 16 nop
    OR        rTmp2,rTmp2,rTmp2  // 17 nop
    LSR       rTmp1,r31,MISO_B   // 18 ** Get MISO

    AND       rTmp1,rTmp1,1      // 19 mask of any other bits
    LSL       rSI,rSI,1          // 20 shift running input up
    OR        rSI,rSI,rTmp1      // 21 add the new bit to si 
    OR        rTmp2,rTmp2,rTmp2  // 22 nop
    OR        rTmp2,rTmp2,rTmp2  // 23 nop
    AND       r30, r30,SCLK_L    // 24 ** SCLK LOW

    LSL       rSO,rSO,1          // 01 shift SO to prep next bit
    SUB       rBc,rBc,1          // 02 dec the bit count
    QBNE      clockbit,rBc,0     // 03 if more bits, goto top of loop

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
