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
        
        reset <= '1';
        enable <= '0';
        plaintext <= x"123456ABCD132536";
        key <= "10101011011101000010000011000010011001101111001101101110";
        -- ciphertext should be x"C0B7A8D05F3A829C";
        wait for 40 ns;
        
        enable <= '1';
        wait for 40 ns;
        
        reset <= '0';
        wait for 10 ns;
        
        key <= "10101011011101000010000011000010011001101111001101101111";
        wait for 10 ns;
        
        key <= "10101011011101000010000011000010011001101111001101110000";
        wait for 10 ns;
        
        key <= "10101011011101000010000011000010011001101111001101110001";
        wait for 10 ns;
        
        key <= "10101011011101000010000011000010011001101111001101110010";
        wait for 10 ns;
        
        wait until valid = '1';
    
        wait for 10 ns;
        enable <= '0';
        wait for 40 ns;
        
        enable <= '1';
        
        wait for 1600 ns;
        
        reset <= '1';
        
        stop;
        
        wait;
    end process;


end Behavioral;
