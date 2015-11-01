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
           iTpg : in  STD_LOGIC_VECTOR (3 downto 0);
           oTst : out SIGNED ( 11 downto 0 ) 
			 );
end Tpg;

architecture Behavioral of Tpg is

signal clkCount         : unsigned ( 2 downto 0 );

begin
    counter: process( iClk )
	 begin
	     if( rising_edge(iClk) ) then
		     clkCount <= clkCount + 1;
		  end if;
	 end process;
	 
	 gentp : process( iClk )
	 begin
	 	if( rising_edge(iClk) ) then
	     if(     iTpg = "0010" )   then -- 1/8Fs half scale square wave
			  if  ( clkCount(2) = '1') then 
			     oTst <= x"801"; -- signed( -1024 );
			  else   					    
			     oTst <= x"7ff"; -- signed(  1024 );
			  end if;
        elsif ( iTpg = "0100" ) then
 			  if  ( clkCount(1) = '1') then 
			     oTst <= x"ffc"; -- signed( -4 );
			  else   					    
			     oTst <= x"004"; -- signed(  4 );
			  end if;          		  
        else
           oTst <= x"3ff"; 		  
        end if;		  
		end if;
	 end process;

end Behavioral;

