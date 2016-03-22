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

--000000001111111111222222222233333333334444444444555555555566666666667777777777
--234567890123456789012345678901234567890123456789012345678901234567890123456789   
--
--
-- Selectable register (8 bits wide).  Captures input value
-- on rising clock when write enable and selector matches id
-- 
entity Srg8 is
    Port ( iClk   : in  STD_LOGIC;
           iSel   : in  STD_LOGIC_VECTOR (5 downto 0);
           iId    : in  STD_LOGIC_VECTOR (5 downto 0);
           iData  : in  STD_LOGIC_VECTOR (7 downto 0);
           oData  : out STD_LOGIC_VECTOR (7 downto 0);
           iWen   : in  STD_LOGIC
           );
end Srg8;

architecture Behavioral of Srg8 is
signal regVal     : STD_LOGIC_VECTOR (7 downto 0) := "00000000";
begin
     oData <= regVal;
     
     reg_proc: process( iClk )
     begin
        if( rising_edge(iClk) ) then
           if ( iSel=iId and iWen='1' ) then
               regVal <= iData;
           end if;
        end if;  
     end process;

end Behavioral;


