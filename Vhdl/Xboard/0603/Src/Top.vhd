--
-- This source code is available under the "Simplified BSD license".
--
-- Copyright (c) 2015, J. Kleiner
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

-- For Spartan6 
Library UNISIM;
use UNISIM.vcomponents.all;

-- This is the top most block of the FPGA.  See the readme.txt
-- for a full description.
entity Top is
    Port ( 
           SPI_MOSI : in  STD_LOGIC;
           SPI_MISO : out STD_LOGIC;
           SPI_SCLK : in  STD_LOGIC;
           SPI_SS   : in  STD_LOGIC;
			  OUT_CLK  : out STD_LOGIC;
			  IN_CLK   : in  STD_LOGIC;
			  EN       : out STD_LOGIC;
			  LEDS     : out STD_LOGIC_VECTOR( 3 downto 0 );
			  
			  ADC_CLK  : in  STD_LOGIC;
			  nADCOE   : out STD_LOGIC;
			  ADC      : in  STD_LOGIC_VECTOR( 11 downto 0 )
	 );
end Top;

architecture structure of Top is

component ControlBlock is
    Port ( 
           iMOSI        : in  STD_LOGIC;
           oMISO        : out STD_LOGIC;
           iSCLK        : in  STD_LOGIC;
           iSS          : in  STD_LOGIC;

			  oLEDS        : out STD_LOGIC_VECTOR( 3 downto 0 );
			  
			  iClk         : in  STD_LOGIC;
			  iAltClk      : in  STD_LOGIC;
			  
			  oFifoRst     : out STD_LOGIC;
			  oFifoSel     : out STD_LOGIC_VECTOR ( 3 downto 0);
			  iFifoRdData  : in  STD_LOGIC_VECTOR( 15 downto 0 );
			  oFifoRdEn    : out STD_LOGIC;
			  iFifoMark    : in  STD_LOGIC;
			  oFifoWrGate  : out STD_LOGIC;
			  
			  oGenTp       : out STD_LOGIC_VECTOR ( 3 downto 0 );
			  oPinc        : out STD_LOGIC_VECTOR ( 15 downto 0 );
			  iStatus      : in  STD_LOGIC_VECTOR ( 15 downto 0 )
	 );
end component;

component SignalBlock is
    Port ( 
			  iClk         : in  STD_LOGIC;
			  iAdcDat      : in  STD_LOGIC_VECTOR (11 downto 0);
	  
			  iFifoSel     : in  STD_LOGIC_VECTOR ( 3 downto 0);
			  oFifoData    : out STD_LOGIC_VECTOR ( 15 downto 0 );
			  oFifoWrEn    : out STD_LOGIC;
			  iFifoWrGate  : in  STD_LOGIC;
			  
			  iGenTp       : in  STD_LOGIC_VECTOR ( 3 downto 0 ); 
			  iPinc        : in  STD_LOGIC_VECTOR ( 15 downto 0 );
			  oStatus      : out STD_LOGIC_VECTOR ( 15 downto 0 )
	 );
end component;


-- The following is from the generated core instantiation template
COMPONENT AdcFifo
  PORT (
    rst    : IN STD_LOGIC;
    wr_clk : IN STD_LOGIC;
    rd_clk : IN STD_LOGIC;
    din    : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
    wr_en  : IN STD_LOGIC;
    rd_en  : IN STD_LOGIC;
    dout   : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    full   : OUT STD_LOGIC;
    empty  : OUT STD_LOGIC;
	 almost_empty : OUT STD_LOGIC;
	 prog_full : OUT STD_LOGIC
  );
END COMPONENT;

-- Derived clock for digital control (from DCM)
signal topClk          : STD_LOGIC; 

-- Selects what gets written into fifo
signal topFifoSel      : STD_LOGIC_VECTOR (3 downto 0); 

-- Fifo read signals
signal topFifoRdData   : STD_LOGIC_VECTOR (15 downto 0);
signal topFifoRdEn     : STD_LOGIC;

-- Fifo write signals
signal topFifoWrData   : STD_LOGIC_VECTOR (15 downto 0);
signal topFifoWrEn     : STD_LOGIC;	
signal topFifoWrGate   : STD_LOGIC;

-- Fifo control and status 
signal topFifoRst      : STD_LOGIC;
signal topInputEn      : STD_LOGIC;
signal topFifoFull     : STD_LOGIC;
signal topFifoEmpty    : STD_LOGIC;
signal topFifoAe       : STD_LOGIC;
signal topFifoHe       : STD_LOGIC;

-- Test pattern generation control
signal topGenTp        : STD_LOGIC_VECTOR (3  downto 0);

-- Phase increment (frequency control)
signal topPinc         : STD_LOGIC_VECTOR (15 downto 0);

-- Summary status
signal topStatus       : STD_LOGIC_VECTOR (15 downto 0);

-- Summary physical indicators
signal topLeds         : STD_LOGIC_VECTOR (3  downto 0);
	
-- ADC/signal processing clock
signal topSampClk      : STD_LOGIC;

begin
     -- Temporarily assign as const 
	  OUT_CLK          <= '0'; -- ADC_CLK;
	  nADCOE           <= '0';
	  EN               <= '1';
	  
	  -- ADC samples available on falling edge
	  -- but fifos and other cores work on rising edge
	  -- so invert the input and use as signal clock
	  topSampClk       <= not ADC_CLK;
	  
	  -- LEDs to normal register control
	  -- LEDS             <= topLeds;
	  LEDS <= topGenTp;

     U01 : ControlBlock
		    Port  Map ( 
           iMOSI      => SPI_MOSI,
           oMISO      => SPI_MISO, 
           iSCLK      => SPI_SCLK,
           iSS        => SPI_SS,

			  oLEDS      => topLeds,
			  
			  iClk       => topClk,
			  iAltClk    => topSampClk,
			  
			  oFifoRst   => topFifoRst,
			  oFifoSel   => topFifoSel,
			  iFifoRdData=> topFifoRdData,
			  oFifoRdEn  => topFifoRdEn,
			  iFifoMark  => topFifoHe,
			  oFifoWrGate=> topFifoWrGate,
			  
			  oGenTp     => topGenTp,
			  oPinc      => topPinc,
			  iStatus    => topStatus

	   );
		
		U02 : SignalBlock
		    Port Map ( 
			  iClk        => topSampClk,
           iAdcDat     => ADC,
			  iFifoSel    => topFifoSel,
			  oFifoData   => topFifoWrData,
			  oFifoWrEn   => topFifoWrEn,
			  iFifoWrGate => topFifoWrGate,
			  
			  iGenTp      => topGenTp,
			  iPinc       => topPinc,
			  oStatus     => topStatus
		);
		
      ----------------------------------------------------------------------				
		-- The following is from the generated core instantiation template
		U03 : AdcFifo
		  PORT MAP (
		    rst          => topFifoRst,
			 wr_clk       => topSampClk,
			 rd_clk       => topClk,
			 din          => topFifoWrData,
			 wr_en        => topFifoWrEn,
			 rd_en        => topFifoRdEn,
			 dout         => topFifoRdData,
			 full         => topFifoFull,
			 empty        => topFifoEmpty,
			 almost_empty => topFifoAe,
			 prog_full    => topFifoHe
		  );
		-- You must compile the wrapper file AdcFifo.vhd when simulating
		-- the core, AdcFifo. When compiling the wrapper file, be sure to
		-- reference the XilinxCoreLib VHDL simulation library. For detailed
		-- instructions, please refer to the "CORE Generator Help".


      ----------------------------------------------------------------------		
		-- The following is from the Spartan6 primitive documentation
		-- DCM_CLKGEN: Frequency Aligned Digital Clock Manager
		-- Spartan-6
		-- Xilinx HDL Libraries Guide, version 13.1
		-- Spartan-6 Libraries Guide for HDL Designs
		-- UG615 (v 13.1) March 1, 2011 www.xilinx.com 83
		-- Chapter 4: About Design Elements
		DCM_CLKGEN_inst : DCM_CLKGEN
			generic map (
				CLKFXDV_DIVIDE  => 2, -- CLKFXDV divide value (2, 4, 8, 16, 32)
				CLKFX_DIVIDE    => 1, -- Divide value - D - (1-256)
				CLKFX_MD_MAX    => 0.0, -- Specify maximum M/D ratio for timing anlysis
				CLKFX_MULTIPLY  => 4, -- Multiply value - M - (2-256)
				CLKIN_PERIOD    => 0.0, -- Input clock period specified in nS
				SPREAD_SPECTRUM => "NONE", -- Spread Spectrum mode "NONE", "CENTER_LOW_SPREAD", "CENTER_HIGH_SPREAD",
				                           -- "VIDEO_LINK_M0", "VIDEO_LINK_M1" or "VIDEO_LINK_M2"
				STARTUP_WAIT    => FALSE -- Delay config DONE until DCM_CLKGEN LOCKED (TRUE/FALSE)
			)
			
			port map (
				CLKFX     => topClk,  -- 1-bit output: Generated clock output
				CLKFX180  => open,    -- CLKFX180, -- 1-bit output: Generated clock output 180 degree out of phase from CLKFX.
				CLKFXDV   => open,    -- CLKFXDV, -- 1-bit output: Divided clock output
				LOCKED    => open,    -- 1-bit output: Locked output
				PROGDONE  => open,    -- PROGDONE, -- 1-bit output: Active high output to indicate the successful re-programming
				STATUS    => open,    -- STATUS, -- 2-bit output: DCM_CLKGEN status
				CLKIN     => IN_CLK,  -- 1-bit input: Input clock
				FREEZEDCM => '0', -- FREEZEDCM, -- 1-bit input: Prevents frequency adjustments to input clock
				PROGCLK   => '0', -- PROGCLK, -- 1-bit input: Clock input for M/D reconfiguration
				PROGDATA  => '0', -- PROGDATA, -- 1-bit input: Serial data input for M/D reconfiguration
				PROGEN    => '0', -- PROGEN, -- 1-bit input: Active high program enable
				RST       => '0'  -- RST -- 1-bit input: Reset input pin
			);
		-- End of DCM_CLKGEN_inst instantiation
		-- To understand the warnings generated see:
		-- http://forums.xilinx.com/t5/Spartan-Family-FPGAs/Reg-DCM-CLKGEN-primitive/td-p/136692
		
	
end structure;

