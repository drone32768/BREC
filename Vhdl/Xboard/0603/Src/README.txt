----------------------------------------------------------------
Generating Cores
See the *.xco files for details of parameters used in generating
the cores.

FIFO core. It uses a non AXI interface, is 4kx16 deep with
a single threshold at 2k.

CFIR core. The coefficients used are specified in the 
cfir.coe file maintained with the VHDL source.  This includes
comments on the cic/cfir parameters used to generated the taps.
For this device, a 96 element 16 bit tap fir with 40 bit inputs
and 16 bit outputs requireds 2 DSP chains at length = 1.

CIC core. Higher order filters require too many DSPs to co-exist
with the DDS and CFIR.  As the order is increased and DSPs are
disallowed the map phase has problems (i.e. doesn't complete).
A N=4 cic without DSP use does complete (ver=0503).

DDS core.  The output is set to 12 bits, the phase argument to 16
and phase dither is enabled.

----------------------------------------------------------------
-- Programming interface.
-- This module implements an SPI based interface.  
-- During a write to the SPI a read value is concurrently
-- clocked out.  The read value clocked out is defined by
-- the previous write and the internal state.  
--
-- See PortController.vhd for the port control bits.  These
-- are the low 4 bits of the 16 bit word and are common
-- across all commands.  They define which port is accessed,
-- and whether the access is a read or write.
-- Command Word is:
-- 1111 1100 0000 0000
-- 5432 1098 7654 3210
--                RWXP 
--
-- R = 1 => read
-- W = 1 => write
-- P = 0(port0), 1(port1)
-- write = xxx4 = write xxx4 to p0
-- write = yyy5 = write yyy5 to p1
-- write = xxx8 = read  from    p0
-- write = yyy9 = read  from    p1
--
-- Port0 is always connected to the control register (r0).
-- Writes to this register control the behavior of the
-- device and define what reads/writes to port 1 are.  Reads
-- to port 0 always return the contents of register 0.
--
----------------------------------------------------------------
-- Register 0
-- 1111 1100 0000 0000
-- 5432 1098 7654 3210
-- ERXX LLLL SSSS CCCC 
--
-- E = EN
-- R = Fifo Reset
-- X = reserved
-- L = LEDs
-- S = P1 selector
--     0000 = reg 0 
--     0001 = reg 1
--     0010 = reg 2 
--     0011 = reg 3 
--     ... See following register definitions
--
-- C = SPI command nibble
--
----------------------------------------------------------------
-- Register 1
-- 1111 1100 0000 0000
-- 5432 1098 7654 3210
-- DDDD DDDD DDDD DDDD
--
-- D = Fifo Data. 15 = MSB
-- Format depends on data selected
-- ADC data is 12 bit unsigned
-- NCO data is 12 bit unsigned
-- Raw non decimated I or Q is 12 bit signed
-- CIC I or Q is 12 bit signed
-- IQ is 12 bit signed (in pairs)
--
----------------------------------------------------------------
-- Register 2
-- 1111 1100 0000 0000
-- 5432 1098 7654 3210
-- DDDD DDDD DDDD DDDD
--
-- D = Counter Data. 15 = MSB
-- Counter of number of reads to port 1.
--
----------------------------------------------------------------
-- Register 3
-- 1111 1100 0000 0000
-- 5432 1098 7654 3210
-- FFFF TTTT XXXX XXXX 
--
-- FFFF = Fifo Input Selector
--     0000 = adc data
--     0001 = nco sin
--     0010 = nco cos
--     0011 = I (raw, non decimated)
--     0100 = Q (raw, non decimated)
--     0101 = I post CIC
--     0110 = Q post CIC
-- TTTT = Test Pattern Control
--     0001 = I/Q counting phased patter
--     0010 = ADC 1/8'th Fs full scale square wave
--
----------------------------------------------------------------
-- Register 4
-- 1111 1100 0000 0000
-- 5432 1098 7654 3210
-- DDDD DDDD DDDD DDDD
--
-- D = Digital Clock Counter Data. 15 = MSB
-- Counter of digital input clock transitions
-- For diagnostic purposes
--
----------------------------------------------------------------
-- Register 5
-- 1111 1100 0000 0000
-- 5432 1098 7654 3210
-- DDDD DDDD DDDD DDDD
--
-- D = Sample Clock Counter Data. 15 = MSB
-- Counter of sample input clock transitions
-- For diagnostic purposes
--
----------------------------------------------------------------
-- Register 6
-- 1111 1100 0000 0000
-- 5432 1098 7654 3210
-- PPPP PPPP PPPP XXXX 
--
-- PPPP = Pinc low 12 bits
--
----------------------------------------------------------------
-- Register 7
-- 1111 1100 0000 0000
-- 5432 1098 7654 3210
-- XXXX XXXX PPPP XXXX 
--
-- PPPP = Pinc high 4 bits
--
----------------------------------------------------------------
-- Register 8
-- 1111 1100 0000 0000
-- 5432 1098 7654 3210
-- DDDD DDDD DDDD DDDD
--
-- D = Firmware version 15 = MSB
-- Coded as hex nibbles. e.g. 0504 = v05.04
--
----------------------------------------------------------------
-- Register 9
-- 1111 1100 0000 0000
-- 5432 1098 7654 3210
-- DDDD DDDD DDDD DDDD
--
-- D = Sample Clock Counter Data. 15 = MSB
-- Low frequency status data.
-- 150909 currently the count of pinc writes.
--

------------------------------------------------------------
-- Generating images.
-- Under "Generate Programming File"  
--    "Process properties" 
--      "Configuration Options" Fields:
--          "Configuration Pin Done" = PullUP
--          "JTAG Pin TCLK         " = Float
--          "Unused IOB pins       " = Float
--      "Startup Options" Fields:
--          "FPGA Startup Clock    " = ...
--             Directly Load = JTAG Clock
--             Serial EPROM  = CCLK
