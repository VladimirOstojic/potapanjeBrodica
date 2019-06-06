-- @license MIT
-- @brief Top system for testing Hamming decoder and encoder.

library ieee;
use ieee.std_logic_1164.all;

entity Sega_Gun_Test is
	port(
		i_sw     		: in  std_logic_vector(1 downto 0);
		o_I2C_SCL		: in  std_logic;
		o_led    		: out std_logic_vector(1 downto 0);
		o_I2C_SDA    	: out std_logic
	);
end entity Sega_Gun_Test;

architecture arch_hamming_top of Sega_Gun_Test is

	signal s_swr			: std_logic;

begin
	-- Encoder instance.
	
	i_sw(0) <= o_led(0) and o_I2C_SCL;
	
	i_sw(1) <= o_led(1) and o_I2C_SDA;
	
	
	
	
	s_swr<= in_btn(0) and i_sw(0);
	
	o_led(0)<= s_swr;
	
	o_pwr <= s_swr;
	
	o_led(1)<= i_sig;
	
	
	
	-- Decoder instance.

	
end architecture arch_hamming_top;
