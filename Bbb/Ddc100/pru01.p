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

#include "PruConst.hp"
#include "Ddc100pru.h"


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

////////////////////////////////////////////////////////////////////////////////
#define    rDbg1Ptr         r4
    MOV    rDbg2Ptr,            (0x2000 + SRAM_OFF_DBG1)

#define    rDbg2Ptr         r5
    MOV    rDbg2Ptr,            (0x2000 + SRAM_OFF_DBG2)

#define    rTmp1            r6
    MOV    rTmp1,               0x0

#define    rTmp2            r7
    MOV    rTmp2,               0x0

#define    rCmdPtr          r8
    MOV    rCmdPtr,             (0x2000 + SRAM_OFF_CMD)

#define    rResPtr          r9
    MOV    rResPtr,             (0x2000 + SRAM_OFF_RES)

#define    rCmdCode         r10
    MOV    rCmdCode,            0x0

#define    rResCode         r11
    MOV    rResCode,            0x0

////////////////////////////////////////////////////////////////////////////////
main_loop:
    // increment dbg2 every loop pass
    LD32      rTmp1, rDbg2Ptr
    ADD       rTmp1,rTmp1,1
    ST32      rTmp1, rDbg2Ptr

    // update last command status
    ST16      rResCode, rResPtr            // store the status code

    // load and dispatch command
    LD16      rCmdCode, rCmdPtr            // load the command code
    QBEQ      xfer2k,rCmdCode,PRU1_CMD_2KWORDS
    JMP       main_loop

xfer2k:
    // check for 2k avail (ok=0 in rResCode)
    // if not avail set  goto main_loop
    MOV       rResCode, 0  // temporary
    QBNE      main_loop,rResPtr,0
     
read2k:
    CALL      read256
    CALL      save256
    JMP       main_loop

////////////////////////////////////////////////////////////////////////////////
save256:
    RET

////////////////////////////////////////////////////////////////////////////////
read256:

    MOV       rTmp1,0x9001
    MOV       rTmp2,rMsgPtr
read256_copyin:
    ST16      rTmp1,rTmp2       // store word
    ADD       rTmp2,rTmp2,2     // inc msg ptr
    SUB       rCnt,rCnt,1       // dec count
    QBNE      read256_copyin,rCnt,0     // loop until done

    MOV       rTmp1,0xff
    MOV       rTmp2,rMsgPtr
    ST16      rTmp1,rTmp2       // store xfer count in msg

waitdone_copyin:
    LD16      rTmp1,rTmp2

    RET
