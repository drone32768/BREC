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

entity ChnlMatch is
    Port ( 
           iSampClk     : in   STD_LOGIC;
           iInput       : in   STD_LOGIC_VECTOR (11 downto 0);
           oOutput      : out  STD_LOGIC_VECTOR (11 downto 0);
           iAdd         : in   STD_LOGIC_VECTOR (7 downto 0);
           iMul         : in   STD_LOGIC_VECTOR (7 downto 0)
          );
end ChnlMatch;

architecture Behavioral of ChnlMatch is
signal postOffset           : signed  ( 11 downto 0 );
signal postNum              : signed  ( 19 downto 0 );
begin

    -- Add the specified offset to remove DC bias
    ADD0: process( iSampClk )
    begin
        if( rising_edge(iSampClk) ) then
           postOffset <= signed(iInput) + resize(signed(iAdd), postOffset'length);
        end if;
    end process;

    -- Multiply unbiased signal by specified multiplier
    -- ( the multiply should be N, and we will divide by 128
    -- later so this has the effect of multiply by N/128)
    MUL0: process( iSampClk )
    begin
        if( rising_edge(iSampClk) ) then
           postNum <=  signed( postOffset ) * signed( iMul );
        end if;
    end process;

    -- Select the high bits
    -- Equivalent to right shift by 7 (or divide by 128)
    oOutput <= STD_LOGIC_VECTOR( postNum( 18 downto 7 ) );
    
end Behavioral;

