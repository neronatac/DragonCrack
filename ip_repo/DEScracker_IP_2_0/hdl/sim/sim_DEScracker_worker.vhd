----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 13.02.2024 21:44:27
-- Design Name: 
-- Module Name: sim_DEScracker_worker - Behavioral
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

entity sim_DEScracker_worker is
--  Port ( );
end sim_DEScracker_worker;

architecture Behavioral of sim_DEScracker_worker is

    signal reset : STD_LOGIC;
    signal clk : STD_LOGIC;
    signal enable : STD_LOGIC;
    signal plaintext : STD_LOGIC_VECTOR (63 downto 0);
    signal start_key : STD_LOGIC_VECTOR (55 downto 0);
    signal end_key : STD_LOGIC_VECTOR (55 downto 0);
    signal mask1 : STD_LOGIC_VECTOR (63 downto 0);
    signal mask2 : STD_LOGIC_VECTOR (63 downto 0);
    signal ref1 : STD_LOGIC_VECTOR (63 downto 0);  -- ref1 must be coherent with mask1 (already masked itself)
    signal ref2 : STD_LOGIC_VECTOR (63 downto 0);  -- ref2 must be coherent with mask2 (already masked itself)
    signal read_result : STD_LOGIC;
    signal current_key : STD_LOGIC_VECTOR (55 downto 0);
    signal ended : STD_LOGIC;
    signal result_available : STD_LOGIC;
    signal result_full : STD_LOGIC;
    signal match_out : STD_LOGIC; -- 0 for match1, 1 for match2
    signal key_out : STD_LOGIC_VECTOR (55 downto 0);

begin

    dut: entity work.DEScracker_worker
    port map (
        reset => reset,
        clk => clk,
        enable => enable,
        plaintext => plaintext,
        start_key => start_key,
        end_key => end_key,
        mask1 => mask1,
        mask2 => mask2,
        ref1 => ref1,
        ref2 => ref2,
        read_result => read_result,
        
        current_key => current_key,
        ended => ended,
        result_available => result_available,
        result_full => result_full,
        match_out => match_out,
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
        -- TEST FOR A SINGLE MATCH
        
        plaintext <= x"123456ABCD132536";
        -- valid key: 10101011011101000010000011000010011001101111001101101110
        start_key <= "10101011011101000010000011000010011001101111001101101010";
        end_key <=   "10101011011101000010000011000010011001101111001101110000";
        -- valid ciphertext: C0B7A8D05F3A829C
        mask1 <=           x"FFFF000000000000";
        ref1 <=            x"C0B7000000000000";
        mask2 <=           x"0000000000000000";
        ref2 <=            x"FFFFFFFFFFFFFFFF";
        
        read_result <= '0';
        enable <= '0';
        reset <= '1';
        
        -- stalled because of reset
        wait for 40 ns;
        
        -- worker waits 
        reset <= '0';
        wait for 40 ns;
        
        -- launch and stop the worker with enable signal
        enable <= '1';
        wait for 20 ns;
        enable <= '0';
        wait for 20 ns;
        enable <= '1';
        
        -- find a result, worker is blocked
        wait until result_available = '1';
        wait for 100 ns;
        
        -- read the result
        read_result <= '1';
        wait for 10 ns;
        read_result <= '0';
        
        -- continue cracking until the end
        wait until ended = '1';
        
        -- worker waits
        wait for 400 ns;
        
        -- TEST FOR SUCCESSIVE MATCHES
        -- disable
        enable <= '0';
        wait for 20 ns;
        
        -- setup
        start_key <= "00000000000000000000000000000000000000000000000000000000";
        end_key <=   "00000000000000000000000000000000000000000000000000010100";
        mask1 <=           x"0000000000000000";
        ref1 <=            x"0000000000000000";
        mask2 <=           x"0000000000000000";
        ref2 <=            x"FFFFFFFFFFFFFFFF";
        
        -- reset
        wait for 20 ns;
        reset <= '1';
        wait for 20 ns;
        
        -- go
        reset <= '0';
        enable <= '1';
        wait until result_available = '1';
        
        while ended = '0' loop
            wait for 40 ns;
            -- read the result
            read_result <= '1';
            wait for 10 ns;
            read_result <= '0';
        end loop;
        
        wait for 100 ns;
        
        stop;
        
        wait;
    end process;


end Behavioral;
