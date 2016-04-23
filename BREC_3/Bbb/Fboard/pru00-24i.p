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
#define MOSI_B   3               // bit pos of MOSI(0)
#define MOSI_C   5               // bit pos of MOSI(1)
#define MISO_B   1               // bit pos of MISO(0)
#define MISO_C   2               // bit pos of MISO(1)
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
#define       rCmdPtr            r3

#define       rCmdCode           r4
#define       rCnt               r5
#define       rDataPtr           r6

#define       rArg0              r7
#define       rSO                r8
#define       rSI                r9
#define       rBc                r10

#define       rTmp1              r25
#define       rTmp2              r26

//  RESERVED                     r29 = return register

    MOV       rDbg1Ptr,          (0x0000 + SRAM_OFF_DBG1)
    MOV       rDbg2Ptr,          (0x0000 + SRAM_OFF_DBG2)
    MOV       rCmdPtr,           (0x0000 + SRAM_OFF_CMD1) 

    MOV       rCmdCode,          0x0
    MOV       rCnt,              0x0
    MOV       rDataPtr,          0x0

    MOV       rArg0,             0x0
    MOV       rSO,               0x0
    MOV       rSI,               0x0
    MOV       rBc,               0x0

    MOV       rTmp1,             0x0
    MOV       rTmp2,             0x0

////////////////////////////////////////////////////////////////////////////////
// Main loop
// ARG: rCm1Ptr = pointer to 32 bit command 
// ARG: rDbg1Ptr= pointer to debug 1 word
// ARG: rDbg2Ptr= pointer to debug 2 word 
//
main_loop:

    // increment dbg1 every main loop pass
    LD32      rTmp1, rDbg1Ptr
    ADD       rTmp1, rTmp1, 1
    ST32      rTmp1, rDbg1Ptr

    // load and dispatch command
    LD16      rCmdCode, rCmdPtr            // load the command code
    ADD       rTmp1,rCmdPtr,2              // get pointer to xfer count
    LD16      rCnt, rTmp1                  // load the xfer count
    QBEQ      xfer_short_array2x,rCmdCode,PRU0_CMD_16ARRAY2x    
    QBEQ      xfer_short_array,  rCmdCode,PRU0_CMD_16ARRAY    
    QBEQ      xfer_byte_stream,  rCmdCode,PRU0_CMD_8STREAM    
    JMP       main_loop          

////////////////////////////////////////////////////////////////////////////////
// Execute a series of 16 bit transfers each with its own slave select
// ARG:   rCnt     = number of words
// ARG:   rCmdPtr  = pointer to cmd word (data is +4 from this)
// LOCAL: rDataPtr = pointer to current words
//
xfer_short_array2x:
    ADD       rDataPtr, rCmdPtr,4 // bytes start 4 bytes after command

    LD32      rTmp1, rDbg2Ptr     // DBG2 - load current value
    ADD       rTmp1, rTmp1, 2     // DBG2 - increment alue
    ST32      rTmp1, rDbg2Ptr     // DBG2 - store value

xfer_short2x:
    LD16      rArg0, rDataPtr     // load the next word to send
    LSL       rArg0,rArg0,16      // place bits in upper most position
    MOV       rBc, 16             // set number of bits to shift
    AND       r30, r30, SS_L      // ss low
    CALL      shiftbits2xb        // shift out/in a byte
    OR        r30, r30, SS_H      // ss high
    ST16      rArg0, rDataPtr     // save the received byte
    ADD       rDataPtr,rDataPtr,2 // inc the xfer byte pointer
    OR        rTmp2,rTmp2,rTmp2   // nop +01
    SUB       rCnt,rCnt,1         // dec the xfer count
    QBNE      xfer_short2x,rCnt,0 // loop if there are more bytes to xfer

    MOV       rTmp1,0             // load a zero
    ST16      rTmp1, rCmdPtr      // clear command bytes
    JMP       main_loop           // return to main loop 

////////////////////////////////////////////////////////////////////////////////
// Execute a series of 16 bit transfers each with its own slave select
// ARG:   rCnt     = number of words
// ARG:   rCmdPtr  = pointer to cmd word (data is +4 from this)
// LOCAL: rDataPtr = pointer to current words
//
xfer_short_array:
    ADD       rDataPtr, rCmdPtr,4 // bytes start 4 bytes after command

    LD32      rTmp1, rDbg2Ptr     // DBG2 - load current value
    ADD       rTmp1, rTmp1, 2     // DBG2 - increment alue
    ST32      rTmp1, rDbg2Ptr     // DBG2 - store value

xfer_short:
    LD16      rArg0, rDataPtr     // load the next word to send
    LSL       rArg0,rArg0,16      // place bits in upper most position
    MOV       rBc, 16             // set number of bits to shift
    AND       r30, r30, SS_L      // ss low
    CALL      shiftbits           // shift out/in a byte
    OR        r30, r30, SS_H      // ss high
    ST16      rArg0, rDataPtr     // save the received byte
    ADD       rDataPtr,rDataPtr,2 // inc the xfer byte pointer
    OR        rTmp2,rTmp2,rTmp2   // nop +01
    SUB       rCnt,rCnt,1         // dec the xfer count
    QBNE      xfer_short,rCnt,0   // loop if there are more bytes to xfer

    MOV       rTmp1,0             // load a zero
    ST16      rTmp1, rCmdPtr      // clear command bytes
    JMP       main_loop           // return to main loop 

////////////////////////////////////////////////////////////////////////////////
// Execute a single command transfer with an arbitrary number of bytes
// under a single slave select
// ARG:   rCnt     = number of bytes
// ARG:   rCmdPtr  = pointer to cmd word (data is +4 from this)
// LOCAL: rDataPtr = pointer to current words
//
xfer_byte_stream: 
    ADD       rDataPtr, rCmdPtr,4 // bytes start 4 bytes after command
    AND       r30, r30, SS_L      // ss low

    LD32      rTmp1, rDbg2Ptr     // DBG2 - load current value
    ADD       rTmp1, rTmp1, 2     // DBG2 - increment alue
    ST32      rTmp1, rDbg2Ptr     // DBG2 - store value

xfer_byte:
    LD8       rArg0, rDataPtr     // load the next byte to send
    LSL       rArg0,rArg0,24      // place bits in upper most position
    MOV       rBc, 8              // set number of bits to shift
    CALL      shiftbits           // shift out/in a byte
    ST8       rArg0, rDataPtr     // save the received byte
    ADD       rDataPtr,rDataPtr,1 // inc the xfer byte pointer
    SUB       rCnt,rCnt,1         // dec the xfer count
    QBNE      xfer_byte,rCnt,0    // loop if there are more bytes to xfer

    OR        r30, r30, SS_H      // ss high
    MOV       rTmp1,0             // load a zero
    ST16      rTmp1, rCmdPtr      // clear command bytes
    JMP       main_loop           // return to main loop 

//-----------------------------------------------------------------------------
// Shift out and in up to 32 bits
// ARG   : rArg0 = bits to shift out (starting at msb), shifted in bits on ret
// ARG   : rBc   = number of bits
// LOCAL : rSO
// LOCAL : rSI
shiftbits:

    MOV       rSO, rArg0         // setup output word
    MOV       rSI, 0             // setup input word

clockbit:
    LSR       rTmp1,rSO,31       // 04 get so msb at bit 0
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
    OR        rTmp2,rTmp2,rTmp2  // 18 nop
    LSR       rTmp1,r31,MISO_B   // 19 ** Get MISO

    AND       rTmp1,rTmp1,1      // 20 mask of any other bits
    LSL       rSI,rSI,1          // 21 shift running input up
    OR        rSI,rSI,rTmp1      // 22 add the new bit to si 
    OR        rTmp2,rTmp2,rTmp2  // 23 nop
    AND       r30, r30,SCLK_L    // 24 ** SCLK LOW

    LSL       rSO,rSO,1          // 01 shift SO to prep next bit
    SUB       rBc,rBc,1          // 02 dec the bit count
    QBNE      clockbit,rBc,0     // 03 if more bits, goto top of loop

    MOV       rArg0,rSI          // move serial input to return
    RET

//-----------------------------------------------------------------------------
// Shift out and in up to 32 bits using 2x spi
// ARG   : rArg0 = bits to shift out (starting at msb), shifted in bits on ret
// ARG   : rBc   = number of bits
// LOCAL : rSO
// LOCAL : rSI
shiftbits2xb:

    MOV       rSO, rArg0         // setup output word
    MOV       rSI, 0             // setup input word

clockbit2xb:
    LSR       rTmp1,rSO,28       // 02 get bit  (31->3)
    AND       rTmp1,rTmp1,8      // 03 mask bit

    LSR       rTmp2,rSO,25       // 04 get bit  (30->5)
    AND       rTmp2,rTmp2,32     // 05 mask bit 

    OR        r30,rTmp1,rTmp2    // 06 ** Set MOSI(0..1)
    LSL       rSO,rSO,2          // 07 shift SO to prep next out bits
    SUB       rBc,rBc,2          // 08 dec the bit count
    LSL       rSI,rSI,2          // 09 shift running input up
    OR        rTmp2,rTmp2,rTmp2  // 10 nop
    OR        rTmp2,rTmp2,rTmp2  // 11 nop
    OR        r30, r30,SCLK_H    // 12 ** SCLK high

    OR        rTmp2,rTmp2,rTmp2  // 13 nop
    OR        rTmp2,rTmp2,rTmp2  // 14 nop
    OR        rTmp2,rTmp2,rTmp2  // 15 nop
    OR        rTmp2,rTmp2,rTmp2  // 16 nop
    OR        rTmp2,rTmp2,rTmp2  // 17 nop
    OR        rTmp2,rTmp2,rTmp2  // 18 nop

    LSR       rTmp1,r31,0        // 19 ** Get MISO(0) (1 -> 1)
    LSR       rTmp2,r31,2        // 20 ** Get MISO(1) (2 -> 0)

    AND       rTmp1,rTmp1,2      // 21 mask of any other bits
    AND       rTmp2,rTmp2,1      // 22 mask of any other bits

    OR        rSI,rSI,rTmp1      // 23 add the new bit to si 
    OR        rSI,rSI,rTmp2      // 24 add the new bit to si 

    AND       r30, r30,SCLK_L    // 25 ** SCLK LOW

    QBNE      clockbit2xb,rBc,0  // 01 if more bits, goto top of loop

    MOV       rArg0,rSI          // move serial input to return
    RET

//-----------------------------------------------------------------------------
//      000000000011111111112222222222
//      012345678901234567890123456789
//             .    .     .     .
// MOSI 0000000VVVVVVVVVVVVVVVVVVVVV000
//             .    .     .     .
// MISO ------------------S------------
//             .    .     .     .
//   ---+      .    .     .     .
// SS   |      .    .     .     .
//      |      .    .     .     .
//      +------------------------------
//             .    .     .     .
//   ---+      .    +-----------+
// SCLK |      .    |     .     |
//      |      .    |     .     |
//      +-----------+     .     +------
//    SCLK    Set  SCLK   Get   SCLK
//    LOW    MOSI  HIGH  MISO   LOW
//
