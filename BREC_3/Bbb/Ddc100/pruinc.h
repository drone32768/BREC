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

// Sram offsets for pru.  This is from perspective of pru code
#define PRU1_LOFF_DRAM_PBASE    0x1000  // 4 bytes
#define PRU1_LOFF_DRAM_OFF      0x1004  // 4 bytes
#define PRU1_LOFF_DBG1          0x1008  // 4 bytes
#define PRU1_LOFF_DBG2          0x100C  // 4 bytes
#define PRU1_LOFF_DBG3          0x1010  // 4 bytes
#define PRU1_LOFF_CMD           0x1014  // 4 bytes
#define PRU1_LOFF_RES           0x1018  // 4 bytes

// pru1 commands
#define PRU1_CMD_NONE          0
#define PRU1_CMD_2KWORDS       1

// spi command definitions
#define SPI_CMD_RD_FIFO_STATUS 0xBD00   // 0x8000 | 0x3d00
#define SPI_CMD_NOP            0x0000

// spi read fifo (actual fifo R63)
#define SPI_CMD_RD_FIFO_DATA   0xBF00   // 0x8000 | 0x3f00

// spi read fifo (testing - counting reg R3)
// #define SPI_CMD_RD_FIFO_DATA   0x8300   // 0x8000 | 0x0300

// This is a function of the library and kernel module
// TODO: should be able to query this ...
#define PRU_MAX_SHORT_SAMPLES (128*1024)
