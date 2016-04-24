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

-- For Spartan6 specific components (e.g. DCM instantiation)
Library UNISIM;
use UNISIM.vcomponents.all;

entity DEB is
    Port ( 
           iRst          : in  STD_LOGIC;
           iSampClk      : in  STD_LOGIC;
           iIQ_data      : in  STD_LOGIC_VECTOR (31 downto 0);
           iIQ_valid     : in  STD_LOGIC;
           oIQ_data      : out STD_LOGIC_VECTOR (31 downto 0);
           oIQ_valid     : out STD_LOGIC
         );
end DEB;

architecture Behavioral of DEB is

COMPONENT F16x32
  PORT (
          m_aclk         : IN  STD_LOGIC;
          s_aclk         : IN  STD_LOGIC;
          s_aresetn      : IN  STD_LOGIC;
          s_axis_tvalid  : IN  STD_LOGIC;
          s_axis_tready  : OUT STD_LOGIC;
          s_axis_tdata   : IN  STD_LOGIC_VECTOR(31 DOWNTO 0);
          m_axis_tvalid  : OUT STD_LOGIC;
          m_axis_tready  : IN  STD_LOGIC;
          m_axis_tdata   : OUT STD_LOGIC_VECTOR(31 DOWNTO 0)
  );
END COMPONENT;

COMPONENT Dec01
  PORT (
    aresetn              : IN  STD_LOGIC;
    aclk                 : IN  STD_LOGIC;
    s_axis_data_tvalid   : IN  STD_LOGIC;
    s_axis_data_tready   : OUT STD_LOGIC;
    s_axis_data_tdata    : IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    m_axis_data_tvalid   : OUT STD_LOGIC;
    m_axis_data_tdata    : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)
  );
END COMPONENT;

-- Filter clock is a multiple of sample clock for filter use
signal filClk          : STD_LOGIC;

-- Input fifo data out
signal FIN_valid       : STD_LOGIC;
signal FIN_data        : STD_LOGIC_VECTOR( 31 downto 0 );

-- I decimator output
signal Id0_v16_valid : STD_LOGIC;
signal Id0_v16_data  : STD_LOGIC_VECTOR( 15 downto 0 );

-- Q decimator output
signal Qd0_v16_valid : STD_LOGIC;
signal Qd0_v16_data  : STD_LOGIC_VECTOR( 15 downto 0 );

-- Combined IQ decimator output (for output fifo input)
signal d_v32_data  : STD_LOGIC_VECTOR( 31 downto 0 );

signal resetn        : STD_LOGIC;
begin
    resetn <= not iRst;
    
    FIN : F16x32
        PORT MAP (
          m_aclk              => filClk,
          s_aclk              => iSampClk,
          s_aresetn           => resetn,
          s_axis_tvalid       => iIQ_valid,
          s_axis_tready       => open,
          s_axis_tdata        => iIQ_data,
          m_axis_tvalid       => FIN_valid,
          m_axis_tready       => '1',
          m_axis_tdata        => FIN_data
    );

    ID0 : Dec01
        PORT MAP (
           aresetn            => resetn,
           aclk               => filClk,
           s_axis_data_tvalid => FIN_valid,
           s_axis_data_tready => open,
           s_axis_data_tdata  => std_logic_vector( FIN_data(31 downto 16) ),
           m_axis_data_tvalid => Id0_v16_valid,
           m_axis_data_tdata  => Id0_v16_data
    );
    
    QD0 : Dec01
        PORT MAP (
           aresetn            => resetn,
           aclk               => filClk,
           s_axis_data_tvalid => FIN_valid,
           s_axis_data_tready => open,
           s_axis_data_tdata  => std_logic_vector( FIN_data(15 downto 0) ),
           m_axis_data_tvalid => Qd0_v16_valid,
           m_axis_data_tdata  => Qd0_v16_data
    );
    
    d_v32_data <= Id0_v16_data & Qd0_v16_data;

    FOUT : F16x32
        PORT MAP (
          m_aclk              => iSampClk,
          s_aclk              => filClk,
          s_aresetn           => resetn,
          s_axis_tvalid       => Qd0_v16_valid,
          s_axis_tready       => open,
          s_axis_tdata        => d_v32_data,
          m_axis_tvalid       => oIQ_valid,
          m_axis_tready       => '1',
          m_axis_tdata        => oIQ_data
    );

    ----------------------------------------------------------------------      
    -- The following is from the Spartan6 primitive documentation   
    -- Begin instantiation template
    -- Spartan-6
    -- Xilinx HDL Libraries Guide, version 13.1
    DEB_filClk : DCM_SP
    generic map (
         CLKDV_DIVIDE       => 2.0, -- CLKDV divide value
                                    -- (1.5,2,2.5,3,3.5,4,4.5,5,5.5,6,6.5,7,7.5,8,9,10,11,12,13,14,15,16).
         CLKFX_DIVIDE       => 1, -- Divide value on CLKFX outputs - D - (1-32)
         CLKFX_MULTIPLY     => 4, -- Multiply value on CLKFX outputs - M - (2-32)
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
         CLKFX    => filClk, -- 1-bit output: Digital Frequency Synthesizer output (DFS)
         CLKFX180 => open, -- 1-bit output: 180 degree CLKFX output
         LOCKED   => open, -- 1-bit output: DCM_SP Lock Output
         PSDONE   => open, -- 1-bit output: Phase shift done output
         STATUS   => open, -- 8-bit output: DCM_SP status output
         CLKFB    => iSampClk, -- 1-bit input: Clock feedback input
         CLKIN    => iSampClk, -- 1-bit input: Clock input
         DSSEN    => '0', -- 1-bit input: Unsupported, specify to GND.
         PSCLK    => '0', -- 1-bit input: Phase shift clock input
         PSEN     => '0', -- 1-bit input: Phase shift enable
         PSINCDEC => '0', -- 1-bit input: Phase shift increment/decrement input
         RST      => '0' -- 1-bit input: Active high reset input
    );
    
end Behavioral;

