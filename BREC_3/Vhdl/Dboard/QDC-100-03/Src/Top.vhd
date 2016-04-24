--
-- This source code is available under the "Simplified BSD license".
--
-- Copyright (c) 2016, J. Kleiner
-- All rights reserved.
--
-- Redistribution and use in source and binary forms, with or without 
-- modification, are permitted provided that the following conditions are met:
--
-- 1. Redistributions of source code must retain the above copyright notice, 
--    this list of conditions and the following disclaimer.
--
-- 2. Redistributions in binary form must reproduce the above copyright 
--    notice, this list of conditions and the following disclaimer in the 
--    documentation and/or other materials provided with the distribution.
--
-- 3. Neither the name of the original author nor the names of its contributors 
--    may be used to endorse or promote products derived from this software 
--    without specific prior written permission.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
-- "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
-- LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
-- A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
-- HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
-- SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
-- TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
-- OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
-- OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
-- (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
-- OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--
--

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

-- For Spartan6 
Library UNISIM;
use UNISIM.vcomponents.all;

entity Top is
   Port ( 
           DCLK         : in  STD_LOGIC;
           LEDS         : out STD_LOGIC_VECTOR( 3 downto 0 );
           GPIO0        : inout STD_LOGIC_VECTOR( 5 downto 0 );
           GPIO1        : inout STD_LOGIC_VECTOR( 5 downto 0 );
           HSPI_SS      : in  STD_LOGIC;
           HSPI_MOSI    : in  STD_LOGIC_VECTOR( 1 downto 0 );
           HSPI_MISO    : out STD_LOGIC_VECTOR( 1 downto 0 );
           HSPI_SCLK    : in  STD_LOGIC;
           SCLK         : in  STD_LOGIC;
           ADCI         : in  STD_LOGIC_VECTOR( 11 downto 0 );
           ADCQ         : in  STD_LOGIC_VECTOR( 11 downto 0 )
   );
end Top;

architecture structure of Top is

component ControlBlock is
    Port ( 
           iMOSI        : in    STD_LOGIC_VECTOR( 1 downto 0);
           oMISO        : out   STD_LOGIC_VECTOR( 1 downto 0);
           iSCLK        : in    STD_LOGIC;
           iSS          : in    STD_LOGIC;

           oLEDS        : out   STD_LOGIC_VECTOR( 3 downto 0 );
           ioGPIO0      : inout STD_LOGIC_VECTOR( 5 downto 0 );
           ioGPIO1      : inout STD_LOGIC_VECTOR( 5 downto 0 );
           
           iClk         : in    STD_LOGIC;
           iAltClk      : in    STD_LOGIC;
           
           oFifoRst     : out   STD_LOGIC;
           iFifoRdData  : in    STD_LOGIC_VECTOR( 15 downto 0 );
           oFifoRdEn    : out   STD_LOGIC;
           iFifoThresh  : in    STD_LOGIC;
           oFifoWrInhib : out   STD_LOGIC;

           oSrcSel      : out   STD_LOGIC_VECTOR ( 3 downto 0);           
           oGenTp       : out   STD_LOGIC_VECTOR ( 3 downto 0 );
           oPinc        : out   STD_LOGIC_VECTOR ( 15 downto 0 );
           iStatus      : in    STD_LOGIC_VECTOR ( 15 downto 0 );
           
           oIoff        : out  STD_LOGIC_VECTOR( 7  downto 0 );
           oInum        : out  STD_LOGIC_VECTOR( 7  downto 0 );
           oQoff        : out  STD_LOGIC_VECTOR( 7  downto 0 );
           oQnum        : out  STD_LOGIC_VECTOR( 7  downto 0 )
    );
end component;

component SignalBlock is
    Port ( 
           iSampClk     : in  STD_LOGIC; 
           iAdcI        : in  STD_LOGIC_VECTOR (11 downto 0);
           iAdcQ        : in  STD_LOGIC_VECTOR (11 downto 0);
           
           iFifoRst     : in  STD_LOGIC;
           oFifoRdData  : out STD_LOGIC_VECTOR ( 15 downto 0 );
           iFifoRdClk   : in  STD_LOGIC;
           iFifoRdEn    : in  STD_LOGIC;
           iFifoWrInhib : in  STD_LOGIC;
           oFifoThresh  : out STD_LOGIC;
           
           iSrcSel      : in  STD_LOGIC_VECTOR ( 3 downto 0);
           iGenTp       : in  STD_LOGIC_VECTOR ( 3 downto 0 ); 
           iPinc        : in  STD_LOGIC_VECTOR ( 15 downto 0 );
           oStatus      : out STD_LOGIC_VECTOR ( 15 downto 0 );
           
           iIoff        : in  STD_LOGIC_VECTOR( 7  downto 0 );
           iInum        : in  STD_LOGIC_VECTOR( 7  downto 0 );
           iQoff        : in  STD_LOGIC_VECTOR( 7  downto 0 );
           iQnum        : in  STD_LOGIC_VECTOR( 7  downto 0 )
    );
end component;

-- Derived clock for digital control (from DCM)
signal topClk          : STD_LOGIC; 

-- ADC sample clock
signal topSampClk      : STD_LOGIC; 

-- Selects what gets written into fifo
signal topSrcSel      : STD_LOGIC_VECTOR (3 downto 0); 

-- Fifo  signals
signal topFifoRst      : STD_LOGIC;
signal topFifoRdData   : STD_LOGIC_VECTOR (15 downto 0);
signal topFifoRdEn     : STD_LOGIC;
signal topFifoWrInhib  : STD_LOGIC;
signal topFifoThresh   : STD_LOGIC;

-- Summary signal block status
signal topSigStat      : STD_LOGIC_VECTOR (15 downto 0);

-- Test pattern generation control
signal topGenTp        : STD_LOGIC_VECTOR (3  downto 0);

-- Phase increment (frequency control)
signal topPinc         : STD_LOGIC_VECTOR (15 downto 0);

-- ADC I/Q data
signal topAdcI         : STD_LOGIC_VECTOR (11 downto 0);
signal topAdcQ         : STD_LOGIC_VECTOR (11 downto 0);

-- Channel match signals
signal topIoff        : STD_LOGIC_VECTOR (7  downto 0);
signal topInum        : STD_LOGIC_VECTOR (7  downto 0);
signal topQoff        : STD_LOGIC_VECTOR (7  downto 0);
signal topQnum        : STD_LOGIC_VECTOR (7  downto 0);

begin
     topSampClk <= SCLK;
     topAdcI    <= ADCI;
     topAdcQ    <= ADCQ;
                
     U01 : ControlBlock
          Port  Map ( 
           iMOSI       => HSPI_MOSI,
           oMISO       => HSPI_MISO, 
           iSCLK       => HSPI_SCLK,
           iSS         => HSPI_SS, 

           oLEDS       => LEDS,
           ioGPIO0     => GPIO0,
           ioGPIO1     => GPIO1,
           
           iClk        => topClk,
           iAltClk     => topClk,
           
           oFifoRst    => topFifoRst,
           iFifoRdData => topFifoRdData, 
           oFifoRdEn   => topFifoRdEn,  
           iFifoThresh => topFifoThresh,
           oFifoWrInhib=> topFifoWrInhib,

           oSrcSel     => topSrcSel,           
           oGenTp      => topGenTp,
           oPinc       => topPinc,
           iStatus     => topSigStat,

           oIoff       => topIoff,
           oInum       => topInum,
           oQoff       => topQoff,
           oQnum       => topQnum           

      );
      
      U02 : SignalBlock
          Port Map ( 
           iSampClk    => topSampClk, -- For testing w/o board topClk,
           iAdcI       => topAdcI,
           iAdcQ       => topAdcQ,
           
           iFifoRst    => topFifoRst,
           oFifoRdData => topFifoRdData,
           iFifoRdClk  => topClk,
           iFifoRdEn   => topFifoRdEn,   
           iFifoWrInhib=> topFifoWrInhib,
           oFifoThresh => topFifoThresh,
           
           iSrcSel     => topSrcSel,
           iGenTp      => topGenTp,
           iPinc       => topPinc,
           oStatus     => topSigStat,
           
           iIoff       => topIoff,
           iInum       => topInum,
           iQoff       => topQoff,
           iQnum       => topQnum            
      );
   
      ----------------------------------------------------------------------      
      -- The following is from the Spartan6 primitive documentation
      -- DCM_CLKGEN: Frequency Aligned Digital Clock Manager
      -- Spartan-6
      -- Xilinx HDL Libraries Guide, version 13.1
      -- Spartan-6 Libraries Guide for HDL Designs
      -- UG615 (v 13.1) March 1, 2011 www.xilinx.com 83
      -- Chapter 4: About Design Elements
--      DCM_CLKGEN_inst : DCM_CLKGEN
--         generic map (
--            CLKFXDV_DIVIDE  => 2,      -- CLKFXDV divide value (2, 4, 8, 16, 32)
--            CLKFX_DIVIDE    => 1,      -- Divide value - D - (1-256)
--            CLKFX_MD_MAX    => 0.0,    -- Specify maximum M/D ratio for timing anlysis
--            CLKFX_MULTIPLY  => 8,      -- Multiply value - M - (2-256)
--            CLKIN_PERIOD    => 0.0,    -- Input clock period specified in nS
--            SPREAD_SPECTRUM => "NONE", -- Spread Spectrum mode "NONE", "CENTER_LOW_SPREAD", "CENTER_HIGH_SPREAD",
--                                       -- "VIDEO_LINK_M0", "VIDEO_LINK_M1" or "VIDEO_LINK_M2"
--            STARTUP_WAIT    => FALSE   -- Delay config DONE until DCM_CLKGEN LOCKED (TRUE/FALSE)
--         )
--         
--         port map (
--            CLKFX     => topClk,  -- 1-bit output: Generated clock output
--            CLKFX180  => open,    -- CLKFX180, -- 1-bit output: Generated clock output 180 degree out of phase from CLKFX.
--            CLKFXDV   => open,    -- CLKFXDV, -- 1-bit output: Divided clock output
--            LOCKED    => open,    -- 1-bit output: Locked output
--            PROGDONE  => open,    -- PROGDONE, -- 1-bit output: Active high output to indicate the successful re-programming
--            STATUS    => open,    -- STATUS, -- 2-bit output: DCM_CLKGEN status
--            CLKIN     => DCLK,    -- 1-bit input: Input clock
--            FREEZEDCM => '0',     -- FREEZEDCM, -- 1-bit input: Prevents frequency adjustments to input clock
--            PROGCLK   => '0',     -- PROGCLK, -- 1-bit input: Clock input for M/D reconfiguration
--            PROGDATA  => '0',     -- PROGDATA, -- 1-bit input: Serial data input for M/D reconfiguration
--            PROGEN    => '0',     -- PROGEN, -- 1-bit input: Active high program enable
--            RST       => '0'      -- RST -- 1-bit input: Reset input pin
--         );
      -- End of DCM_CLKGEN_inst instantiation
      -- To understand the warnings generated see:
      -- http://forums.xilinx.com/t5/Spartan-Family-FPGAs/Reg-DCM-CLKGEN-primitive/td-p/136692
      

      ----------------------------------------------------------------------      
      -- The following is from the Spartan6 primitive documentation   
      -- Begin instantiation template
      -- Spartan-6
      -- Xilinx HDL Libraries Guide, version 13.1
      DCM_SP_inst : DCM_SP
      generic map (
         CLKDV_DIVIDE       => 2.0, -- CLKDV divide value
                                    -- (1.5,2,2.5,3,3.5,4,4.5,5,5.5,6,6.5,7,7.5,8,9,10,11,12,13,14,15,16).
         CLKFX_DIVIDE       => 1, -- Divide value on CLKFX outputs - D - (1-32)
         CLKFX_MULTIPLY     => 8, -- Multiply value on CLKFX outputs - M - (2-32)
         CLKIN_DIVIDE_BY_2  => FALSE, -- CLKIN divide by two (TRUE/FALSE)
         -- CLKIN_PERIOD       => 83.3333333333333333, -- Input clock period specified in nS
         CLKOUT_PHASE_SHIFT => "NONE", -- Output phase shift (NONE, FIXED, VARIABLE)
         CLK_FEEDBACK       => "1X", -- Feedback source (NONE, 1X, 2X)
         DESKEW_ADJUST      => "SYSTEM_SYNCHRONOUS", -- SYSTEM_SYNCHRNOUS or SOURCE_SYNCHRONOUS
         DFS_FREQUENCY_MODE => "LOW", -- Unsupported - Do not change value
         DLL_FREQUENCY_MODE => "LOW", -- Unsupported - Do not change value
         DSS_MODE           => "NONE", -- Unsupported - Do not change value
         DUTY_CYCLE_CORRECTION => TRUE, -- Unsupported - Do not change value
         FACTORY_JF         => X"c080", -- Unsupported - Do not change value
         PHASE_SHIFT        => 0, -- Amount of fixed phase shift (-255 to 255)
         STARTUP_WAIT       => FALSE -- Delay config DONE until DCM_SP LOCKED (TRUE/FALSE)
      )
      port map (
         CLK0     => open, -- 1-bit output: 0 degree clock output
         CLK180   => open, -- 1-bit output: 180 degree clock output
         CLK270   => open, -- 1-bit output: 270 degree clock output
         CLK2X    => open, -- 1-bit output: 2X clock frequency clock output
         CLK2X180 => open, -- 1-bit output: 2X clock frequency, 180 degree clock output
         CLK90    => open, -- 1-bit output: 90 degree clock output
         CLKDV    => open, -- 1-bit output: Divided clock output
         CLKFX    => topClk, -- 1-bit output: Digital Frequency Synthesizer output (DFS)
         CLKFX180 => open, -- 1-bit output: 180 degree CLKFX output
         LOCKED   => open, -- 1-bit output: DCM_SP Lock Output
         PSDONE   => open, -- 1-bit output: Phase shift done output
         STATUS   => open, -- 8-bit output: DCM_SP status output
         CLKFB    => DCLK, -- 1-bit input: Clock feedback input
         CLKIN    => DCLK, -- 1-bit input: Clock input
         DSSEN    => '0', -- 1-bit input: Unsupported, specify to GND.
         PSCLK    => '0', -- 1-bit input: Phase shift clock input
         PSEN     => '0', -- 1-bit input: Phase shift enable
         PSINCDEC => '0', -- 1-bit input: Phase shift increment/decrement input
         RST      => '0' -- 1-bit input: Active high reset input
      );

      -- End instantiation template
      
end structure;

