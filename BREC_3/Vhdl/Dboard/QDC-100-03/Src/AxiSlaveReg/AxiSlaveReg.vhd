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

entity AxiSlaveReg is
    Port ( iInWord : in   STD_LOGIC_VECTOR (15 downto 0);
           iWrEn   : in   STD_LOGIC;
           iAclk   : in   STD_LOGIC;
           iArdy   : in   STD_LOGIC;
           oAvalid : out  STD_LOGIC;
           oAdata  : out  STD_LOGIC_VECTOR (15 downto 0)
         );
end AxiSlaveReg;

--
-- This module takes a 16 bit unclocked input and
-- writes it to a Axi slave register when the input
-- data changes.
--
-- NOTE: Care must be taken with the input.  Axi 
-- register writes will occur as long as the input
-- pattern changes.
--
architecture Behavioral of AxiSlaveReg is
    -- FSM signals
    type   state_type is (
                            stWAIT_CHANGE,
                            stLATCH_DATA,
                            stASSERT_VALID, 
                            stWAIT_RDY
                          );
    signal nextState    : state_type := stWAIT_CHANGE;
    signal currentState : state_type := stWAIT_CHANGE;
    
    -- Registered version of data to be written
    signal nextData     : STD_LOGIC_VECTOR (15 downto 0) := "0000000000000000";
    signal currentData  : STD_LOGIC_VECTOR (15 downto 0) := "0000000000000000";
begin

    -- state register
    state: process( iAclk )
    begin
        if( rising_edge(iAclk) ) then
           currentState <= nextState;
        end if;  
    end process;
    
    -- register data and changes to clock
    data: process( iAclk )
    begin
        if( rising_edge(iAclk) ) then
           currentData <= nextData;
        end if;  
    end process;

    -- FSM
    logic: process( currentState, iInWord, currentData, iWren, iARdy ) 
    begin
        oAdata <= currentData;
            
        case currentState is

        when stWAIT_CHANGE =>
            oAvalid      <= '0';
            nextData     <= iInWord;
            if iWrEn='1' and ( iInWord /= currentData ) then
                nextState <= stLATCH_DATA;
            else 
                nextState <= stWAIT_CHANGE;
            end if;
            
         when stLATCH_DATA   =>
            oAvalid      <= '0';
            nextData     <= currentData;
            nextState    <= stASSERT_VALID;
            
         when stASSERT_VALID =>
            oAvalid      <= '1';
            nextData     <= currentData;
            nextState    <= stWAIT_RDY;
            
         when stWAIT_RDY     =>
            oAvalid      <= '0';
            nextData     <= currentData;   
            if iArdy='1'  then
                 nextState <= stWAIT_CHANGE;
            else
                 nextState <= stWAIT_RDY;
            end if;            
             
         -- unknown state
         when others =>
            oAvalid      <= '0';
            nextData     <= currentData;      
            nextState    <= stWAIT_CHANGE;

        end case;
    end process;
    
end Behavioral;

