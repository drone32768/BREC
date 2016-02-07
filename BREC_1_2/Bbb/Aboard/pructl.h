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
// The "data page" is the first page of sram.  This is used as a fifo from
// pru0 to pru1 for ADC samples.  The offset is:
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
// 0x0000  - Offset bytes into sram data page of where pru0 will write 
//           next sample
//           Used by pru00 to indicate to pru01 where it will write next
// 
// 0x0004  - Dram physical base pointer of samples
//           Used by pru01 as start of dram memory region to write to.
// 
// 0x0008  - CS low Spin count + 1
//           pru00 will spin one less than this number of times holding
//           cs low. Each spin is 2 instructions or 10nS. [ use 1 to get
//           0 iterations and 10nS check ]
// 
// 0x000C  - Reserved.
//           This location is not used and is set to 0xdeadbeef by pructl.
// 
// 0x0010  - Offset within sram data page that pru01 is reading.
//           This location is used to indicate to cpu where in sram pru01 is
//           reading from (and can be compared for debugging purposes) to 
//           where pru00 is writing to.
// 
// 0x0014  - Dram offset in bytes of last pru01 write within dram buffer
//           Used by pru01 to indicate to cpu were in physical memory it 
//           is writing
// 
// 0x0018  - Reserved.
//           This location is not used and is set to 0xbabedead by pructl.
// 
// 0x001C  - Reserved.
//           This location is not used and is set to 0xbeefcafe by pructl.
//
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

#define PRU0_OFFSET_SRAM_HEAD  0x1000
#define PRU0_OFFSET_DRAM_PBASE 0x1004
#define PRU0_OFFSET_SPIN_COUNT 0x1008
#define PRU0_OFFSET_RES1       0x100C
#define PRU0_OFFSET_SRAM_TAIL  0x1010
#define PRU0_OFFSET_DRAM_HEAD  0x1014
#define PRU0_OFFSET_RES2       0x1018

#define PRU_MAX_SHORT_SAMPLES (128*1024)
