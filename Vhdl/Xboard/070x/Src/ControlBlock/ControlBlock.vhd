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
-- This module is the primary digital interface block including
-- control and streaming interface.  It includes multiple 
-- internal registers and counters along with status inputs
-- physical indicator outputs and a fifo and SPI interface.
--
entity ControlBlock is
    Port ( 
           iMOSI        : in  STD_LOGIC;
           oMISO        : out STD_LOGIC;
           iSCLK        : in  STD_LOGIC;
           iSS          : in  STD_LOGIC;

			  oLEDS        : out STD_LOGIC_VECTOR( 3 downto 0 );
			  
			  iClk         : in  STD_LOGIC;
			  iAltClk      : in  STD_LOGIC;
			  
			  oFifoRst     : out STD_LOGIC;
			  oFifoSel     : out STD_LOGIC_VECTOR ( 3 downto 0);
			  iFifoRdData  : in  STD_LOGIC_VECTOR( 15 downto 0 );
			  oFifoRdEn    : out STD_LOGIC;
			  iFifoMark    : in  STD_LOGIC;
			  oFifoWrGate  : out STD_LOGIC;
			  
			  oGenTp       : out STD_LOGIC_VECTOR ( 3 downto 0 );
			  oPinc        : out STD_LOGIC_VECTOR ( 15 downto 0 );
			  iStatus      : in  STD_LOGIC_VECTOR ( 15 downto 0 )
	 );  
end ControlBlock;

architecture Behavioral of ControlBlock is

component SpiSlave is
    Port ( 
	        iMOSI         : in  STD_LOGIC; 
           oMISO         : out STD_LOGIC;
           iSCLK         : in  STD_LOGIC;
           iSS           : in  STD_LOGIC;
			  
           oBSY          : out STD_LOGIC;
           oSI           : out STD_LOGIC_VECTOR (15 downto 0);
           iSO           : in  STD_LOGIC_VECTOR (15 downto 0)
		);
end component;

component PortCtl is
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
end component;

component Cnt16 is 
     Port (
	        EN            : in   std_logic;
		     CLK	          : in	  std_logic;
		     DOUT	       : out  std_logic_vector(15 downto 0)
  	        );
end component;

-- Serial input
signal ctlSi          : STD_LOGIC_VECTOR (15 downto 0);

-- Serial output
signal ctlSo          : STD_LOGIC_VECTOR (15 downto 0);

-- Bsy from serial slave interface
signal ctlBsy         : STD_LOGIC;

-- R0 = main control register
signal ctlReg00        : STD_LOGIC_VECTOR (15 downto 0) := "0000000000000000";

-- R3 = secondary control register
signal ctlReg03        : STD_LOGIC_VECTOR (15 downto 0) := "0000000000000000";

-- R6 = PINC
signal ctlReg06        : STD_LOGIC_VECTOR (15 downto 0) := "0000000000000000";

-- R7 = instantaneous ADC value
signal ctlReg07        : STD_LOGIC_VECTOR (15 downto 0) := "0000000000000000";

-- R8 = version register
signal ctlReg08        : STD_LOGIC_VECTOR (15 downto 0);

-- Port 1 r/w/enables
signal ctlP1WrData    : STD_LOGIC_VECTOR (15 downto 0);
signal ctlP1WrEn      : STD_LOGIC;
signal ctlP1RdData    : STD_LOGIC_VECTOR (15 downto 0);
signal ctlP1RdEn      : STD_LOGIC;

-- Port 0 r/w/enables
signal ctlP0WrData    : STD_LOGIC_VECTOR (15 downto 0);
signal ctlP0WrEn      : STD_LOGIC;
signal ctlP0RdData    : STD_LOGIC_VECTOR (15 downto 0);
signal ctlP0RdEn      : STD_LOGIC;

-- Selector for what connects to Port 1
signal ctlP1Selector  : STD_LOGIC_VECTOR ( 3 downto 0);

-- Writeable Register write enables
signal ctlReg00WrEn    : STD_LOGIC;
signal ctlReg03WrEn    : STD_LOGIC;
signal ctlReg06WrEn    : STD_LOGIC;
signal ctlReg07WrEn    : STD_LOGIC;

-- Various diagnostic counters
signal ctlP1RdCntData  : STD_LOGIC_VECTOR (15 downto 0);
signal ctlClk1CntData  : STD_LOGIC_VECTOR (15 downto 0);
signal ctlClk2CntData  : STD_LOGIC_VECTOR (15 downto 0);

begin

     -- Set version
     ctlReg08         <= x"0704";
	  
	  -- Fifo Control bits
	  -- NOTE: oFifoRdEn controlled by 
	  -- selector (see below)
	  oFifoWrGate      <= ctlReg00(15);
	  oFifoRst         <= ctlReg00(14);
	  
	  -- oLEDS = control word fields
	  oLEDS(3)         <= ctlReg00(11);
	  oLEDS(2)         <= ctlReg00(10);
	  oLEDS(1)         <= ctlReg00(9);
	  oLEDS(0)         <= ctlReg00(8); 
	  
	  -- oPinc is upper 12 bits of r6
	  oPinc( 15 downto 12 ) <= ctlReg07( 7 downto 4 );
	  oPinc( 11 downto 0  ) <= ctlReg06( 15 downto 4 );
	  	  
	  -- P1 access selected by r0 control bits 
	  ctlP1Selector(3) <= ctlReg00(7);
	  ctlP1Selector(2) <= ctlReg00(6);
	  ctlP1Selector(1) <= ctlReg00(5);
	  ctlP1Selector(0) <= ctlReg00(4);
	  
	  -- Fifo input source selected by r3 control bits
	  oFifoSel(3)      <= ctlReg03(15);
	  oFifoSel(2)      <= ctlReg03(14);
     oFifoSel(1)      <= ctlReg03(13);
     oFifoSel(0)      <= ctlReg03(12);
	  
	  -- Generate test pattern by r3 control bits
	  oGenTp(3)        <= ctlReg03(11);
	  oGenTp(2)        <= ctlReg03(10);
	  oGenTp(1)        <= ctlReg03(9);
	  oGenTp(0)        <= ctlReg03(8);

	  -- P0 always read register 0 and status
	  ctlP0RdData      <= ctlReg00(15 downto 14) &
	                      iFifoMark              &
								 ctlReg00(12 downto 0 );

     -- P0 writes alway register 0
     ctlReg00WrEn     <= ctlP0WrEn;
	  
     -- P1 reads based on selector bits	  
	  ctlP1RdData <= ctlReg00       when ctlP1Selector="0000" else  
	                 iFifoRdData    when ctlP1Selector="0001" else
						  ctlP1RdCntData when ctlP1Selector="0010" else  
						  ctlReg03       when ctlP1Selector="0011" else
						  ctlClk2CntData when ctlP1Selector="0100" else
						  ctlClk1CntData when ctlP1Selector="0101" else
						  ctlReg06       when ctlP1Selector="0110" else
						  ctlReg07       when ctlP1Selector="0111" else
						  ctlReg08       when ctlP1Selector="1000" else
						  iStatus        when ctlP1Selector="1001" else
						  ctlP1RdCntData;
						  
	  -- Fifo read enable is P1 read enable (only when selected)
	  -- Fifo is not idempotent to so its enable is special
	  oFifoRdEn    <= ctlP1RdEn     when ctlP1Selector="0001" else
	                  '0';
		
	  -- P1 writes based on selector bits
	  ctlReg03WrEn <= '1' when ctlP1WrEn='1' and ctlP1Selector="0011" else
	                  '0';
	  ctlReg06WrEn <= '1' when ctlP1WrEn='1' and ctlP1Selector="0110" else
	                  '0';
	  ctlReg07WrEn <= '1' when ctlP1WrEn='1' and ctlP1Selector="0111" else
	                  '0';
							
     -- Register 0 (primary control)						
	  Reg00: process( iClk )
     begin
	     if( rising_edge(iClk) ) then
		     if ( ctlReg00WrEn='1' ) then
			      ctlReg00 <= ctlP0WrData;
			  end if;
		  end if;  
     end process;
	  
	  -- Register 1 ( Fifo Data )
	  
	  -- Register 2 ( Read counter diagnostics )
	  
	  -- Register 3 (secondary control)
	  Reg03: process( iClk )
     begin
	     if( rising_edge(iClk) ) then
		     if ( ctlReg03WrEn='1' ) then
			      ctlReg03 <= ctlP1WrData;
			  end if;
		  end if;  
     end process;
	  
	  -- Register 4 ( clock counter diag )
	  
	  -- Register 5 ( clock counter diag )

     -- Register 6 (pinc low 12 bits)
	  Reg06: process( iClk ) 
     begin
	     if( rising_edge(iClk) ) then
		     if ( ctlReg06WrEn='1' ) then
			      ctlReg06 <= ctlP1WrData;
			  end if;
		  end if;  
     end process;
	  
     -- Register 7 (pinc high 4 bits)
	  Reg07: process( iClk ) 
     begin
	     if( rising_edge(iClk) ) then
		     if ( ctlReg07WrEn='1' ) then
			      ctlReg07 <= ctlP1WrData;
			  end if;
		  end if;  
     end process;
	  
	  -- Register 8 ( version )
	  
	  -- Register 9 ( status data )
	  
	  U01: SpiSlave port map( 
	                     iMOSI => iMOSI, 
			               oMISO => oMISO, 
								iSCLK => iSCLK, 
								iSS   => iSS, 
								oBSY  => ctlBsy, 
								oSI   => ctlSi, 
								iSO   => ctlSo
								); 
								
	  U02: PortCtl port map( 
	                     iSYS_CLK    => iClk,
								iSERIAL_IN  => ctlSi, 
								oSERIAL_OUT => ctlSo, 
								iSERIAL_BSY => ctlBsy,
								
								iP0_rDATA   => ctlP0RdData,
			               oP0_rEN     => ctlP0RdEn,
								
								oP0_wDATA   => ctlP0WrData,
			               oP0_wEN     => ctlP0WrEn,
								
								iP1_rDATA   => ctlP1RdData, 
			               oP1_rEN     => ctlP1RdEn,
			  
			               oP1_wDATA   => ctlP1WrData, 
			               oP1_wEN     => ctlP1WrEn

								);
								
	  U03: Cnt16 port map(
		                  EN          => ctlP1RdEn,
			               CLK         => iClk,
		                  DOUT        => ctlP1RdCntData
		                  ); 
		
	  U04: Cnt16 port map(
		                  EN          => '1',
			               CLK         => iAltClk,
		                  DOUT        => ctlClk1CntData
		                  ); 
								
	  U05: Cnt16 port map(
		                  EN          => '1',
			               CLK         => iClk,
		                  DOUT        => ctlClk2CntData
		                  ); 

end Behavioral;
