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
-- This module is the primary digital interface block including
-- control and streaming interface.  It includes multiple 
-- internal registers and counters along with status inputs
-- physical indicator outputs and a fifo and SPI interface.
--

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
entity ControlBlock is
    Port ( 
           iMOSI        : in    STD_LOGIC_VECTOR( 1 downto 0);
           oMISO        : out   STD_LOGIC_VECTOR( 1 downto 0);
           iSCLK        : in    STD_LOGIC;
           iSS          : in    STD_LOGIC;

           oLEDS        : out   STD_LOGIC_VECTOR( 3 downto 0 );
           ioGPIO0      : inout STD_LOGIC_VECTOR( 5 downto 0 );
           ioGPIO1      : inout STD_LOGIC_VECTOR( 5 downto 0 );
           
           iClk         : in    STD_LOGIC;
           iAltClk      : in    STD_LOGIC;
           
           oFifoRst     : out   STD_LOGIC;
           iFifoRdData  : in    STD_LOGIC_VECTOR( 15 downto 0 );
           oFifoRdEn    : out   STD_LOGIC;
           iFifoThresh  : in    STD_LOGIC;
           oFifoWrInhib : out   STD_LOGIC;

           oSrcSel      : out   STD_LOGIC_VECTOR ( 3 downto 0);           
           oGenTp       : out   STD_LOGIC_VECTOR ( 3 downto 0 );
           oPinc        : out   STD_LOGIC_VECTOR ( 15 downto 0 );
           iStatus      : in    STD_LOGIC_VECTOR ( 15 downto 0 );
           
           oIoff        : out   STD_LOGIC_VECTOR( 7  downto 0 );
           oInum        : out   STD_LOGIC_VECTOR( 7  downto 0 );
           oQoff        : out   STD_LOGIC_VECTOR( 7  downto 0 );
           oQnum        : out   STD_LOGIC_VECTOR( 7  downto 0 )
    );  
end ControlBlock;

architecture Behavioral of ControlBlock is

component SpiSlave is
    Port ( 
           iDSPI         : in  STD_LOGIC;
           iMOSI         : in  STD_LOGIC_VECTOR( 1 downto 0); 
           oMISO         : out STD_LOGIC_VECTOR( 1 downto 0);
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
           
           iRD           : in   STD_LOGIC_VECTOR (15 downto 0);
           oRE           : out  STD_LOGIC;
           
           oWD           : out  STD_LOGIC_VECTOR (15 downto 0);
           oWE           : out  STD_LOGIC;
           
           oNsel         : out  STD_LOGIC_VECTOR (5 downto 0 )
           );
end component;

component Cnt16 is 
     Port (
           EN            : in   std_logic;
           CLK             : in     std_logic;
           DOUT          : out  std_logic_vector(15 downto 0)
             );
end component;

component Srg16 is 
    Port ( iClk   : in  STD_LOGIC;
           iSel   : in  STD_LOGIC_VECTOR (5 downto 0);
           iId    : in  STD_LOGIC_VECTOR (5 downto 0);
           iData  : in  STD_LOGIC_VECTOR (15 downto 0);
           oData  : out STD_LOGIC_VECTOR (15 downto 0);
           iWen   : in  STD_LOGIC
           );
end component;

component Gpio6 is
    Port ( iClk   : in     STD_LOGIC;
           ioGpio : inout  STD_LOGIC_VECTOR (5 downto 0);
           
           iDid   : in   STD_LOGIC_VECTOR (5 downto 0);
           iDsel  : in   STD_LOGIC_VECTOR (5 downto 0);
    
           iOid   : in   STD_LOGIC_VECTOR (5 downto 0);
           iOsel  : in   STD_LOGIC_VECTOR (5 downto 0);
            
           iWen   : in   STD_LOGIC;
           iWdat  : in   STD_LOGIC_VECTOR (7 downto 0);
           
           oIdat  : out  STD_LOGIC_VECTOR (7 downto 0);
           oDdat  : out  STD_LOGIC_VECTOR (7 downto 0);
           oOdat  : out  STD_LOGIC_VECTOR (7 downto 0)
           
           );
end component;
           
component Srg8 is
    Port ( iClk   : in  STD_LOGIC;
           iSel   : in  STD_LOGIC_VECTOR (5 downto 0);
           iId    : in  STD_LOGIC_VECTOR (5 downto 0);
           iData  : in  STD_LOGIC_VECTOR (7 downto 0);
           oData  : out STD_LOGIC_VECTOR (7 downto 0);
           iWen   : in  STD_LOGIC
           );
end component;

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

-- Serial input
signal ctlSi           : STD_LOGIC_VECTOR (15 downto 0);

-- Serial output
signal ctlSo           : STD_LOGIC_VECTOR (15 downto 0);

-- Bsy from serial slave interface
signal ctlBsy          : STD_LOGIC;

-- R0 = id register (RO)
signal ctlReg00        : STD_LOGIC_VECTOR (15 downto 0);

-- R1 = device and version register (RO)
signal ctlReg01        : STD_LOGIC_VECTOR (15 downto 0);

-- R2 = visual indicator register (RW)
signal ctlReg02        : STD_LOGIC_VECTOR( 7 downto 0 );

-- R3 = read counter (RO)
signal ctlReg03        : STD_LOGIC_VECTOR (15 downto 0); 

-- R4 = clock 1 counter (RO)
signal ctlReg04        : STD_LOGIC_VECTOR (15 downto 0); 

-- R5 = clock 2 counter (RO)
signal ctlReg05        : STD_LOGIC_VECTOR (15 downto 0); 

-- R6 = primary control
signal ctlReg06        : STD_LOGIC_VECTOR( 7 downto 0 );

-- R10
signal ctlReg10        : STD_LOGIC_VECTOR( 7 downto 0 );

-- R11
signal ctlReg11        : STD_LOGIC_VECTOR( 7 downto 0 );

-- R12
signal ctlReg12        : STD_LOGIC_VECTOR( 7 downto 0 );

-- R13 
signal ctlReg13        : STD_LOGIC_VECTOR( 7 downto 0 );

-- R14
signal ctlReg14        : STD_LOGIC_VECTOR( 7 downto 0 );

-- R15
signal ctlReg15        : STD_LOGIC_VECTOR( 7 downto 0 );

-- R16 = secondary control register
signal ctlReg16        : STD_LOGIC_VECTOR( 7 downto 0 );

-- R17 = PINC (least significant 8 bits)
signal ctlReg17        : STD_LOGIC_VECTOR( 7 downto 0 );

-- R18 = PINC (most significant 8 bits)
signal ctlReg18        : STD_LOGIC_VECTOR( 7 downto 0 );

-- R19 = TPG (test pattern generator)
signal ctlReg19        : STD_LOGIC_VECTOR( 7 downto 0 ); 

-- R20 = I offset
signal ctlReg20        : STD_LOGIC_VECTOR( 7 downto 0 ); 

-- R21 = I numerator
signal ctlReg21        : STD_LOGIC_VECTOR( 7 downto 0 ); 

-- R22 = Q offset
signal ctlReg22        : STD_LOGIC_VECTOR( 7 downto 0 ); 

-- R23 = Q numerator
signal ctlReg23        : STD_LOGIC_VECTOR( 7 downto 0 ); 

-- Register r/w enables, disables, and selector
signal ctlRegWd        : STD_LOGIC_VECTOR (15 downto 0);
signal ctlRegWe        : STD_LOGIC;
signal ctlRegRd        : STD_LOGIC_VECTOR (15 downto 0);
signal ctlRegRe        : STD_LOGIC;
signal ctlRegSel       : STD_LOGIC_VECTOR ( 5 downto 0 );

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
begin
 
     -- Read selector data     
     ctlRegRd    <= ctlReg00               when ctlRegSel="000000" else  
                    ctlReg01               when ctlRegSel="000001" else
                    x"00" & ctlReg02       when ctlRegSel="000010" else
                    ctlReg03               when ctlRegSel="000011" else  
                    ctlReg04               when ctlRegSel="000100" else                    
                    ctlReg05               when ctlRegSel="000101" else
                    x"00" & ctlReg06       when ctlRegSel="000110" else

                    x"00" & ctlReg10       when ctlRegSel="001010" else
                    x"00" & ctlReg11       when ctlRegSel="001011" else
                    x"00" & ctlReg12       when ctlRegSel="001100" else
                    
                    x"00" & ctlReg13       when ctlRegSel="001101" else
                    x"00" & ctlReg14       when ctlRegSel="001110" else
                    x"00" & ctlReg15       when ctlRegSel="001111" else
                    
                    x"00" & ctlReg16       when ctlRegSel="010000" else
                    x"00" & ctlReg17       when ctlRegSel="010001" else
                    x"00" & ctlReg18       when ctlRegSel="010010" else
                    x"00" & ctlReg19       when ctlRegSel="010011" else
                    
                    x"000" & 
                         "000" & iFifoThresh when ctlRegSel="111101" else
                    iStatus                when ctlRegSel="111110" else
                    iFifoRdData            when ctlRegSel="111111" else
                    x"0000";

     -- Fifo is not idempotent to so its enable is special
     oFifoRdEn        <= ctlRegRe  when ctlRegSel="111111" else '0';

     -- oLEDS = control word fields
     oLEDS(3)         <= ctlReg02(3);
     oLEDS(2)         <= ctlReg02(2);
     oLEDS(1)         <= ctlReg02(1);
     oLEDS(0)         <= ctlReg02(0); 
     
     -- Fifo Control bits
     oFifoWrInhib     <= ctlReg16(7);
     oFifoRst         <= ctlReg16(6);
         
     -- Fifo input source selected control bits
     oSrcSel(3)       <= ctlReg16(3);
     oSrcSel(2)       <= ctlReg16(2);
     oSrcSel(1)       <= ctlReg16(1);
     oSrcSel(0)       <= ctlReg16(0);
                                                   
     -- oPinc bits
     oPinc(7 downto  0) <= ctlReg17( 7 downto 0 );
     oPinc(15 downto 8) <= ctlReg18( 7 downto 0 );

     -- Generate test pattern  bits
     oGenTp(3)        <= ctlReg19(3);
     oGenTp(2)        <= ctlReg19(2);
     oGenTp(1)        <= ctlReg19(1);
     oGenTp(0)        <= ctlReg19(0);
     
     -- I/Q channel match signals
     oIoff            <= ctlReg20;
     oInum            <= ctlReg21;
     oQoff            <= ctlReg22;
     oQnum            <= ctlReg23;
                            
     -- R0 = id register (RO)
     ctlReg00         <= x"BDC1";
     
     -- R1 = device and version register (RO)
     ctlReg01         <= x"1463";

     -- R2 = visual indicator register (RW)                
     R02 : Srg8 port map ( 
                        iClk        => iClk,
                        iSel        => ctlRegSel,
                        iId         => "000010",
                        iData       => ctlRegWd(7 downto 0),
                        oData       => ctlReg02,
                        iWen        => ctlRegWe
                        );
                        
     -- R3 = read counter (RO)                     
     R03 : Cnt16 port map(
                        EN          => ctlRegRe,
                        CLK         => iClk,
                        DOUT        => ctlReg03
                        ); 
      
     -- R4 = clock 1 counter (RO)
     R04 : Cnt16 port map(
                        EN          => '1',
                        CLK         => iAltClk,
                        DOUT        => ctlReg04
                        ); 

     -- R5 = clock 2 counter (RO)                        
     R05 : Cnt16 port map(
                        EN          => '1',
                        CLK         => iClk,
                        DOUT        => ctlReg05
                        ); 

     -- R6 = primary control                        
     R06 : Srg8 port map ( 
                        iClk        => iClk,
                        iSel        => ctlRegSel,
                        iId         => "000110",
                        iData       => ctlRegWd(7 downto 0),
                        oData       => ctlReg06,
                        iWen        => ctlRegWe
                        );

     -- R10-R12                         
     R10R11R12 : Gpio6 port map ( 
                        iClk        => iClk,
                        ioGpio      => ioGPIO0,
                        iDid        => "001011",
                        iDsel       => ctlRegSel,
                        iOid        => "001100",
                        iOsel       => ctlRegSel,
                        iWen        => ctlRegWe,
                        iWdat       => ctlRegWd(7 downto 0),
                        oIdat       => ctlReg10,
                        oDdat       => ctlReg11,
                        oOdat       => ctlReg12
                        );
                              
     -- R13-R15                         
     R13R14R15 : Gpio6 port map ( 
                        iClk        => iClk,
                        ioGpio      => ioGPIO1,
                        iDid        => "001110",
                        iDsel       => ctlRegSel,
                        iOid        => "001111",
                        iOsel       => ctlRegSel,
                        iWen        => ctlRegWe,
                        iWdat       => ctlRegWd(7 downto 0),
                        oIdat       => ctlReg13,
                        oDdat       => ctlReg14,
                        oOdat       => ctlReg15
                        );
                        
     -- R16 = secondary control register
     R16 : Srg8 port map ( 
                        iClk        => iClk,
                        iSel        => ctlRegSel,
                        iId         => "010000",
                        iData       => ctlRegWd(7 downto 0),
                        oData       => ctlReg16,
                        iWen        => ctlRegWe
                        );
                        
     -- R17 = PINC (least significant 8 bits)
     R17 : Srg8 port map ( 
                        iClk        => iClk,
                        iSel        => ctlRegSel,
                        iId         => "010001",
                        iData       => ctlRegWd(7 downto 0),
                        oData       => ctlReg17,
                        iWen        => ctlRegWe
                        );

     -- R18 = PINC (most significant 8 bits)
     R18 : Srg8 port map ( 
                        iClk        => iClk,
                        iSel        => ctlRegSel,
                        iId         => "010010",
                        iData       => ctlRegWd(7 downto 0),
                        oData       => ctlReg18,
                        iWen        => ctlRegWe
                        );
    
     -- R19 = TPG (test pattern generator)
     R19 : Srg8 port map ( 
                        iClk        => iClk,
                        iSel        => ctlRegSel,
                        iId         => "010011",
                        iData       => ctlRegWd(7 downto 0),
                        oData       => ctlReg19,
                        iWen        => ctlRegWe
                        );

     -- R20 = Ioff
     R20 : Srg8 port map ( 
                        iClk        => iClk,
                        iSel        => ctlRegSel,
                        iId         => "010100",
                        iData       => ctlRegWd(7 downto 0),
                        oData       => ctlReg20,
                        iWen        => ctlRegWe
                        );               

     -- R21 = Inum
     R21 : Srg8 port map ( 
                        iClk        => iClk,
                        iSel        => ctlRegSel,
                        iId         => "010101",
                        iData       => ctlRegWd(7 downto 0),
                        oData       => ctlReg21,
                        iWen        => ctlRegWe
                        );

     -- R22 = Qoff
     R22 : Srg8 port map ( 
                        iClk        => iClk,
                        iSel        => ctlRegSel,
                        iId         => "010110",
                        iData       => ctlRegWd(7 downto 0),
                        oData       => ctlReg22,
                        iWen        => ctlRegWe
                        );

     -- R23 = Qnum
     R23 : Srg8 port map ( 
                        iClk        => iClk,
                        iSel        => ctlRegSel,
                        iId         => "010111",
                        iData       => ctlRegWd(7 downto 0),
                        oData       => ctlReg23,
                        iWen        => ctlRegWe
                        );
                        
     U01: SpiSlave port map( 
                        iDSPI       => ctlReg06(0), -- '0',
                        iMOSI       => iMOSI, 
                        oMISO       => oMISO, 
                        iSCLK       => iSCLK, 
                        iSS         => iSS, 
                        oBSY        => ctlBsy, 
                        oSI         => ctlSi, 
                        iSO         => ctlSo
                        ); 
                        
     U02: PortCtl port map( 
                        iSYS_CLK    => iClk,
                        iSERIAL_IN  => ctlSi, 
                        oSERIAL_OUT => ctlSo, 
                        iSERIAL_BSY => ctlBsy,

                        iRD         => ctlRegRd,
                        oRE         => ctlRegRe,
                        oWD         => ctlRegWd,
                        oWE         => ctlRegWe,
                        oNsel       => ctlRegSel
                        );

end Behavioral;
