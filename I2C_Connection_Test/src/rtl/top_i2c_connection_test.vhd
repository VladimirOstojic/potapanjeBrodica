-- @license MIT
-- @brief Top system for testing Hamming decoder and encoder.

library ieee;
use ieee.std_logic_1164.all;

entity top_i2c_connection_test is
	port(
		i_sw     		: in  std_logic_vector(1 downto 0);
		o_i2c_scl		: out  std_logic;
		o_led    		: out std_logic_vector(1 downto 0);
		o_i2c_sda    	: out std_logic
	);
end entity top_i2c_connection_test;

architecture arch_hamming_top of top_i2c_connection_test is

	signal s_swr			: std_logic;

begin
	-- Encoder instance.
	
	
	o_led(0) <= '1' when i_sw(0)='1' else '0';
	o_led(1) <= '1' when i_sw(1)='1' else '0';
	
	o_i2c_scl <= '1' when i_sw(0) = '1' else '0';
	o_i2c_sda <= '1' when i_sw(0) = '1' else '0';
	
	
	
	
	
	
	-- Decoder instance.

	
end architecture arch_hamming_top;
