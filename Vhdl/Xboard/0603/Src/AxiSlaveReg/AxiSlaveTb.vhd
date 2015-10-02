
library STD;
use STD.textio.all;                     -- basic I/O
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
use IEEE.std_logic_textio.all;          -- I/O for logic types
 
ENTITY AxiSlaveTb IS
END AxiSlaveTb;
 
ARCHITECTURE behavior OF AxiSlaveTb IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT AxiSlaveReg
    PORT(
         iInWord : IN  std_logic_vector(15 downto 0);
         iWrEn : IN  std_logic;
         iAclk : IN  std_logic;
         iArdy : IN  std_logic;
         oAvalid : OUT  std_logic;
         oAdata : OUT  std_logic_vector(15 downto 0)
        );
    END COMPONENT;
    

   --Inputs
   signal iInWord : std_logic_vector(15 downto 0) := (others => '0');
   signal iWrEn : std_logic := '0';
   signal iAclk : std_logic := '0';
   signal iArdy : std_logic := '0';

 	--Outputs
   signal oAvalid : std_logic;
   signal oAdata : std_logic_vector(15 downto 0);

   -- Clock period definitions
   constant iAclk_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: AxiSlaveReg PORT MAP (
          iInWord => iInWord,
          iWrEn => iWrEn,
          iAclk => iAclk,
          iArdy => iArdy,
          oAvalid => oAvalid,
          oAdata => oAdata
        );

   -- Clock process definitions
   iAclk_process :process
   begin
		iAclk <= '0';
		wait for iAclk_period/2;
		iAclk <= '1';
		wait for iAclk_period/2;
   end process;
 

   -- Stimulus process
   stim_proc: process
	   variable lineBf   : line;
   begin		
      -- hold reset state for 100 ns.
      wait for 100 ns;	

      wait for iAclk_period*10;

      -- insert stimulus here 
		write(lineBf,"AxiSlaveTb00: Test Begin");
		writeline(output,lineBf);

		write(lineBf,"AxiSlaveTb00: Test Done");
		writeline(output,lineBf);
		
		iArdy <= '1';
		iWrEn <= '1';
		write(lineBf,"AxiSlaveTb00: Ardy and WrEn fixed to true.");
		writeline(output,lineBf);
		
		iInWord <= x"00ff";
		write(lineBf,", iInWord = ");
      hwrite(lineBf, iInWord );
		
		wait for iAclk_period*10;
				
      wait;
   end process;

END;
