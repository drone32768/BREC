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

--
-- This module takes a serial input and output (in parallel form)
-- and conducts a read or write to port 0 or port 1 based on the
-- serial input word.
--
entity PortCtl is
    Port ( 
			  iSYS_CLK      : in   STD_LOGIC;
			  
	        iSERIAL_IN    : in   STD_LOGIC_VECTOR (15 downto 0);
           oSERIAL_OUT   : out  STD_LOGIC_VECTOR (15 downto 0);
           iSERIAL_BSY   : in   STD_LOGIC;
			  
			  iP0_rDATA     : in   STD_LOGIC_VECTOR (15 downto 0);
			  oP0_rEN       : out  STD_LOGIC;
			  
			  oP0_wDATA     : out  STD_LOGIC_VECTOR (15 downto 0);
			  oP0_wEN       : out  STD_LOGIC;

			  iP1_rDATA     : in   STD_LOGIC_VECTOR (15 downto 0);
			  oP1_rEN       : out  STD_LOGIC;
			  
			  oP1_wDATA     : out  STD_LOGIC_VECTOR (15 downto 0);
			  oP1_wEN       : out  STD_LOGIC

			  );
end PortCtl;

-- NOTE: A serial read or write operation take 5 system clocks to execute.
-- one to transition from waiting, one to parse, two for the read/write
-- and one to see bsy
architecture Behavioral of PortCtl is
    -- FSM variables
    type   state_type is (stWAIT_READY, 
	                       stPARSE, 
	                       stPORT_READa,  stPORT_READb,  stPORT_READc,  stPORT_READd,
                          stPORT_WRITEa, stPORT_WRITEb, stPORT_WRITEc, stPORT_WRITEd,							  
								  stWAIT_BSY );
    signal nextState    : state_type := stWAIT_BSY;
	 signal currentState : state_type := stWAIT_BSY;
	 -- Enable to load port read data to serial output
	 signal soEn        : STD_LOGIC;
begin

    -- State register
    StateReg: process( iSYS_CLK )
    begin
	     if( rising_edge(iSYS_CLK) ) then
	        currentState <= nextState;
		  end if;  
    end process;

    -- Serial out selector and register	 
	 SoReg: process( iSYS_CLK )
    begin
	     if( rising_edge(iSYS_CLK) ) then
		     if ( soEn='1' ) then
			      if ( iSERIAL_IN(0)='0' ) then
			         oSERIAL_OUT <= iP0_rDATA;
					else
					   oSERIAL_OUT <= iP1_rDATA;
					end if;
			  end if;
		  end if;  
    end process;
	 
	 -- State machine to conduct port read/writes with serial interface
    Fsm: process( currentState, iSERIAL_BSY, iSERIAL_IN  ) 
	 begin
	     -- Both ports get the serial input register
        oP0_wDATA    <= iSERIAL_IN;
        oP1_wDATA    <= iSERIAL_IN;
				
	     case currentState is

        -- enter iSERIAL_BSY=1
	     when stWAIT_READY =>
            oP0_rEN      <= '0';
				oP1_rEN      <= '0';
				oP0_wEN      <= '0';
				oP1_wEN      <= '0';
				soEn         <= '0';
		      if iSERIAL_BSY='0' then
				    nextState <= stPARSE;
				else 
				    nextState <= stWAIT_READY;
			   end if;
				
        -- stPARSE reg0 command
		  	-- Command Word is
			-- 1111 1100 0000 0000
			-- 5432 1098 7654 3210
			--                RWXP 
			--
			-- R = 1 => read
			-- W = 1 => write
			-- P = 0(port0), 1(port1)
			--
	     when stPARSE =>
            oP0_rEN      <= '0';
				oP1_rEN      <= '0';
				oP0_wEN      <= '0';
				oP1_wEN      <= '0';
				soEn         <= '0';
				-- read command
			   if    iSERIAL_IN(3)='1' then       
			       nextState    <= stPORT_READa;
			   -- write command
			   elsif iSERIAL_IN(2)='1' then        
			       nextState    <= stPORT_WRITEa;
				-- unrecognized command
				else 
					 nextState    <= stWAIT_BSY;
			   end if;
				
		   --  read port data
			when stPORT_READa =>
			   if( iSERIAL_IN(0)='0' ) then
				    oP0_rEN  <= '1';
					 oP1_rEN  <= '0';
				else
					 oP0_rEN  <= '0';
					 oP1_rEN  <= '1';
				end if;
				oP0_wEN      <= '0';
				oP1_wEN      <= '0';
				soEn         <= '0';
				nextState    <= stPORT_READb;
			when stPORT_READb =>
            oP0_rEN      <= '0';
				oP1_rEN      <= '0';
				oP0_wEN      <= '0';
				oP1_wEN      <= '0';
				soEn         <= '1';
            nextState    <= stWAIT_BSY;
            			
		   --  write port data
			when stPORT_WRITEa =>
			   oP0_rEN      <= '0';
            oP1_rEN      <= '0';
			   if( iSERIAL_IN(0)='0' ) then
				    oP0_wEN  <= '1';
					 oP1_wEN  <= '0';
				else
					 oP0_wEN  <= '0';
					 oP1_wEN  <= '1';
				end if;
				soEn         <= '0';				
				nextState    <= stPORT_WRITEb;
			when stPORT_WRITEb => -- blank state
			   oP0_rEN      <= '0';
            oP1_rEN      <= '0';
				oP0_wEN      <= '0';
				oP1_wEN      <= '0';	
				soEn         <= '0';
            nextState    <= stWAIT_BSY;
		
			-- enter iSERIAL_BSY=0 
			when stWAIT_BSY =>
			   oP0_rEN      <= '0';
            oP1_rEN      <= '0';
				oP0_wEN      <= '0';
				oP1_wEN      <= '0';	
				soEn         <= '0';
		      if iSERIAL_BSY='1' then
			       nextState   <= stWAIT_READY; 
			   else
				    nextState   <= stWAIT_BSY; 
			   end if;

         -- unknown state
	      when others =>
			   oP0_rEN      <= '0';
            oP1_rEN      <= '0';
				oP0_wEN      <= '0';
				oP1_wEN      <= '0';	
				soEn         <= '0';			
			   nextState    <= stWAIT_BSY;

	     end case;
    end process;
	 
end Behavioral;

