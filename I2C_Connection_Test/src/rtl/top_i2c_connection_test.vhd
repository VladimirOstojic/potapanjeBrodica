-- @license MIT
-- @brief Top system for testing Hamming decoder and encoder.

library ieee;
use ieee.std_logic_1164.all;

entity top_i2c_connection_test is
	port(
		i_sw     		: in  std_logic_vector(1 downto 0);
		o_i2c_scl		: in  std_logic;
		o_led    		: out std_logic_vector(1 downto 0);
		o_i2c_sda    	: out std_logic
	);
end entity top_i2c_connection_test;

architecture arch_hamming_top of top_i2c_connection_test is

	signal s_swr			: std_logic;

begin
	-- Encoder instance.
	
	i_sw(0) <= o_led(0) and o_i2c_scl;
	
	i_sw(1) <= o_led(1) and o_i2c_sda;
	
	
	o_led(0)<= s_swr;
	
	
	-- Decoder instance.

	
end architecture arch_hamming_top;
