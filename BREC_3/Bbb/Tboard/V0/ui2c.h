//
//
// This source code is available under the "Simplified BSD license".
//
// Copyright (c) 2015, J. Kleiner
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
/**
This class implements a user space gpio based I2C interface.  It must
be provided a gpio for scl and sda and then allows bus primitive operations
by slave device drivers.
*/
class UI2C {

public:

    /** Error flags */
#   define UI2C_ERR_CFG          0x00000001
#   define UI2C_ERR_START_CONDA  0x00000002
#   define UI2C_ERR_START_CONDB  0x00000004
#   define UI2C_ERR_STOP_COND    0x00000008
#   define UI2C_ERR_WRITE_ACK    0x00000010
#   define UI2C_ERR_READ_ACK     0x00000020
#   define UI2C_ERR_HIGH_TOUT    0x00000040

#   define UI2C_ERR_WRITE_A      0x00001000
#   define UI2C_ERR_WRITE_B      0x00002000
#   define UI2C_ERR_WRITE_C      0x00004000

    UI2C();

    uint32_t Dbg( uint32_t dbg );
    uint32_t configure(  GpioPin *scl, GpioPin *sda );
    uint32_t start_cond();
    uint32_t stop_cond();
    uint32_t write_cycle( uint8_t byte );
    uint32_t read_cycle(  uint8_t *bytePtr, int nack );

    /** Debug flags */
#   define UI2C_DBG_CFG          0x00000001
#   define UI2C_DBG_START        0x00000002
#   define UI2C_DBG_WRITE_CYCLE  0x00000004
#   define UI2C_DBG_READ_CYCLE   0x00000008

private:
    int       mDbg;      // Debug flags (or in items of interest)
    int       mUsHold;   // Microseconds to hold values
    GpioPin  *mGpioSCL;  // User space SCL gpio object (see configure)
    GpioPin  *mGpioSDA;  // User space SDA gpio object (see configure)

    uint32_t  wait_high( GpioPin  *pin );
    uint32_t  read_value( GpioPin  *pin );
    uint32_t  pull_low( GpioPin  *pin );

};

