----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 07.02.2024 20:38:40
-- Design Name: 
-- Module Name: sim_DES - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use std.env.stop;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity sim_DES is
--  Port ( );
end sim_DES;

architecture Behavioral of sim_DES is

    signal plaintext : STD_LOGIC_VECTOR (0 to 63);
    signal key : STD_LOGIC_VECTOR (0 to 55);
    signal clk : STD_LOGIC;
    signal reset : STD_LOGIC;
    signal enable : STD_LOGIC;
    signal valid : STD_LOGIC;
    signal ciphertext : STD_LOGIC_VECTOR (0 to 63);
    signal key_out : STD_LOGIC_VECTOR (0 to 55);

begin

    dut: entity work.DES(arch1)
    port map(
        plaintext => plaintext,
        key => key,
        clk => clk,
        reset => reset,
        enable => enable,
        valid => valid,
        ciphertext => ciphertext,
        key_out => key_out
    );
    
    -- clock generation
    process
    begin
        clk <= '1';
        wait for 5 ns;
        clk <= '0';
        wait for 5 ns;
    end process;
    
    -- tests
    process
    begin
        enable <= '1';
        reset <= '1';
        plaintext <= x"123456ABCD132536";
        key <= "10101011011101000010000011000010011001101111001101101110";
        wait for 500 ns;
        
        assert valid = '0';
        assert ciphertext = x"0000000000000000";
        assert key_out = x"00000000000000";
        
        wait for 10 ns;
        reset <= '0';
        enable <= '0';
        wait for 100 ns;
        
        enable <= '1';
        
        wait until valid = '1';
        assert ciphertext = x"C0B7A8D05F3A829C";
        assert key_out = "10101011011101000010000011000010011001101111001101101110";
    
        plaintext <= x"0123456789abcdef";
        key <= "00010010011010010101101111001001101101111011011111111000";
        wait for 500 ns;
        
        assert ciphertext = x"C0B7A8D05F3A829C";
        assert valid = '1';
        assert key_out = "10101011011101000010000011000010011001101111001101101110";
        
        wait for 10 ns;
        enable <= '1';
        
        wait for 500 ns;
        assert ciphertext = x"85e813540f0ab405";
        assert valid = '1';
        assert key_out = "00010010011010010101101111001001101101111011011111111000";
        
        wait for 20 ns;
        
        reset <= '1';
        assert valid = '0';
        assert ciphertext = x"0000000000000000";
        assert key_out = x"00000000000000";
        
        stop;
        
        wait;
    end process;


end Behavioral;
