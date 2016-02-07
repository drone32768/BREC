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
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

--
-- This module takes two 16 bit input and writes them to a
-- single fifo. 8 clocks are required to write both
-- values.
--
entity IqWriter is
    Port (
           iClk         : in   STD_LOGIC;
			  iIdata       : in   STD_LOGIC_VECTOR (15 downto 0);
	        iIvalid      : in   STD_LOGIC;
           iQdata       : in   STD_LOGIC_VECTOR (15 downto 0);
           iQvalid      : in   STD_LOGIC;
			  iWrGate      : in   STD_LOGIC;
			  oFifoWrData  : out  STD_LOGIC_VECTOR (15 downto 0);
           oFifoWrEn    : out  STD_LOGIC;
			  iTp          : in   STD_LOGIC
           
			);
end IqWriter;

architecture Behavioral of IqWriter is
    -- FSM signals
    type   state_type is (
	                         stWAIT_VALID,
									 stIVALID,
									 stQVALID,
									 stIQVALID,
	                         stWRITE_Q,
	                         stSELECT_I, 
								    stWRITE_I
								  );
    signal nextState    : state_type := stWAIT_VALID;
	 signal currentState : state_type := stWAIT_VALID;
	 
	 -- I and Q registered data
	 signal Idata        : STD_LOGIC_VECTOR (15 downto 0);
	 signal Qdata        : STD_LOGIC_VECTOR (15 downto 0);
	 
	 -- Selects I or Q data to be written
	 signal IqSel        : STD_LOGIC;
	 
	 -- Data for test pattern
	 signal ItestPattern : std_logic_vector(15 downto 0) := x"0000"; -- "0000000000000000";
	 signal QtestPattern : std_logic_vector(15 downto 0) := x"0025"; -- "0000000000000000";
begin

    -- State transition register
    StateReg: process( iClk )
    begin
	     if( rising_edge(iClk) ) then
	        currentState <= nextState;
		  end if;  
    end process;
	 
	 TestPattern: process( iClk )
	 begin
	      if( rising_edge(iClk) ) then
			   ItestPattern <=  unsigned(ItestPattern) + 1;
				QtestPattern <=  unsigned(QtestPattern) + 1;
			end if;
	 end process;
	 
	 -- Last valid Q data register
    QdataReg: process( iClk )
    begin
	     if( rising_edge(iClk) ) then
		     if iQvalid='1' then
	            Qdata <= iQdata;
			  end if;
		  end if;  
    end process;

    -- Last valid I data register
    IdataReg: process( iClk )
    begin
	     if( rising_edge(iClk) ) then
		     if iIvalid='1' then
	            Idata <= iIdata;
			  end if;
		  end if;  
    end process;
		
	 -- Select test pattern or actual data and I or Q
    oFifoWrData <=  Idata        when iTp='0' and IqSel='1' else
	                 Qdata        when iTp='0' and IqSel='0' else
						  ItestPattern when iTp='1' and IqSel='1' else
						  QtestPattern when iTp='1' and IqSel='0' else
						  Idata;
						  
    -- FSM to write I and then Q data to fifo
    StateMachine: process( currentState, iQvalid, iIvalid, iWrGate ) 
	 begin

	     case currentState is

        -- Enter with no valids 
		  -- Clears wr and selects Q
		  -- Waits until one or all inputs valid
	     when stWAIT_VALID =>
		      oFifoWrEn    <= '0';
				IqSel        <= '0';
				if    iIvalid='1' and iQvalid='1' then
				   nextState <= stIQVALID;
				elsif iIvalid='1' and iQvalid='0' then
				   nextState <= stIVALID;
				elsif iIvalid='0' and iQvalid='1' then
				   nextState <= stQVALID;
				else 
				   nextState <= stWAIT_VALID;
				end if;
				
			-- Q was valid
		   when stQVALID    =>
		      oFifoWrEn    <= '0';
				IqSel        <= '0';
				if  iIvalid='1' then
				   nextState <= stIQVALID;	
            else
				   nextState <= stQVALID;
            end if;				
			
			-- I was valid
			when stIVALID    =>
		      oFifoWrEn    <= '0';
				IqSel        <= '0';
				if  iQvalid='1' then
				   nextState <= stIQVALID;	
            else
				   nextState <= stIVALID;
            end if;		
				
			-- Both I and Q have been valid
			when stIQVALID   =>
				oFifoWrEn    <= '0';
				IqSel        <= '0';
				if iWrGate='1' then -- need to write all or none
				   nextState <= stWRITE_Q;
				else
				   nextState <= stWAIT_VALID;
				end if;
				
			-- Write Q to fifo (was selected previously)
		   when stWRITE_Q   =>
		      oFifoWrEn    <= '1';
				IqSel        <= '0';
				nextState    <= stSELECT_I;
				
			-- Change fifo input to I data
			when stSELECT_I =>
		      oFifoWrEn    <= '0';
				IqSel        <= '1';
				nextState    <= stWRITE_I;
				
			-- Write I to fifo (and return to waiting for valid)
			when stWRITE_I  =>
		      oFifoWrEn    <= '1';
				IqSel        <= '1';
				nextState    <= stWAIT_VALID;			
			    
         -- unknown state
	      when others =>
		      oFifoWrEn    <= '0';
				IqSel        <= '0';	
			   nextState    <= stWAIT_VALID;

	     end case;
		  
    end process;
end Behavioral;

