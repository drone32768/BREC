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
use IEEE.STD_LOGIC_UNSIGNED.ALL;

-- For Spartan6 
Library UNISIM;
use UNISIM.vcomponents.all;

entity Top is
	Port ( 
			  
		     HSPI_SS      : in  STD_LOGIC;
		     HSPI_MOSI    : in  STD_LOGIC_VECTOR( 1 downto 0 );
		     HSPI_MISO    : out STD_LOGIC_VECTOR( 1 downto 0 );
		     HSPI_SCLK    : in  STD_LOGIC;
			  
			  FSPI_SS      : out  STD_LOGIC;
		     FSPI_MOSI    : out  STD_LOGIC;
		     FSPI_MISO    : in   STD_LOGIC;
		     FSPI_SCLK    : out  STD_LOGIC

	);
end Top;

architecture structure of Top is

begin
    FSPI_SS      <= HSPI_SS;
	 FSPI_MOSI    <= HSPI_MOSI(0);
	 FSPI_SCLK    <= HSPI_SCLK;
	 
	 HSPI_MISO(0) <= FSPI_MISO;
	 HSPI_MISO(1) <= '1';

end structure;

