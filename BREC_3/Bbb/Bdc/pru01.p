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
///
.setcallreg r29.w0

.origin 0
.entrypoint MAIN1

#include "pruconst.hp"
#include "pruinc.h"


//   
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

////////////////////////////////////////////////////////////////////////////////
#define    rDbg1Ptr         r4
    MOV    rDbg1Ptr,            (SRAM_OFF_DBG1)

#define    rDbg2Ptr         r5
    MOV    rDbg2Ptr,            (SRAM_OFF_DBG2)

#define    rTmp1            r6
    MOV    rTmp1,               0x0

#define    rTmp2            r7
    MOV    rTmp2,               0x0

#define    rCmdPtr          r8
    MOV    rCmdPtr,             (SRAM_OFF_CMD)

#define    rResPtr          r9
    MOV    rResPtr,             (SRAM_OFF_RES)

#define    rCmdCode         r10
    MOV    rCmdCode,            0x0

#define    rResCode         r11
    MOV    rResCode,            0x0

#define    rCnt             r12
    MOV    rCnt,                0x0

#define    rPru0CmdPtr      r13
    MOV    rPru0CmdPtr,         (0x2000+8) // PRU0 SRAM + 8

#define    rDramPtr         r15
    MOV    rDramPtr,            0x0

#define    rDrmBasePtr      r16
    MOV       rDrmBasePtr,      (SRAM_OFF_DRAM_PBASE)
    LD32      rDrmBasePtr, rDrmBasePtr


#define    rDrmOffset       r17
    MOV    rDrmOffset,          0x0

#define    rDrmOffsetMask   r18
    MOV    rDrmOffsetMask,      0x0003ffff

#define    rDrmOffsetPtr    r9
    MOV    rDrmOffsetPtr,       (SRAM_OFF_DRAM_OFF)


#define PRU0_CMD_16ARRAY 2 // fixme

////////////////////////////////////////////////////////////////////////////////
main_loop:
    // increment dbg2 every loop pass
    LD32      rTmp1, rDbg1Ptr
    ADD       rTmp1,rTmp1,1
    ST32      rTmp1, rDbg1Ptr

    // update last command status
    ST16      rResCode, rResPtr            // store the status code

    // load and dispatch command
    LD16      rCmdCode, rCmdPtr            // load the command code
    QBEQ      xfer2k,rCmdCode,PRU1_CMD_2KWORDS
    JMP       main_loop

xfer2k:
    // check for 2k avail (ok=0 in rResCode)
    // if not avail set  goto main_loop
    MOV       rResCode, 0  // TODO : temporary
    QBNE      main_loop,rResCode,0
     
read2k:
    // TODO create loop to xfer 2048 using 8x256
    CALL      read256
    CALL      save256

    // save dram offset to sram so cpu can see where pru is next
    ST32      rDrmOffset, rDrmOffsetPtr  
    JMP       main_loop

////////////////////////////////////////////////////////////////////////////////
save256:
    ADD       rDramPtr, rDrmBasePtr, rDrmOffset

    MOV       rTmp2,rPru0CmdPtr      // get pru0 cmd address
    ADD       rTmp2,rTmp2,4          // pru0 payload starts at +4 from cmd
    MOV       rCnt,256               // setup 256 count
save256_copyout:
    LD16      rTmp1,rTmp2            // load  data word
    ST16      rTmp1,rDramPtr         // store data word

    ADD       rDramPtr,rDramPtr,2    // inc dst ptr
    ADD       rTmp2,rTmp2,2          // inc src ptr
    SUB       rCnt,rCnt,1            // dec count
    QBNE      save256_copyout,rCnt,0 // loop until done

    MOV       rTmp1, 256
    ADD       rDrmOffset,rDrmOffset,rTmp1
    AND       rDrmOffset,rDrmOffset,rDrmOffsetMask

    RET

////////////////////////////////////////////////////////////////////////////////
read256:
    MOV       rTmp2,rPru0CmdPtr // get pru0 cmd address
    ADD       rTmp2,rTmp2,4     // pru0 payload starts at +4 from cmd
    MOV       rTmp1,0x8300      // spi word to execute rd=0x8000, R3=0x0300
    MOV       rCnt,256
read256_copyin:
    ST16      rTmp1,rTmp2       // store word
    ADD       rTmp2,rTmp2,2     // inc msg ptr
    SUB       rCnt,rCnt,1       // dec count
    QBNE      read256_copyin,rCnt,0     // loop until done

    // Store word count to pru0
    MOV       rTmp1,256
    MOV       rTmp2,rPru0CmdPtr // get pru0 cmd address
    ADD       rTmp2,rTmp2,2     // pru0 payload size starts at +2 from cmd
    ST16      rTmp1,rPru0CmdPtr // store xfer count

    // Store command to pru0
    MOV       rTmp2,rPru0CmdPtr // get pru0 cmd address
    MOV       rTmp1,PRU0_CMD_16ARRAY
    ST16      rTmp1,rPru0CmdPtr // store xfer command

waitdone_pru0:
    LD16      rTmp1,rPru0CmdPtr
    QBNE      waitdone_pru0,rTmp1,0  // loop until done

    RET
