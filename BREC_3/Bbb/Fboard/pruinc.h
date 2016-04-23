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

//
// SPI Access commands for pru0 take the following form:
//
// +0 bytes,  16 bits: transfer code - spi operation
//            PRU0_CMD_16ARRAY2x - array of 16 bit xfers using 2x spi
//            PRU0_CMD_16ARRAY   - array of 16 bit xfers using 1x spi
//            PRU0_CMD_8STREAM   - array of 8 bit xfers using 1x spi
//
// +2 bytes, 16 bits: transfer count - number of items to transfer
// +4 bytes, 8 or 16 bits: data/command 0 - first 8 or 16 bit quantity
// +X bytes, 8 or 16 bits: data/command 1 - second 8 or 16 bit quantity
// ... Up to a limit of 512 bytes
// 
// There are two mailbox or command inputs.  One is directed for cpu
// use, while the other is intended to be pru1.
//
// SRAM_OFF_CMD1 = 0x00000(pru0)   +  0x0008 (SRAM_OFF_CMD1)
// SRAM_OFF_CMD2 = 0x10000(shared) +  0x0008 (SRAM_OFF_CMD2)
//

// sram offsets for cpu
#define SRAM_OFF_DBG1       0x00000  // 4 bytes
#define SRAM_OFF_DBG2       0x00004  // 4 bytes
#define SRAM_OFF_CMD1       0x00008  // 4+512 bytes
#define SRAM_OFF_CMD2       0x10008  // 4+512 bytes

// command codes in upper 16 bits of 32 bit int at SRAM_OFF_CMD1
#define PRU0_CMD_8STREAM    1       
#define PRU0_CMD_16ARRAY    2
#define PRU0_CMD_16ARRAY2x  3

// 
// ---------------------------------------------------------------------------
// PRU0             Memory Description                Notes
// ADDR
// ---------------------------------------------------------------------------
//        ********** PRU0 SRAM ****************
// 0x0000 |                                   | 
//        |                                   | 
// 0x0fff |                                   |
//        +-----------------------------------+
// 0x1000 |                                   |
//        |                                   |
// 0x1fff |                                   |
//        ********** PRU1 SRAM ****************
// 0x2000 |                                   |
//        |                                   |
// 0x2fff |                                   |
//        +-----------------------------------+
// 0x3000 |                                   |
//        |                                   |
// 0x3fff |                                   |
//        +-----------------------------------+
// 0x4000 |xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|
//        |xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|
//        |xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|
//        ********** SHARED SRAM **************
// 0x10000|                                   |
//        |          ....                     |
//        |                                   |
//        **********  END SRAM  ***************
//
//

