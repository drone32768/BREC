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

// Need the F board pru0 to get offsets for its command and command values
#include "../Fboard/pruinc.h"

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
MAIN1:
    // Enable OCP master port
    LBCO      r0, CONST_PRUCFG, 4, 4
    CLR       r0, r0, 4  // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    SBCO      r0, CONST_PRUCFG, 4, 4

////////////////////////////////////////////////////////////////////////////////
#define    rDbg1Ptr         r4
    MOV    rDbg1Ptr,            (PRU1_LOFF_DBG1)

#define    rDbg2Ptr         r5
    MOV    rDbg2Ptr,            (PRU1_LOFF_DBG2)

#define    rTmp1            r6
    MOV    rTmp1,               0x0

#define    rTmp2            r7
    MOV    rTmp2,               0x0

#define    rCmdPtr          r8
    MOV    rCmdPtr,             (PRU1_LOFF_CMD)

#define    rResPtr          r9
    MOV    rResPtr,             (PRU1_LOFF_RES)

#define    rCmdCode         r10
    MOV    rCmdCode,            0x0

#define    rResCode         r11
    MOV    rResCode,            0x0

#define    rCnt             r12
    MOV    rCnt,                0x0

#define    rP0CPcod         r13  
    MOV    rP0CPcod,            PRU0_LOFF_CMD2 // NOTE: from F board inc

#define    rRes01           r14 

#define    rRes02           r15 

#define    rArg0            r16
    MOV    rArg0,               0x0

#define    rDrmOffsetMask   r17
    MOV    rDrmOffsetMask,      (0x0003ffff)

#define    rDrmOffset       r18 
    MOV    rDrmOffset,           0x0

#define    rDrmBasePtr      r19
    MOV    rDrmBasePtr,          PRU1_LOFF_DRAM_PBASE
    LD32   rDrmBasePtr, rDrmBasePtr

#define    rDrmOffsetPtrPtr r20
    MOV    rDrmOffsetPtrPtr,     PRU1_LOFF_DRAM_OFF

#define    rDbg3Ptr         r21
    MOV    rDbg3Ptr,            (PRU1_LOFF_DBG3)

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
    QBEQ      xfer_block,rCmdCode,PRU1_CMD_2KWORDS

    // command not processed
    MOV       rResCode, rCmdCode, 
    JMP       main_loop

xfer_block:
    // increment dbg2 every xfer attempt
    LD32      rTmp1, rDbg2Ptr
    ADD       rTmp1,rTmp1,1
    ST32      rTmp1, rDbg2Ptr

    // check for 2k avail (ok=0 in rResCode)
    MOV       rArg0,rP0CPcod   
    CALL      fifo_below_2k
    QBNE      xfer_done,rArg0,0
     
readblock:
    MOV       rArg0,rP0CPcod    // 256
    CALL      read256
    MOV       rArg0,rP0CPcod   
    CALL      save256

    MOV       rArg0,rP0CPcod    // 512
    CALL      read256
    MOV       rArg0,rP0CPcod   
    CALL      save256

    MOV       rArg0,rP0CPcod    // 768
    CALL      read256
    MOV       rArg0,rP0CPcod   
    CALL      save256

    MOV       rArg0,rP0CPcod    // 1024 
    CALL      read256
    MOV       rArg0,rP0CPcod   
    CALL      save256

    MOV       rArg0,rP0CPcod    // 256
    CALL      read256
    MOV       rArg0,rP0CPcod   
    CALL      save256

    MOV       rArg0,rP0CPcod    // 512
    CALL      read256
    MOV       rArg0,rP0CPcod   
    CALL      save256

    MOV       rArg0,rP0CPcod    // 768
    CALL      read256
    MOV       rArg0,rP0CPcod   
    CALL      save256

    MOV       rArg0,rP0CPcod    // 1024 
    CALL      read256
    MOV       rArg0,rP0CPcod   
    CALL      save256

xfer_done:
    // Set res to cmd and return through main loop
    MOV       rResCode, rCmdCode, 
    JMP       main_loop

////////////////////////////////////////////////////////////////////////////////
// Saves 256 words from pru0 ram to dram
// ARG  : rArg0 = pointer to pru0 cmd
// LOCAL: rTmp1, rTmp2, rCnt
// LOCAL: rDrm*
// RET  : not applicable
save256:
    // Setup to copy 256 results
    MOV       rTmp2,rArg0         
    ADD       rTmp2,rTmp2,6          // skip over first result
    MOV       rCnt,256           

save256_copyout:

    // Load the sample from pru0 ram
    LD16      rTmp1,rTmp2            // store word
    ADD       rTmp2,rTmp2,2          // inc msg ptr
    SUB       rCnt,rCnt,1            // dec count

    // Store and advance ddr dst pointer
    SBBO      rTmp1,rDrmBasePtr,rDrmOffset, 2       // store samp in drm
    ADD       rDrmOffset,rDrmOffset,2               // inc drm dst addr 
    AND       rDrmOffset,rDrmOffset,rDrmOffsetMask  // wrap dst addr

    // Loop until done copying
    QBNE      save256_copyout,rCnt,0 // loop until done

    // Save dram head in sram so cpu can access
    ST32      rDrmOffset,rDrmOffsetPtrPtr   

    RET

////////////////////////////////////////////////////////////////////////////////
//
// Read 256 spi commands (to read fifo) with results left in pru0 ram
// ARG  : rArg0 = pointer to pru0 cmd
// LOCAL: rTmp1, rTmp2
// RET  : not applicable
//
read256:

    // Setup to fill in 256 command words (each word is a read command)
    MOV       rTmp1,rArg0         
    ADD       rTmp1,rTmp1, 4         // +4 first spi command
    MOV       rTmp2,SPI_CMD_RD_FIFO_DATA  
    MOV       rCnt,256           

    // Fill in the payload of read command words
read256_copyin:
    ST16      rTmp2,rTmp1            // store word
    ADD       rTmp1,rTmp1,2          // inc msg ptr
    SUB       rCnt,rCnt,1            // dec count
    QBNE      read256_copyin,rCnt,0  // loop until done

    // Add one last nop command to read last word
    MOV       rTmp2,SPI_CMD_NOP      // last spi word to execute 
    ST16      rTmp2,rTmp1            // store word

    // Fill in the command word count 
    MOV       rTmp1,rArg0        
    ADD       rTmp1,rTmp1, 2         // +2 spi command count
    MOV       rTmp2,257
    ST16      rTmp2,rTmp1            // store count

    // Issue the command to start
    MOV       rTmp2,PRU0_CMD_16ARRAY  // +0 pru0 command
    ST16      rTmp2,rArg0

    // Wait until the command word count is 0 (pru0 is done)
waitdone_pru0:
    LD16      rTmp2,rArg0
    QBNE      waitdone_pru0,rTmp2,0  // loop until done

    RET

////////////////////////////////////////////////////////////////////////////////
//
// Check the fifo status for at least 2k samples
// ARG  : rArg0 = pointer to pru0 cmd
// LOCAL: rTmp1, rTmp2, rCnt
// RET  : rArg0 = non 0 if fifo is below 2k, 0 else
//
fifo_below_2k:

    // Store an SPI command
    MOV       rTmp1,rArg0        
    ADD       rTmp1,rTmp1, 6    // +6 for second command
    MOV       rTmp2,SPI_CMD_RD_FIFO_STATUS        
    ST16      rTmp2,rTmp1   

    // Store an SPI command
    MOV       rTmp1,rArg0        
    ADD       rTmp1,rTmp1, 4    // +4 for first command
    MOV       rTmp2,SPI_CMD_RD_FIFO_STATUS        
    ST16      rTmp2,rTmp1   

    // Store the PRU0 transfer count
    MOV       rTmp1,rArg0        
    ADD       rTmp1,rTmp1, 2    // +2 count of spi commands 
    MOV       rTmp2, 2        
    ST16      rTmp2,rTmp1   

    // Store the PRU0 operation/command
    MOV       rTmp1,rArg0       // +0 for command  
    MOV       rTmp2,PRU0_CMD_16ARRAY        
    ST16      rTmp2,rTmp1   

check2kfifo_waita:
    MOV       rTmp1,rArg0        
    LD16      rTmp2,rTmp1
    QBNE      check2kfifo_waita,rTmp2,0  // loop until done

    // Get the final result 
    MOV       rTmp1,rArg0        
    ADD       rTmp1,rTmp1, 6     // Get results of 2nd command at +6 
    LD16      rTmp2,rTmp1
   
    // Save result in arg0 and return
    MOV       rArg0,rTmp2
    RET
