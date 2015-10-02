--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   19:41:11 07/21/2015
-- Design Name:   
-- Module Name:   C:/Users/user/Desktop/Jim/Rf/_140-XuLA2-LX9/ISE/04-Fifo/PortCtlTb00.vhd
-- Project Name:  Fifo
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: PortCtl
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
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
use IEEE.std_logic_textio.all;          -- I/O for logic types
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY PortCtlTb00 IS
END PortCtlTb00;
 
ARCHITECTURE behavior OF PortCtlTb00 IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT PortCtl
    PORT(
         iSYS_CLK : IN  std_logic;
         iSERIAL_IN : IN  std_logic_vector(15 downto 0);
         oSERIAL_OUT : OUT  std_logic_vector(15 downto 0);
         iSERIAL_BSY : IN  std_logic;
         iP0_rDATA : IN  std_logic_vector(15 downto 0);
         oP0_rEN : OUT  std_logic;
         oP0_wDATA : OUT  std_logic_vector(15 downto 0);
         oP0_wEN : OUT  std_logic;
         iP1_rDATA : IN  std_logic_vector(15 downto 0);
         oP1_rEN : OUT  std_logic;
         oP1_wDATA : OUT  std_logic_vector(15 downto 0);
         oP1_wEN : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal iSYS_CLK : std_logic := '0';
   signal iSERIAL_IN : std_logic_vector(15 downto 0) := (others => '0');
   signal iSERIAL_BSY : std_logic := '0';
   signal iP0_rDATA : std_logic_vector(15 downto 0) := (others => '0');
   signal iP1_rDATA : std_logic_vector(15 downto 0) := (others => '0');

 	--Outputs
   signal oSERIAL_OUT : std_logic_vector(15 downto 0);
   signal oP0_rEN : std_logic;
   signal oP0_wDATA : std_logic_vector(15 downto 0);
   signal oP0_wEN : std_logic;
   signal oP1_rEN : std_logic;
   signal oP1_wDATA : std_logic_vector(15 downto 0);
   signal oP1_wEN : std_logic;

   -- Clock period definitions
   constant iSYS_CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: PortCtl PORT MAP (
          iSYS_CLK => iSYS_CLK,
          iSERIAL_IN => iSERIAL_IN,
          oSERIAL_OUT => oSERIAL_OUT,
          iSERIAL_BSY => iSERIAL_BSY,
          iP0_rDATA => iP0_rDATA,
          oP0_rEN => oP0_rEN,
          oP0_wDATA => oP0_wDATA,
          oP0_wEN => oP0_wEN,
          iP1_rDATA => iP1_rDATA,
          oP1_rEN => oP1_rEN,
          oP1_wDATA => oP1_wDATA,
          oP1_wEN => oP1_wEN
        );

   -- Clock process definitions
   iSYS_CLK_process :process
   begin
		iSYS_CLK <= '0';
		wait for iSYS_CLK_period/2;
		iSYS_CLK <= '1';
		wait for iSYS_CLK_period/2;
   end process;
	
   -- Stimulus process
   stim_proc: process
	   variable lineBf   : line;
   begin		
	
      -- hold reset state for 100 ns.
      wait for 100 ns;	
		report("Port Controller Test Start");
		
		write(lineBf,"Port Controller Test Start");
		writeline(output,lineBf);
		iSERIAL_BSY <= '0';
		
		-- Write Port 0
		write(lineBf,"############# P0 write");
		writeline(output,lineBf);
		
		iSERIAL_IN  <= "1100110011000100";
		write(lineBf,"Write = ");
		hwrite(lineBf,iSERIAL_IN);
		writeline(output,lineBf);
		iSERIAL_BSY <= '1';
		wait for iSYS_CLK_period * 16;
	   iSERIAL_BSY <= '0';
	   wait for iSYS_CLK_period * 16;
	
		write(lineBf,">> oP0_wDATA = ");
      hwrite(lineBf,  oP0_wDATA );	
		write(lineBf,", oSERIAL_OUT = ");
      hwrite(lineBf, oSERIAL_OUT );
      writeline(output,lineBf);
		
		write(lineBf,"Controller Test Done");
		writeline(output,lineBf);
		
		-- Read Port 0
		write(lineBf,"############# P0 read");
		writeline(output,lineBf);
		
		iP0_rDATA   <= "1010101010101010";
		iSERIAL_IN  <= "0000000000001000";
		write(lineBf,"Read = ");
		hwrite(lineBf,iSERIAL_IN);
		writeline(output,lineBf);
		iSERIAL_BSY <= '1';
		wait for iSYS_CLK_period * 16;
	   iSERIAL_BSY <= '0';
	   wait for iSYS_CLK_period * 16;
	
		write(lineBf,">> oP0_wDATA = ");
      hwrite(lineBf,  oP0_wDATA );	
		write(lineBf,", oSERIAL_OUT = ");
      hwrite(lineBf,  oSERIAL_OUT );
      writeline(output,lineBf);
		
		write(lineBf,"Controller Test Done");
		writeline(output,lineBf);
		
      -- done
      wait;
   end process;


END;
