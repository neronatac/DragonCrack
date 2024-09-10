----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 04.02.2024 21:18:18
-- Design Name: 
-- Module Name: DES - Arch1
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

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity DES is
    Port ( plaintext : in STD_LOGIC_VECTOR (0 to 63);
           key : in STD_LOGIC_VECTOR (0 to 55);
           clk : in STD_LOGIC;
           reset: in STD_LOGIC;
           enable : in STD_LOGIC;
           valid : out STD_LOGIC;
           ciphertext : out STD_LOGIC_VECTOR (0 to 63);
           key_out : out STD_LOGIC_VECTOR (0 to 55));
end DES;

architecture Arch1 of DES is
    -- Arch1: straightforward (all combinational, 1 latch at the output)

    type ip_table_t is array(0 to 63) of natural;
    constant IP_TABLE: ip_table_t := (
        58, 50, 42, 34, 26, 18, 10, 2,
        60, 52, 44, 36, 28, 20, 12, 4,
        62, 54, 46, 38, 30, 22, 14, 6,
        64, 56, 48, 40, 32, 24, 16, 8,
        57, 49, 41, 33, 25, 17, 9,  1,
        59, 51, 43, 35, 27, 19, 11, 3,
        61, 53, 45, 37, 29, 21, 13, 5,
        63, 55, 47, 39, 31, 23, 15, 7
    );
    
    constant FP_TABLE: ip_table_t := (
        40, 8, 48, 16, 56, 24, 64, 32,
        39, 7, 47, 15, 55, 23, 63, 31,
        38, 6, 46, 14, 54, 22, 62, 30,
        37, 5, 45, 13, 53, 21, 61, 29,
        36, 4, 44, 12, 52, 20, 60, 28,
        35, 3, 43, 11, 51, 19, 59, 27,
        34, 2, 42, 10, 50, 18, 58, 26,
        33, 1, 41, 9,  49, 17, 57, 25
    );
    
    type pc1_table_t is array(0 to 55) of natural;
    constant PC1_TABLE: pc1_table_t := (
        57, 49, 41, 33, 25, 17, 9,
        1,  58, 50, 42, 34, 26, 18,
        10, 2,  59, 51, 43, 35, 27,
        19, 11, 3,  60, 52, 44, 36,
        63, 55, 47, 39, 31, 23, 15,
        7,  62, 54, 46, 38, 30, 22,
        14, 6,  61, 53, 45, 37, 29,
        21, 13, 5,  28, 20, 12, 4
    );
    
    type pc2_table_t is array(0 to 47) of natural;
    constant PC2_TABLE: pc2_table_t := (
        14, 17, 11, 24, 1,  5,  3,  28,
        15, 6,  21, 10, 23, 19, 12, 4,
        26, 8,  16, 7,  27, 20, 13, 2,
        41, 52, 31, 37, 47, 55, 30, 40,
        51, 45, 33, 48, 44, 49, 39, 56,
        34, 53, 46, 42, 50, 36, 29, 32
    );
    
    type rot_table_t is array(0 to 15) of natural;
    constant ROT_TABLE: rot_table_t := (
        1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1
    );
    
    signal ip_out, fp_in, fp_out: std_logic_vector(0 to 63);
    
    type semi_state_t is array(0 to 15) of std_logic_vector(0 to 31);
    signal left_parts, right_parts: semi_state_t;
    
    type f_outs_t is array(0 to 15) of std_logic_vector(0 to 31);
    signal f_outs: f_outs_t;
    
    type subkeys_t is array(0 to 15) of std_logic_vector(0 to 47);
    signal subkeys: subkeys_t;
    
    signal key_with_parity: std_logic_vector(0 to 63);
    signal pc1_out: std_logic_vector(0 to 55);
    
    type rotated_t is array(0 to 15) of std_logic_vector(0 to 27);
    signal rotated_left, rotated_right: rotated_t;
    
    type pc2_inputs_t is array(0 to 15) of std_logic_vector(0 to 55);
    signal pc2_inputs: pc2_inputs_t;
    
    signal key_sig : std_logic_vector(0 to 55);
    signal valid_sig : std_logic;

begin

    ------------------------------------------------
    -- Output signals
    ------------------------------------------------
    key_out <= key_sig;
    ciphertext <= fp_out;
    valid <= valid_sig;
    
    process(clk, reset)
    begin
        if reset = '1' then
            valid_sig <= '0';
            key_sig <= (others => '0');
        elsif rising_edge(clk) then
            if enable = '1' then
                valid_sig <= '1';
                key_sig <= key;
            else
                valid_sig <= valid_sig;
                key_sig <= key_sig;
            end if;
        end if;
    end process;

    ------------------------------------------------
    -- Cipher algorithm
    ------------------------------------------------

    -- Initial Permutation
    ip_gen: for i in 0 to 63 generate
        ip_out(i) <= plaintext(IP_TABLE(i)-1);
    end generate ip_gen;
    
    -- rounds
    left_parts(0) <= ip_out(0 to 31);
    right_parts(0) <= ip_out(32 to 63);
    F_function_inst : entity work.F_function
    port map(
        half_block => right_parts(0),
        subkey => subkeys(0),
        output => f_outs(0)
    );
    rounds_gen: for i in 1 to 15 generate
        left_parts(i) <= right_parts(i-1);
        right_parts(i) <= f_outs(i-1) xor left_parts(i-1);
        
        F_function_inst : entity work.F_function
        port map(
            half_block => right_parts(i),
            subkey => subkeys(i),
            output => f_outs(i)
        );
    end generate rounds_gen;
    
    -- Final Permutation
    fp_in <= (f_outs(15) xor left_parts(15)) & right_parts(15);
    fp_gen: for i in 0 to 63 generate
        fp_out(i) <= fp_in(FP_TABLE(i)-1);
    end generate fp_gen;
    
    ------------------------------------------------
    -- Key schedule
    ------------------------------------------------
    parity_gen: for i in 0 to 7 generate
        key_with_parity(i*8 to i*8+6) <= key_sig(i*7 to i*7+6);
        key_with_parity(i*8+7) <= '0';
    end generate parity_gen;
    
    -- PC1
    pc1_gen: for i in 0 to 55 generate
        pc1_out(i) <= key_with_parity(PC1_TABLE(i)-1);
    end generate pc1_gen;
    
    -- rounds
    rotated_left(0) <= pc1_out(ROT_TABLE(0) to 27) & pc1_out(0 to ROT_TABLE(0)-1);
    rotated_right(0) <= pc1_out(28 + ROT_TABLE(0) to 55) & pc1_out(28 to 28 + ROT_TABLE(0)-1);
    key_rots_gen: for i in 1 to 15 generate
        rotated_left(i) <= rotated_left(i-1)(ROT_TABLE(i) to 27) & rotated_left(i-1)(0 to ROT_TABLE(i)-1);
        rotated_right(i) <= rotated_right(i-1)(ROT_TABLE(i) to 27) & rotated_right(i-1)(0 to ROT_TABLE(i)-1);
    end generate key_rots_gen;
    subkeys_gen: for i in 0 to 15 generate
        pc2_inputs(i) <= rotated_left(i) & rotated_right(i);
        pc2_gen: for j in 0 to 47 generate
            subkeys(i)(j) <= pc2_inputs(i)(PC2_TABLE(j)-1);
        end generate pc2_gen;
    end generate subkeys_gen;

end Arch1;
