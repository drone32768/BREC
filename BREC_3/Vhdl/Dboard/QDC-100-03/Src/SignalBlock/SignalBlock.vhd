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
use ieee.numeric_std.all ;

-- For Spartan6 specific components (e.g. DCM instantiation)
Library UNISIM;
use UNISIM.vcomponents.all;

--000000001111111111222222222233333333334444444444555555555566666666667777777777
--234567890123456789012345678901234567890123456789012345678901234567890123456789   

entity SignalBlock is
    Port ( 
           iSampClk    : in  STD_LOGIC;
           iAdcI       : in  STD_LOGIC_VECTOR (11 downto 0);
           iAdcQ       : in  STD_LOGIC_VECTOR (11 downto 0);
           
           iFifoRst    : in  STD_LOGIC;
           oFifoRdData : out STD_LOGIC_VECTOR (15 downto 0);
           iFifoRdClk  : in  STD_LOGIC;
           iFifoRdEn   : in  STD_LOGIC;
           iFifoWrInhib: in  STD_LOGIC;
           oFifoThresh : out STD_LOGIC;
           
           iSrcSel     : in  STD_LOGIC_VECTOR( 3  downto 0 );
           iGenTp      : in  STD_LOGIC_VECTOR( 3  downto 0 );
           iPinc       : in  STD_LOGIC_VECTOR( 15 downto 0 );
           oStatus     : out STD_LOGIC_VECTOR( 15 downto 0 );
           
           iIoff       : in  STD_LOGIC_VECTOR( 7  downto 0 );
           iInum       : in  STD_LOGIC_VECTOR( 7  downto 0 );
           iQoff       : in  STD_LOGIC_VECTOR( 7  downto 0 );
           iQnum       : in  STD_LOGIC_VECTOR( 7  downto 0 )
    );
end SignalBlock;

architecture Behavioral of SignalBlock is
COMPONENT Tpg is
    Port ( 
           iClk          : in  STD_LOGIC;
           iTpg          : in  STD_LOGIC_VECTOR (  3 downto 0);
           oI            : out STD_LOGIC_VECTOR ( 11 downto 0 );
           oQ            : out STD_LOGIC_VECTOR ( 11 downto 0 )
         );
end COMPONENT;

COMPONENT ChnlMatch is
    Port ( 
           iSampClk     : in   STD_LOGIC;
           iInput       : in   STD_LOGIC_VECTOR (11 downto 0);
           oOutput      : out  STD_LOGIC_VECTOR (11 downto 0);
           iAdd         : in   STD_LOGIC_VECTOR (7 downto 0);
           iMul         : in   STD_LOGIC_VECTOR (7 downto 0)
          );
end COMPONENT;

COMPONENT AxiSlaveReg is
    Port ( 
           iInWord       : in   STD_LOGIC_VECTOR (15 downto 0);
           iWrEn         : in   STD_LOGIC;
           iAclk         : in   STD_LOGIC;
           iArdy         : in   STD_LOGIC;
           oAvalid       : out  STD_LOGIC;
           oAdata        : out  STD_LOGIC_VECTOR (15 downto 0)
         );
end COMPONENT;

COMPONENT Nco
  PORT (
    aclk                 : IN STD_LOGIC;
    s_axis_config_tvalid : IN STD_LOGIC;
    s_axis_config_tdata  : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
    m_axis_data_tvalid   : OUT STD_LOGIC;
    m_axis_data_tdata    : OUT STD_LOGIC_VECTOR(31 DOWNTO 0)
  );
END COMPONENT;

COMPONENT QM00
  PORT (
    aclk                 : IN STD_LOGIC;
    aresetn              : IN STD_LOGIC;
    s_axis_a_tvalid      : IN STD_LOGIC;
    s_axis_a_tdata       : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
    s_axis_b_tvalid      : IN STD_LOGIC;
    s_axis_b_tdata       : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
    m_axis_dout_tvalid   : OUT STD_LOGIC;
    m_axis_dout_tdata    : OUT STD_LOGIC_VECTOR(31 DOWNTO 0)
  );
END COMPONENT;

COMPONENT DEA
  PORT (
    iRst                 : in  STD_LOGIC; 
    iSampClk             : in  STD_LOGIC;
    iIQ_data             : in  STD_LOGIC_VECTOR (31 downto 0);
    iIQ_valid            : in  STD_LOGIC;
    oIQ_data             : out STD_LOGIC_VECTOR (31 downto 0);
    oIQ_valid            : out STD_LOGIC
  );
END COMPONENT;

COMPONENT DEB
  PORT (
    iRst                 : in  STD_LOGIC; 
    iSampClk             : in  STD_LOGIC;
    iIQ_data             : in  STD_LOGIC_VECTOR (31 downto 0);
    iIQ_valid            : in  STD_LOGIC;
    oIQ_data             : out STD_LOGIC_VECTOR (31 downto 0);
    oIQ_valid            : out STD_LOGIC
  );
END COMPONENT;

COMPONENT SampFifo
  PORT (
    rst                : IN STD_LOGIC;
    wr_clk             : IN STD_LOGIC;
    rd_clk             : IN STD_LOGIC;
    din                : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
    wr_en              : IN STD_LOGIC;
    rd_en              : IN STD_LOGIC;
    dout               : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    full               : OUT STD_LOGIC;
    empty              : OUT STD_LOGIC;
    prog_empty         : OUT STD_LOGIC
  );
END COMPONENT;

-- 
--
--            I_v12   I_eq_v12                                                 
-- iAdcI-->MUX----->CMI+                                                     
--          ^  (0)     |    +--ncoData                           
--          |          |    v  (2)                               
-- Itp_v12 ---         |   ---                  ---                 ---
--        |   |        |  |CMP| IQmx_v32_data  |DEA|Dea0_v32_data  |DEB|Deb0_v32_data
--        |TPG|     (1)+->|MUL|--------------->|   |-------------->|   |-------------->
--        |   |        |  |   |     (3)        |   |     (4)       |   |    (5)
-- Qtp_v12 ---         |   ---                  ---                 --- 
--          |          |                                         
--          v  (0)     |                                         
-- iAdcQ-->MUX----->CMQ+                                                      
--            Q_v12   Q_eq_v12                                                          
--              

-- Fifo write controls and data
signal fifoWrData      : STD_LOGIC_VECTOR ( 31 downto 0 );
signal fifoWrEn        : STD_LOGIC;
signal fifoWrValid     : STD_LOGIC;

-- Test pattern
signal Itp_v12          : STD_LOGIC_VECTOR ( 11 downto 0 );
signal Qtp_v12          : STD_LOGIC_VECTOR ( 11 downto 0 );

-- Input I and Q (either actual ADC values or test pattern)
signal I_v12           : STD_LOGIC_VECTOR ( 11 downto 0 );
signal Q_v12           : STD_LOGIC_VECTOR ( 11 downto 0 );
signal IQin_v32_data   : STD_LOGIC_VECTOR ( 31 downto 0 );

-- Matched I and Q input
signal Ieq_v12         : STD_LOGIC_VECTOR ( 11 downto 0 );
signal Qeq_v12         : STD_LOGIC_VECTOR ( 11 downto 0 );
signal IQeq_v32_data   : STD_LOGIC_VECTOR ( 31 downto 0 );
signal IQes_v32_data   : STD_LOGIC_VECTOR ( 31 downto 0 );

-- DDS/NCO cfg and data
signal ncoCfgValid     : STD_LOGIC;
signal ncoCfg          : STD_LOGIC_VECTOR (15 downto 0);
signal ncoDataValid    : STD_LOGIC;
signal ncoData         : STD_LOGIC_VECTOR (31 downto 0);

-- Mixed I and Q 
signal IQmx_v32_data   : STD_LOGIC_VECTOR( 31 downto 0);
signal IQmx_v32_valid  : STD_LOGIC;
    
-- First stage decimator output
signal Dea0_v32_data  : STD_LOGIC_VECTOR( 31 downto 0 );
signal Dea0_v32_valid : STD_LOGIC;

-- Second stage decimator output
signal Deb0_v32_data  : STD_LOGIC_VECTOR( 31 downto 0 );
signal Deb0_v32_valid : STD_LOGIC;

-- Active low version of input reset
signal resetn          : STD_LOGIC;

begin
                    
    -- Output status                
    oStatus     <= std_logic_vector( iPinc ); 

    -- Active low version of reset
    resetn         <= not iFifoRst;
    
    -- Test Pattern Generator 
    TPG0:  Tpg port map (
          iClk        => iSampClk,
          iTpg        => iGenTp,
          oI          => Itp_v12,
          oQ          => Qtp_v12
    );
                                                
    -- Mux to select inputs as ADC or TPG
    -- The input is the ADC unless using a test pattern
    I_v12       <=  iAdcI  when iGenTp  = "0000" else
                    Itp_v12;                  
    Q_v12       <=  iAdcQ  when iGenTp  = "0000" else
                    Qtp_v12; 

    -- Combine the input i and q for output
    IQin_v32_data <= I_v12 & "0000" & Q_v12 & "0000";
    
    -- Match/Equalize I
    CMI : ChnlMatch 
       PORT MAP ( 
           iSampClk           => iSampClk,
           iInput             => I_v12,
           oOutput            => Ieq_v12,
           iAdd               => iIoff,
           iMul               => iInum
    );

    -- Match/Equalize Q
    CMQ : ChnlMatch 
       PORT MAP ( 
           iSampClk           => iSampClk,
           iInput             => Q_v12,
           oOutput            => Qeq_v12,
           iAdd               => iQoff,
           iMul               => iQnum
    );

    -- Combine the equalized  i and q (for output)
    IQeq_v32_data <= Ieq_v12 & "0000" & Qeq_v12 & "0000";
    
    -- DDS/NCO (Generated dds. 12 bit out, 16 bit phase)
    NCO0 : Nco
       PORT MAP (
          aclk                 => iSampClk,
          s_axis_config_tvalid => ncoCfgValid,
          s_axis_config_tdata  => ncoCfg,
          m_axis_data_tvalid   => ncoDataValid,
          m_axis_data_tdata    => ncoData
       );
       
    -- AXI Slave Register writer to control NCO
    ASR0:  AxiSlaveReg port map(
          iInWord     => iPinc,
          iWrEn       => '1',
          iAclk       => iSampClk,
          iArdy       => '1', 
          oAvalid     => ncoCfgValid,
          oAdata      => ncoCfg
    );
    
    -- Combine equalized I and Q for multiplication
    -- NOTE: the complex multiplier inputs are 12 bits signed, however,
    -- the generated core interface uses an even number of bytes as input
    IQes_v32_data <= "0000" & Ieq_v12 & "0000" & Qeq_v12;    
    
    --  Quadrature(complex) Multiply the NCO signal by the equalized ADC
    CMPY : QM00
        PORT MAP (
          aclk               => iSampClk,
          aresetn            => resetn,
          s_axis_a_tvalid    => '1',
          s_axis_a_tdata     => ncoData,
          s_axis_b_tvalid    => '1',
          s_axis_b_tdata     => IQes_v32_data,
          m_axis_dout_tvalid => IQmx_v32_valid,
          m_axis_dout_tdata  => IQmx_v32_data
    );
  
    -- Stage A decimator
    DEA0: DEA port map(
          iRst         => iFifoRst,
          iSampClk     => iSampClk,
          iIQ_data     => IQmx_v32_data,
          iIQ_valid    => IQmx_v32_valid,
          oIQ_data     => Dea0_v32_data,
          oIQ_valid    => Dea0_v32_valid
    );

    -- Stage B decimator
    DEB0: DEB port map(
          iRst         => iFifoRst,
          iSampClk     => iSampClk,
          iIQ_data     => Dea0_v32_data,
          iIQ_valid    => Dea0_v32_valid,
          oIQ_data     => Deb0_v32_data,
          oIQ_valid    => Deb0_v32_valid
    );
    
    -- What gets written to fifo is determined by selector
    fifoWrData  <=  IQin_v32_data                   when iSrcSel="0000" else
                    IQeq_v32_data                   when iSrcSel="0001" else
                    ncoData                         when iSrcSel="0010" else
                    IQmx_v32_data                   when iSrcSel="0011" else                                                   
                    Dea0_v32_data                   when iSrcSel="0100" else    
                    Deb0_v32_data                   when iSrcSel="0101" else 
                    IQeq_v32_data ;
                    
    fifoWrValid <=  '1'                             when iSrcSel="0000" else
                    '1'                             when iSrcSel="0001" else
                    '1'                             when iSrcSel="0010" else
                    '1'                             when iSrcSel="0011" else                    
                    Dea0_v32_valid                  when iSrcSel="0100" else
                    Deb0_v32_valid                  when iSrcSel="0101" else
                    '1';   
                    
    fifoWrEn   <= fifoWrValid and ( not iFifoWrInhib );
                  
    -- Generated fifo core.  
    -- 32 bit wide input,
    -- 16 bit wide output.  
    -- 2k indicator.
    FIF : SampFifo
       PORT MAP (
          rst        => iFifoRst,
          wr_clk     => iSampClk,
          rd_clk     => iFifoRdClk,
          din        => fifoWrData,
          wr_en      => fifoWrEn,
          rd_en      => iFifoRdEn,
          dout       => oFifoRdData,
          full       => open,
          empty      => open,
          prog_empty => oFifoThresh
    );
     
end Behavioral;

