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
use IEEE.std_logic_unsigned.all;
use ieee.numeric_std.all;

--000000001111111111222222222233333333334444444444555555555566666666667777777777
--234567890123456789012345678901234567890123456789012345678901234567890123456789	
--
-- This module acts as a SPI slave interface with SS low, and rising
-- SCLK.
--
-- MOSI, MISO, SCLK, SS are standard spi interfaces
-- Busy  high indicates data is being clocked in/out of the serial
-- interface.  Busy low indicates no data is being clocked in/out.
-- SO value at busy start is clocked out.
-- SI value at busy stop is value clocked in.
--
entity SpiSlave is
    Port ( 
	        iDSPI : in  STD_LOGIC;
	        iMOSI : in  STD_LOGIC_VECTOR( 1 downto 0);
           oMISO : out STD_LOGIC_VECTOR( 1 downto 0);
           iSCLK : in  STD_LOGIC;
           iSS   : in  STD_LOGIC;
			  
           oBSY  : out STD_LOGIC;
           oSI   : out STD_LOGIC_VECTOR (15 downto 0);
           iSO   : in  STD_LOGIC_VECTOR (15 downto 0)
		);
end SpiSlave;

architecture Behavioral of SpiSlave is
signal bitPos : std_logic_vector(3 downto 0);
begin

	 -- NOTE: This is explicitly coded this way to avoid a register
	 -- with an asynch load.  This is appears to always synthesize as 
	 -- a FF with set/reset which is not supported on Spartan6.
	 -- This results in both warnings and RTL with nonsensical input/outputs
	 
	 -- NOTE: This is explicitly coded this way to prevent r/w on SS
	 -- changes only.  A busy indicator requires SS low with 16 valid
	 -- SCLK.  SS low resets the bit position to allow resynch of serial stream
	 
    process(iSCLK, iSS)
    begin     
	     if iSS='0' then
			  if rising_edge(iSCLK) then
			      if( iDSPI='1' ) then
					    oSI(   conv_integer(bitPos)    ) <= iMOSI(0);
						 oSI(   conv_integer(bitPos) - 1) <= iMOSI(1);
					    oMISO(0) <= iSO(   conv_integer(bitPos)      );	
                   oMISO(1) <= iSO(   conv_integer(bitPos) - 1  );						 
					else
					    oSI(   conv_integer(bitPos)    ) <= iMOSI(0);
					    oMISO(0) <= iSO(   conv_integer(bitPos)      );
                   oMISO(1) <= iMOSI(1);						 
               end if;					
			  end if;
			  if falling_edge(iSCLK) then
			      if( iDSPI='1' ) then
						if( bitPos="0001" ) then
							 oBSY <= '0';
						else
							 oBSY <= '1';
						end if;
					   bitPos  <= std_logic_vector( unsigned( bitPos ) - 2 );
					else
						if( bitPos="0000" ) then
							 oBSY <= '0';
						else
							 oBSY <= '1';
						end if;
   	            bitPos  <= std_logic_vector( unsigned( bitPos ) - 1 );	
               end if;					
			  end if;
		  else
		      bitPos <= "1111";
		  end if;
    end process;
	 
end Behavioral;
