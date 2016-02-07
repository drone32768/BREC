
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity Top is
	Port ( 
		  DCLK      : in  STD_LOGIC;
		  LEDS      : out STD_LOGIC_VECTOR( 3 downto 0 )
	);
end Top;

architecture Behavioral of Top is
  signal cnt_r : std_logic_vector(24 downto 0) := (others=>'0');
begin

   -- Counter
	process(DCLK) is
	begin
	  if rising_edge(DCLK) then
		 cnt_r <= cnt_r + 1;
	  end if;  
	end process;

	-- Flash by counter bit
	LEDS(0) <= cnt_r(23);
	LEDS(1) <= cnt_r(22);
	LEDS(2) <= cnt_r(21);
	LEDS(3) <= cnt_r(20);

end Behavioral;
