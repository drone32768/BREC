--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   06:56:02 06/03/2015
-- Design Name:   
-- Module Name:   C:/Users/user/Desktop/Jim/Rf/_140-XuLA2-LX9/ISE/SPI-00/SpiSlaveTb00.vhd
-- Project Name:  SPI-00
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: SpiSlave
-- 
-- Dependencies:
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
--
-- Notes: 
-- This testbench has been automatically generated using types std_logic and
-- std_logic_vector for the ports of the unit under test.  Xilinx recommends
-- that these types always be used for the top-level I/O of a design in order
-- to guarantee that the testbench will bind correctly to the post-implementation 
-- simulation model.
--------------------------------------------------------------------------------


library STD;
use STD.textio.all;                     -- basic I/O
library IEEE;
use IEEE.std_logic_1164.all;            -- basic logic types
use IEEE.std_logic_textio.all;          -- I/O for logic types
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY SpiSlaveTb00 IS
END SpiSlaveTb00;
 
ARCHITECTURE behavior OF SpiSlaveTb00 IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT SpiSlave
    PORT(
         MOSI : IN  std_logic;
         MISO : OUT  std_logic;
         SCLK : IN  std_logic;
         SS : IN  std_logic;
         BSY : OUT  std_logic;
         SI : OUT  std_logic_vector(15 downto 0);
         SO : IN  std_logic_vector(15 downto 0)
        );
    END COMPONENT;
    

   --Inputs
   signal MOSI : std_logic := '0';
   signal SCLK : std_logic := '0';
   signal SS   : std_logic := '0';
   signal SO   : std_logic_vector(15 downto 0) := (others => '0');

 	--Outputs
   signal MISO : std_logic;
   signal BSY : std_logic;
   signal SI : std_logic_vector(15 downto 0);

   -- Test registers
	signal masterTx : STD_LOGIC_VECTOR( 15 downto 0 ) ; 
	signal slaveTx  : STD_LOGIC_VECTOR( 15 downto 0 ) ;
	signal masterRx : STD_LOGIC_VECTOR( 15 downto 0 ) ;
   signal slaveRx  : STD_LOGIC_VECTOR( 15 downto 0 ) ;
		  
   -- Clock period definitions
   constant SCLK_period : time := 100 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: SpiSlave PORT MAP (
          MOSI => MOSI,
          MISO => MISO,
          SCLK => SCLK,
          SS => SS,
          BSY => BSY,
          SI => SI,
          SO => SO
        );

   -- Stimulus process
   stim_proc: process
		  variable lineBf   : line;
   begin	
	   slaveTx  <= "0000000000000001"; -- "0011001100110011"; 
		masterTx <= "1100110011001100";
		slaveRx  <= "0000000000000000";
		masterRx <= "0000000000000000";
		wait for 100 ns;
		
		SCLK <= '0';
      SS   <= '1';	
   	SO   <= slaveTx;
		wait for 100 ns;	
		
		write(lineBf,"SPI Test Start");
		writeline(output,lineBf);
		
		write(lineBf,"master tx = ");
      hwrite(lineBf, masterTx );
		writeline(output,lineBf);
		
		write(lineBf,"slave  tx = ");
      hwrite(lineBf, slaveTx );
		writeline(output,lineBf);
			  
		SS   <= '0';
		wait for SCLK_period;
		for b in 1 to 16 loop
		    MOSI     <= masterTx(15);
			 masterTx <= masterTx( 14 downto 0 ) & '0';
			 wait for SCLK_period/2;

			 SCLK <= '1';
			 wait for SCLK_period/2;
			 
			 masterRx <= masterRx ( 14 downto 0 )& MISO;
			 wait for SCLK_period/2;
						 
			 SCLK <= '0';
			 wait for SCLK_period/2;
			 
		end loop;
		SS <= '1';
		wait for SCLK_period;
		
      slaveRx <= SI;
		wait for 100 ns;
		
      write(lineBf, "slave  rx = ");
      hwrite(lineBf,slaveRx);
		writeline(output,lineBf);
		
		write(lineBf, "master rx = ");
		hwrite(lineBf,masterRx);
		writeline(output, lineBf);
		
		write(lineBf,"SPI Test End");
		writeline(output,lineBf);

      -- Done
      wait;
		
   end process;

END;
