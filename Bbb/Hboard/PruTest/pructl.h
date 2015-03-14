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

// 
// The "data page" is the first page of sram.  This is used as a fifo between
// pru0 and pru1 for ADC samples.  The offset is:
//      0x0000 - pru 0 [ pru0's version of pru0's sram is +0x0000 ]
//      0x2000 - pru 0 [ pru1's version of pru0's sram is +0x0000 ]
// 
// The "control page" is the second page sram.  The offset is:
//      0x1000 - pru 0 [ pru0's version of pru0's sram is +0x0000 ]
//      0x3000 - pru 1 [ pru1's version of pru0's sram is +0x2000 ]
// 
// The control page has several 32 bit (4 byte) paramters, pru-pru, and pru-cpu
// variables.  Those offset are the following:
// 
// 0x0000  - Offset bytes into sram data page of where pru writer will write 
//           next sample
// 
#define PRU0_OFFSET_SRAM_HEAD  0x1000

//
// 0x0004  - Dram physical base pointer of samples
//           Used by pru as start of dram memory region to write to.
// 
#define PRU0_OFFSET_DRAM_PBASE 0x1004  

//
// 0x0008  - Number of time to spin before attempting to collect next sample
// 
#define PRU0_OFFSET_SPIN_COUNT 0x1008

//
// 0x000C  - Reserved.
//           This location is not used and is set to 0xdeadbeef by pructl.
// 
#define PRU0_OFFSET_RES1       0x100C

//
// 0x0010  - Unused
// 
#define PRU0_OFFSET_SRAM_TAIL  0x1010  


// 0x0014  - Dram offset in bytes of last pru write within dram buffer
//           Used by pru to indicate to cpu were in physical memory it 
//           is writing
//
#define PRU0_OFFSET_DRAM_HEAD  0x1014  // H

//
// 
// 0x0018  - Reserved.
//           This location is not used and is set to 0xbabedead by pructl.
// 
#define PRU0_OFFSET_RES2       0x1018

//
// 0x001C  - Reserved.
//           This location is not used and is set to 0xbeefcafe by pructl.
//
#define PRU0_OFFSET_RES3       0x101C


// 
// ---------------------------------------------------------------------------
// PRU0             Memory Description                Notes
// ADDR
// ---------------------------------------------------------------------------
//        ********** PRU0 SRAM ****************
// 0x0000 |                                   |  <--  SRAM FIFO BASE = 0x0000
//        |         PRU0 -> PRU1 FIFO         |   |  
//   0fff |                                   |   +-> SRAM FIFO MASK = 0x0fff
//        +-----------------------------------+
// 0x1000 | PRU0_OFFSET_SRAM_HEAD             |
//   1004 | PRU0_OFFSET_DRAM_PBASE            |
//   1008 | PRU0_OFFSET_SPIN_COUNT            |
//        |          ....                     |
//   1fff |                                   |
//        ********** PRU1 SRAM ****************
// 0x2000 |                                   |
//        |          ....                     |
//   3fff |                                   |
//        ********** SHARED SRAM **************
// 0x4000 |                                   |
//        |          ....                     |
//        |                                   |
//        **********  END SRAM  ***************
//
//


// TODO this should not be treated as a constant but should be placed
// in sram based on cpu library
#define PRU_MAX_SHORT_SAMPLES (128*1024)
