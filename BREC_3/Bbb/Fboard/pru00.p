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

////////////////////////////////////////////////////////////////////////////////
//
// Internal definitions (bit defintions for later use)
//
#define MOSI_B   3               // bit pos of MOSI(0)
#define MOSI_C   5               // bit pos of MOSI(1)
#define MISO_B   1               // bit pos of MISO(0)
#define MISO_C   2               // bit pos of MISO(1)
#define SCLK_H   0x01            // OR  into r30 to set high
#define SCLK_L   0xfe            // AND into r30 to set low
#define SS_H     0x80            // OR  into r30 to set high
#define SS_L     0x7f            // AND into r30 to set low

////////////////////////////////////////////////////////////////////////////////
//
// Entry point
//
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
//
// Register use and default values
//
#define       rDbg1Ptr           r1
    MOV       rDbg1Ptr,          (0x0000 + PRU0_LOFF_DBG1)

#define       rDbg2Ptr           r2
    MOV       rDbg2Ptr,          (0x0000 + PRU0_LOFF_DBG2)

#define       rCmdPtr            r3
    MOV       rCmdPtr,           0x0

#define       rCmdCode           r4
    MOV       rCmdCode,          0x0

#define       rCnt               r5
    MOV       rCnt,              0x0

#define       rDataPtr           r6
    MOV       rDataPtr,          0x0

#define       rArg0              r7
    MOV       rArg0,             0x0

#define       rSO                r8
    MOV       rSO,               0x0

#define       rSI                r9
    MOV       rSI,               0x0

#define       rBc                r10
    MOV       rBc,               0x0

#define       rCmdPtr1           r11
    MOV       rCmdPtr1,          (PRU0_LOFF_CMD1) 

#define       rCmdPtr2           r12
    MOV       rCmdPtr2,          (PRU0_LOFF_CMD2) 

#define       rTmp1              r25
    MOV       rTmp1,             0x0

#define       rTmp2              r26
    MOV       rTmp2,             0x0

//  RESERVED                     r29 = return register

////////////////////////////////////////////////////////////////////////////////
// Main loop
// ARG: rCm1Ptr = pointer to 32 bit command 
// ARG: rDbg1Ptr= pointer to debug 1 word
// ARG: rDbg2Ptr= pointer to debug 2 word 
//
// This loop services external requests for SPI commands.
// See the interface description in pruinc.h
//
main_loop:

    // Increment dbg1 every main loop pass
    LD32      rTmp1, rDbg1Ptr
    ADD       rTmp1, rTmp1, 1
    ST32      rTmp1, rDbg1Ptr

    // Load and dispatch command from cpu
    MOV       rCmdPtr, rCmdPtr1
    LD16      rCmdCode, rCmdPtr            // load the command code
    ADD       rTmp1,rCmdPtr,2              // get pointer to xfer count
    LD16      rCnt, rTmp1                  // load the xfer count
    QBEQ      xfer_short_array2x,rCmdCode,PRU0_CMD_16ARRAY2x    
    QBEQ      xfer_short_array,  rCmdCode,PRU0_CMD_16ARRAY    
    QBEQ      xfer_byte_stream,  rCmdCode,PRU0_CMD_8STREAM    

    // Load and dispatch command from pru1
    MOV       rCmdPtr, rCmdPtr2
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
    CALL      shiftbits2x         // shift out/in a byte
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

////////////////////////////////////////////////////////////////////////////////
/// Serial bit shifting ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
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
    OR        r30, r30,SCLK_H    // 11 ** SCLK high

    OR        rTmp2,rTmp2,rTmp2  // 12 nop
    OR        rTmp2,rTmp2,rTmp2  // 13 nop
    OR        rTmp2,rTmp2,rTmp2  // 14 nop
    OR        rTmp2,rTmp2,rTmp2  // 15 nop
    OR        rTmp2,rTmp2,rTmp2  // 16 nop
    LSR       rTmp1,r31,MISO_B   // 17 ** Get MISO

    AND       rTmp1,rTmp1,1      // 18 mask of any other bits
    LSL       rSI,rSI,1          // 19 shift running input up
    OR        rSI,rSI,rTmp1      // 20 add the new bit to si 
    OR        rTmp2,rTmp2,rTmp2  // 21 nop
    AND       r30, r30,SCLK_L    // 22 ** SCLK LOW

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
shiftbits2x:

    MOV       rSO, rArg0         // setup output word
    MOV       rSI, 0             // setup input word

clockbit2x:
    LSR       rTmp1,rSO,28       // 02 get bit  (31->3)
    AND       rTmp1,rTmp1,8      // 03 mask bit

    LSR       rTmp2,rSO,25       // 04 get bit  (30->5)
    AND       rTmp2,rTmp2,32     // 05 mask bit 

    OR        r30,rTmp1,rTmp2    // 06 ** Set MOSI(0..1)
    LSL       rSO,rSO,2          // 07 shift SO to prep next out bits
    SUB       rBc,rBc,2          // 08 dec the bit count
    LSL       rSI,rSI,2          // 09 shift running input up
    OR        rTmp2,rTmp2,rTmp2  // 10 nop
    OR        rTmp2,rTmp2,rTmp2  // XX nop
    OR        r30, r30,SCLK_H    // 11 ** SCLK high

    OR        rTmp2,rTmp2,rTmp2  // XX nop
    OR        rTmp2,rTmp2,rTmp2  // 12 nop
    OR        rTmp2,rTmp2,rTmp2  // 13 nop
    OR        rTmp2,rTmp2,rTmp2  // 14 nop
    OR        rTmp2,rTmp2,rTmp2  // 15 nop

    LSR       rTmp1,r31,0        // 16 ** Get MISO(0) (1 -> 1)
    LSR       rTmp2,r31,2        // 17 ** Get MISO(1) (2 -> 0)

    AND       rTmp1,rTmp1,2      // 18 mask of any other bits
    AND       rTmp2,rTmp2,1      // 19 mask of any other bits

    OR        rSI,rSI,rTmp1      // 20 add the new bit to si 
    OR        rSI,rSI,rTmp2      // 21 add the new bit to si 

    AND       r30, r30,SCLK_L    // 22 ** SCLK LOW

    QBNE      clockbit2x,rBc,0   // 01 if more bits, goto top of loop

    MOV       rArg0,rSI          // move serial input to return
    RET

