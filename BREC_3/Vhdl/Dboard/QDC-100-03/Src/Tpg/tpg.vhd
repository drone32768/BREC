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
use ieee.numeric_std.all ;
--
-- This module takes a 4 bit test pattern specifier 
-- and generates a signed 12 bit output signal.
--
entity Tpg is
    Port ( 
	        iClk : in  STD_LOGIC;
           iTpg : in  STD_LOGIC_VECTOR (  3 downto 0);
           oI   : out STD_LOGIC_VECTOR ( 11 downto 0 );
           oQ   : out STD_LOGIC_VECTOR ( 11 downto 0 )
			 );
end Tpg;

architecture Behavioral of Tpg is

COMPONENT TpgNco
  PORT (
           aclk                 : IN STD_LOGIC;
           s_axis_config_tvalid : IN STD_LOGIC;
           s_axis_config_tdata  : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
           m_axis_data_tvalid   : OUT STD_LOGIC;
           m_axis_data_tdata    : OUT STD_LOGIC_VECTOR(31 DOWNTO 0)
  );
END COMPONENT;

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

-- DDS/NCO cfg and data
signal ncoCfgValid     : STD_LOGIC;
signal ncoCfg          : STD_LOGIC_VECTOR (15 downto 0);
signal ncoCfgS         : STD_LOGIC_VECTOR( 7 downto 0 );
signal ncoDataValid    : STD_LOGIC;
signal ncoData         : STD_LOGIC_VECTOR (31 downto 0);

signal pinc            : STD_LOGIC_VECTOR( 15 downto 0 );
signal shr             : STD_LOGIC_VECTOR( 1 downto 0 );

begin
    
    -- Parameter selection
	 State : process( iClk )
	 begin
	 	if( rising_edge(iClk) ) then
        if(    iTpg = "0001" ) then
            pinc <= x"0020";
            shr  <= "00";
        elsif( iTpg = "0010" ) then
            pinc <= x"0010";
            shr  <= "01";   
        elsif( iTpg = "0011" ) then
            pinc <= x"0008";
            shr  <= "10";               
        else
            pinc <= x"0001";
            shr  <= "00";
        end if;
      end if;
    end process;
    
    -- AXI Slave Register writer to control NCO
    ASR0:  AxiSlaveReg port map(
          iInWord     => pinc,
          iWrEn       => '1',
          iAclk       => iClk,
          iArdy       => '1', 
          oAvalid     => ncoCfgValid,
          oAdata      => ncoCfg
    );
     
    -- DDS/NCO generating I/Q test pattern
    ncoCfgS <= ncoCfg( 7 downto 0 );
    NCO: TpgNco
       PORT MAP (
         aclk                 => iClk,
         s_axis_config_tvalid => ncoCfgValid,
         s_axis_config_tdata  => ncoCfgS,
         m_axis_data_tvalid   => open,
         m_axis_data_tdata    => ncoData
    );

    -- Shift final output output     
    oI <= ncoData(11 downto 0)                              when shr="00" else  
          ncoData(11) & ncoData(11 downto 1)                when shr="01" else
          ncoData(11) & ncoData(11) & ncoData(11 downto 2)  when shr="10" else
          ncoData(11) & ncoData(11) & ncoData(11) & ncoData(11 downto 3);
    oQ <= ncoData(27 downto 16)                             when shr="00" else  
          ncoData(27) & ncoData(27 downto 17)               when shr="01" else
          ncoData(27) & ncoData(27) & ncoData(27 downto 18) when shr="10" else
          ncoData(27) & ncoData(27) & ncoData(27) & ncoData(27 downto 19);
                    
end Behavioral;

