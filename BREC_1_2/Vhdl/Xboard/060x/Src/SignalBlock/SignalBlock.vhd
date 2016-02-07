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

--000000001111111111222222222233333333334444444444555555555566666666667777777777
--234567890123456789012345678901234567890123456789012345678901234567890123456789	
--
--                        I_s24      I_s48      I_s16
--           S_s12 --->MUL----->CIC00---->CFIR00---->
-- Tpg            |    ^                             |
--  |             |    |                             |
--  |         -   |    | ncoData(27:16)             ----
--   ------->| |  |    |                           | IQ | IqData
--  Tp_s12   | |--+    LO <---- pinc (16)          |    |----->
--  -------->| |  |    |                           | wr |
--  iAdcDat   -   |    | ncoData(11:0)              ----
--                |    |                             |
--                |    v                             |
--           S_s12 --->MUL----->CIC00---->CFIR00---->
--                        Q_s24      Q_s48      Q_s16         
--
entity SignalBlock is
    Port ( 
			  iClk       : in  STD_LOGIC;
           iAdcDat    : in  STD_LOGIC_VECTOR (11 downto 0);
			  
			  iFifoSel   : in  STD_LOGIC_VECTOR (3 downto 0);
			  oFifoData  : out STD_LOGIC_VECTOR (15 downto 0);
			  oFifoWrEn  : out STD_LOGIC;
			  iFifoWrGate: in  STD_LOGIC;
			  
			  iGenTp     : in  STD_LOGIC_VECTOR( 3  downto 0 );
			  iPinc      : in  STD_LOGIC_VECTOR( 15 downto 0 );
			  oStatus    : out STD_LOGIC_VECTOR( 15 downto 0 )
	 );
end SignalBlock;

architecture Behavioral of SignalBlock is

-- The following is from the generated core instantiation template
COMPONENT QuadLo
  PORT (
           aclk                 : IN  STD_LOGIC;
           s_axis_config_tvalid : IN  STD_LOGIC;
           s_axis_config_tdata  : IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
           m_axis_data_tvalid   : OUT STD_LOGIC;
           m_axis_data_tdata    : OUT STD_LOGIC_VECTOR(31 DOWNTO 0)
  );
END COMPONENT;

-- The following is from the generated core instantiation template
COMPONENT Cic00
  PORT (
           aclk               : IN STD_LOGIC;
	        aresetn            : IN STD_LOGIC;
           s_axis_data_tdata  : IN STD_LOGIC_VECTOR(23 DOWNTO 0);
           s_axis_data_tvalid : IN STD_LOGIC;
           s_axis_data_tready : OUT STD_LOGIC;
           m_axis_data_tdata  : OUT STD_LOGIC_VECTOR(47 DOWNTO 0);
           m_axis_data_tvalid : OUT STD_LOGIC
  );
END COMPONENT;

-- The following is from the generated core instantiation template
COMPONENT CFIR00
  PORT (
           aresetn            : IN  STD_LOGIC;
           aclk               : IN  STD_LOGIC;
           s_axis_data_tvalid : IN  STD_LOGIC;
           s_axis_data_tready : OUT STD_LOGIC;
           s_axis_data_tdata  : IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
           m_axis_data_tvalid : OUT STD_LOGIC;
           m_axis_data_tdata  : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)
  );
END COMPONENT;

Component Tpg is
    Port ( 
	        iClk          : in  STD_LOGIC;
           iTpg          : in  STD_LOGIC_VECTOR (3 downto 0);
           oTst          : out SIGNED ( 11 downto 0 )
			);
end component;

Component AxiSlaveReg is
    Port ( 
	        iInWord       : in   STD_LOGIC_VECTOR (15 downto 0);
	        iWrEn         : in   STD_LOGIC;
           iAclk         : in   STD_LOGIC;
           iArdy         : in   STD_LOGIC;
           oAvalid       : out  STD_LOGIC;
           oAdata        : out  STD_LOGIC_VECTOR (15 downto 0)
			);
end component;

Component IqWriter is
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
end component;

signal ncoCfgValid      : STD_LOGIC;
signal ncoCfg           : STD_LOGIC_VECTOR (15 downto 0);

signal ncoDataValid     : STD_LOGIC;
signal ncoData          : STD_LOGIC_VECTOR (31 downto 0);

signal S_s12            : signed ( 11 downto 0 );
signal Tp_s12           : signed ( 11 downto 0 );
signal S_v12            : STD_LOGIC_VECTOR  (11 downto 0);

signal I_s24            : signed ( 23 downto 0 );
signal Q_s24            : signed ( 23 downto 0 );

signal I_v24            : STD_LOGIC_VECTOR ( 23 downto 0 );
signal Q_v24            : STD_LOGIC_VECTOR ( 23 downto 0 );

signal I_v48            : STD_LOGIC_VECTOR ( 47 downto 0 );
signal Q_v48            : STD_LOGIC_VECTOR ( 47 downto 0 );
signal Q_v48Valid       : STD_LOGIC;
signal I_v48Valid       : STD_LOGIC;

signal I_v16            : STD_LOGIC_VECTOR ( 15 downto 0 );
signal Q_v16            : STD_LOGIC_VECTOR ( 15 downto 0 );
signal Q_v16Valid       : STD_LOGIC;
signal I_v16Valid       : STD_LOGIC;

signal IqData           : STD_LOGIC_VECTOR ( 15 downto 0 );
signal IqWrEn           : STD_LOGIC;

signal resetn           : STD_LOGIC;
signal pincCount        : unsigned ( 15 downto 0 ) := "0000000000000000";

begin
	 I_v24   <= std_logic_vector( I_s24 );
	 Q_v24   <= std_logic_vector( Q_s24 );
	 S_v12   <= std_logic_vector( S_s12 );
	 oStatus <= std_logic_vector( pincCount ); -- TODO change to needed status
	    
	 -- what gets written to fifo is determined by selector
	 oFifoData   <=  "0000" & iAdcDat            when iFifoSel="0000" else
	                 ncoData(31 downto 16)       when iFifoSel="0001" else
                    ncoData(15 downto 0 )       when iFifoSel="0010" else
						  I_v24  (23 downto 8 )       when iFifoSel="0011" else
						  Q_v24  (23 downto 8 )       when iFifoSel="0100" else
						  I_v48  (47 downto 32)       when iFifoSel="0101" else
						  Q_v48  (47 downto 32)       when iFifoSel="0110" else
						  IqData                      when iFifoSel="0111" else
						  I_v16                       when iFifoSel="1000" else
						  Q_v16                       when iFifoSel="1001" else
						  S_v12 & "0000";
						  
    oFifoWrEn   <=  iFifoWrGate                 when iFifoSel="0000" else
		              iFifoWrGate                 when iFifoSel="0001" else
						  iFifoWrGate                 when iFifoSel="0010" else
						  iFifoWrGate                 when iFifoSel="0011" else
						  iFifoWrGate                 when iFifoSel="0100" else
						  Q_v48Valid and iFifoWrGate  when iFifoSel="0101" else
						  I_v48Valid and iFifoWrGate  when iFifoSel="0110" else
						  IqWrEn                      when iFifoSel="0111" else
						  Q_v16Valid and iFifoWrGate  when iFifoSel="1000" else
						  I_v16Valid and iFifoWrGate  when iFifoSel="1001" else						  
						  iFifoWrGate;			  					  

	 format: process( iClk )
    begin
	 	if( rising_edge(iClk) ) then
	     if( iGenTp = "0000" ) then 
           -- Convert adc input to signed 12 bit		  
			  S_s12 <= signed( unsigned(iAdcDat) - 2048 );					  
		  else
		     -- Use test pattern
			  S_s12 <= Tp_s12;		  
        end if;		  
		end if;		  
    end process;
	 
	 mulIQ: process( iClk )
	 begin
	     if( rising_edge(iClk) ) then
		     I_s24 <=  S_s12 * signed( ncoData( 27 downto 16 ) );
			  Q_s24 <=  S_s12 * signed( ncoData( 11 downto 0 ) );
		  end if;
	 end process;

    resets: process ( iClk, iFifoWrGate ) -- sync reset to clock
    begin
	     if( rising_edge(iClk) ) then
		     resetn <= iFifoWrGate;
		  end if;
    end process;
	 
	 pincCounter: process ( iClk, ncoCfgValid ) -- verify
	 begin
	 	  if( rising_edge(iClk) ) then
		     if ( ncoCfgValid='1' ) then
		         pincCount <= pincCount + 1;
			  end if;
		  end if;
	 end process;
	 
    U01:  AxiSlaveReg port map(
	       iInWord     => iPinc,
	       iWrEn       => '1',
			 iAclk       => iClk,
			 iArdy       => '1', 
			 oAvalid     => ncoCfgValid,
			 oAdata      => ncoCfg
    );
	 
	 U02:  IqWriter port map (
			 iClk        => iClk,
			 iIdata      => I_v16,      -- I_v48 (47 downto 32),
			 iIvalid     => I_v16Valid, -- I_v48Valid,
			 iQdata      => Q_v16,      -- Q_v48 (47 downto 32),
			 iQValid     => Q_v16Valid, -- Q_v48Valid,
			 iWrGate     => iFifoWrGate,
			 oFifoWrData => IqData,
			 oFifoWrEn   => IqWrEn,
		    iTp         => iGenTp(0)
	 );

	 U03:  Tpg port map (
			 iClk        => iClk,
			 iTpg        => iGenTp,
			 oTst        => Tp_s12
	 );
	 
    U103 : Cic00
        PORT MAP (
          aclk               => iClk,
			 aresetn            => resetn,
          s_axis_data_tdata  => Q_v24,
          s_axis_data_tvalid => '1',
          s_axis_data_tready => open,
          m_axis_data_tdata  => Q_v48,
          m_axis_data_tvalid => Q_v48Valid
        );

    U104 : Cic00
        PORT MAP (
          aclk               => iClk,
			 aresetn            => resetn,
          s_axis_data_tdata  => I_v24,
          s_axis_data_tvalid => '1',
          s_axis_data_tready => open,
          m_axis_data_tdata  => I_v48,
          m_axis_data_tvalid => I_v48Valid
        );
		  
    U105 : CFIR00
       PORT MAP (
          aresetn            => resetn,
          aclk               => iClk,
          s_axis_data_tvalid => I_v48Valid,
          s_axis_data_tready => open,
          s_axis_data_tdata  => I_v48 (47 downto 32 ),
          m_axis_data_tvalid => I_v16Valid,
          m_axis_data_tdata  => I_v16
      );

    U106 : CFIR00
       PORT MAP (
          aresetn            => resetn,
          aclk               => iClk,
          s_axis_data_tvalid => Q_v48Valid,
          s_axis_data_tready => open,
          s_axis_data_tdata  => Q_v48 (47 downto 32 ),
          m_axis_data_tvalid => Q_v16Valid,
          m_axis_data_tdata  => Q_v16
      );
		
    U102 : QuadLo
       PORT MAP (
          aclk                 => iClk,
          s_axis_config_tvalid => ncoCfgValid,
          s_axis_config_tdata  => ncoCfg,
          m_axis_data_tvalid   => ncoDataValid,
          m_axis_data_tdata    => ncoData
       );	 

end Behavioral;

