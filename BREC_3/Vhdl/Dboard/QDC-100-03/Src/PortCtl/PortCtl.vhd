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
           
           iRD           : in   STD_LOGIC_VECTOR (15 downto 0);
           oRE           : out  STD_LOGIC;
           
           oWD           : out  STD_LOGIC_VECTOR (15 downto 0);
           oWE           : out  STD_LOGIC;
           
           oNsel         : out  STD_LOGIC_VECTOR (5 downto 0 )

           );
end PortCtl;

--
--           --+                                                          +-----
--             |                                                          |
--             |                                                          |
-- iSERIAL_BSY +----------------------------------------------------------+
--
-- iSERIAL_IN  xVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVxxxxxxxxx
--
-- iSERIAL_OUT xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxVVVVVVVVVVVVVVVVVVVVVVVVVVVV
--
--                wait                   reada,      readb        wait
--                rdy         parse      writea      writeb       bsy
--               +-----+     +-----+     +-----+     +-----+     +-----+
--               |     |     |     |     |     |     |     |     |     |
--               |     |     |     |     |     |     |     |     |     |
-- iSYS_CLK   ---+     +-----+     +-----+     +-----+     +-----+     +----
--
-- oWE        000000000000000000000000000001111111111111000000000000000000000000000000         
-- oRE        000000000000000000000000000001111111111111000000000000000000000000000000
-- soEN       000000000000000000000000000000000000000001111111111110000000000000000000
-- oWD        xVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVxxxxxxxxxxx        
-- iRD        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
--
architecture Behavioral of PortCtl is
    -- FSM variables
    type   state_type is (stWAIT_READY,  stWAIT_READYb,
                          stPARSE, 
                          stPORT_READa,  stPORT_READb, stPORT_READc,
                          stPORT_WRITEa, stPORT_WRITEb,                    
                          stWAIT_BSY,    stWAIT_BSYb );
    signal nextState    : state_type := stWAIT_BSY;
    signal currentState : state_type := stWAIT_BSY;
    -- Enable to load port read data to serial output
    signal soEn         : STD_LOGIC;
    signal bsy          : STD_LOGIC;
begin

    -- State register
    StateReg: process( iSYS_CLK )
    begin
        if( rising_edge(iSYS_CLK) ) then
           currentState <= nextState;
        end if;  
    end process;

    -- Latch serial out    
    SoReg: process( iSYS_CLK )
    begin
        if( rising_edge(iSYS_CLK) ) then
           if ( soEn='1' ) then
               oSERIAL_OUT <= iRD;
           end if;
        end if;  
    end process;
    
    -- Synchronize latch.
    -- While this should not be strictl necessary,
    -- depending on the final implementation asynch
    -- transitions on this input cause problems with
    -- the FSM outputs (read enable)
    BsyLatch: process( iSYS_CLK )
    begin
        if( rising_edge(iSYS_CLK) ) then
            bsy <= iSERIAL_BSY;
        end if;  
    end process;    
    
    -- State machine to conduct port read/writes with serial interface
    Fsm: process( currentState, bsy, iSERIAL_IN  ) 
    begin
        -- write data is serial input (always)
        oWD    <= iSERIAL_IN;
        oNsel  <= iSERIAL_IN(13 downto 8);
            
        case currentState is

        -- enter iSERIAL_BSY=1 (b)
        when stWAIT_READY =>
            oRE      <= '0';
            oWE      <= '0';
            soEn     <= '0';
            if bsy='0' then
                nextState <= stPARSE;
            else 
                nextState <= stWAIT_READY;
            end if;
            
        -- stPARSE reg0 command
           -- Command Word is
         -- 1111 1100 0000 0000
         -- 5432 1098 7654 3210
         -- RWNN NNNN 
         --
         -- R = 1 => read
         -- W = 1 => write
         -- N = selector
         --
        when stPARSE =>
            oRE      <= '0';
            oWE      <= '0';
            soEn     <= '0';
            -- read command
            if    iSERIAL_IN(15)='1' then       
                nextState    <= stPORT_READa;
            -- write command
            elsif iSERIAL_IN(14)='1' then        
                nextState    <= stPORT_WRITEa;
            -- unrecognized command
            else 
                nextState    <= stWAIT_BSY;
            end if;
            
         --  read port data
         when stPORT_READa =>
            oRE      <= '1';
            oWE      <= '0';
            soEn     <= '0';
            nextState<= stPORT_READb;      
         when stPORT_READb =>
            oRE      <= '0';
            oWE      <= '0';
            soEn     <= '1';
            nextState<= stWAIT_BSY;
                     
         --  write port data
         when stPORT_WRITEa =>
            oRE      <= '0';
            oWE      <= '1';
            soEn     <= '0';      
            nextState<= stPORT_WRITEb;
         when stPORT_WRITEb => -- blank state
            oRE      <= '0';
            oWE      <= '0';
            soEn     <= '0';
            nextState<= stWAIT_BSY;
               
         -- enter iSERIAL_BSY=0 
         when stWAIT_BSY =>
            oRE      <= '0';
            oWE      <= '0';
            soEn     <= '0';
            if bsy='1' then
                nextState   <= stWAIT_READY; 
            else
                nextState   <= stWAIT_BSY; 
            end if;
 
         -- unknown state
         when others =>
            oRE      <= '0';
            oWE      <= '0';
            soEn     <= '0';   
            nextState<= stWAIT_BSY;

        end case;
    end process;
    
end Behavioral;

