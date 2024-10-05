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
use IEEE.NUMERIC_STD.ALL;

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
    
    type rot_table_t is array(1 to 16) of natural;
    constant ROT_TABLE_SUM: rot_table_t := (
        1, 2, 4, 6, 8, 10, 12, 14, 15, 17, 19, 21, 23, 25, 27, 28
    );
    
    signal ip_out, fp_in, fp_out: std_logic_vector(0 to 63);
    
    type semi_state_t is array(0 to 16) of std_logic_vector(0 to 31);
    signal left_parts, right_parts: semi_state_t;
    
    type f_outs_t is array(0 to 15) of std_logic_vector(0 to 31);
    signal f_outs: f_outs_t;
    
    type subkeys_t is array(1 to 16) of std_logic_vector(0 to 47);
    signal subkeys: subkeys_t;
    
    type keys_with_parity_sig_t is array(1 to 16) of std_logic_vector(0 to 63);
    signal keys_with_parity: keys_with_parity_sig_t;
    
    type pc1_outs_t is array(1 to 16) of std_logic_vector(0 to 55);
    signal pc1_outs: pc1_outs_t;
    
    type rotated_t is array(1 to 16) of std_logic_vector(0 to 27);
    signal rotated_left, rotated_right: rotated_t;
    
    type pc2_inputs_t is array(1 to 16) of std_logic_vector(0 to 55);
    signal pc2_inputs: pc2_inputs_t;
    
    type keys_sig_t is array(1 to 16) of std_logic_vector(0 to 55);
    signal keys_sig : keys_sig_t;
    signal valids_sig : std_logic_vector(0 to 16);

begin

    ------------------------------------------------
    -- Output signals
    ------------------------------------------------
    ciphertext <= fp_out;
    valid <= valids_sig(16);
    process(clk, reset)
    begin
        if rising_edge(clk) then
            if reset = '1' then
                key_out <= (others => '0');
            elsif enable = '1' then
                key_out <= keys_sig(16);
            end if;
        end if;
    end process;

    ------------------------------------------------
    -- Cipher algorithm + valid signal
    ------------------------------------------------

    -- Initial Permutation
    ip_gen: for i in 0 to 63 generate
        ip_out(i) <= plaintext(IP_TABLE(i)-1);
    end generate ip_gen; 
    
    -- Rounds
    process(clk, reset)
    begin
        if reset = '1' then
            left_parts <= (others => (others => '0'));
            right_parts <= (others => (others => '0'));
            valids_sig <= (others => '0');
            keys_sig <= (others => (others => '0'));
        elsif rising_edge(clk) then
            if enable = '1' then
                left_parts(0) <= ip_out(0 to 31);
                right_parts(0) <= ip_out(32 to 63);
                valids_sig(0) <= '1';
                keys_sig(1) <= key;
                for i in 1 to 16 loop
                    left_parts(i) <= right_parts(i-1);
                    right_parts(i) <= f_outs(i-1) xor left_parts(i-1);
                    valids_sig(i) <= valids_sig(i-1);
                end loop;
                for i in 2 to 16 loop
                    keys_sig(i) <= keys_sig(i-1);
                end loop;
            else
                left_parts <= left_parts;
                right_parts <= right_parts;
                valids_sig <= valids_sig;
                keys_sig <= keys_sig;
            end if;
        end if;
    end process;
    
--    process(clk, reset)
--    begin
--        if reset = '1' then
--            left_parts <= (others => (others => '0'));
--            right_parts <= (others => (others => '0'));
--            valids_sig <= (others => '0');
--            keys_sig <= (others => (others => '0'));
--        elsif rising_edge(clk) then
--            if enable = '1' then
--                left_parts(1) <= ip_out(32 to 63);
--                right_parts(1) <= f_outs(0) xor ip_out(0 to 31);
--                valids_sig(1) <= '1';
--                keys_sig(1) <= key;
--                for i in 2 to 16 loop
--                    left_parts(i) <= right_parts(i-1);
--                    right_parts(i) <= f_outs(i-1) xor left_parts(i-1);
--                    valids_sig(i) <= valids_sig(i-1);
--                    keys_sig(i) <= keys_sig(i-1);
--                end loop;
--            else
--                left_parts <= left_parts;
--                right_parts <= right_parts;
--                valids_sig <= valids_sig;
--                keys_sig <= keys_sig;
--            end if;
--        end if;
--    end process;
    
    
    -- F functions
    F_function_inst0 : entity work.F_function
    port map(
        half_block => ip_out(32 to 63),
        subkey => subkeys(1),
        output => f_outs(0)
    );
    F_functions: for i in 1 to 15 generate
        F_function_inst : entity work.F_function
        port map(
            half_block => right_parts(i),
            subkey => subkeys(i+1),
            output => f_outs(i)
        );
    end generate;
    
    
    -- Final Permutation
    fp_in <= right_parts(16) & left_parts(16);
    fp_gen: for i in 0 to 63 generate
        fp_out(i) <= fp_in(FP_TABLE(i)-1);
    end generate fp_gen;
    
    
    ------------------------------------------------
    -- Key schedule
    ------------------------------------------------
    keys_with_parity_gen: for k in 1 to 16 generate
        parity_gen: for i in 0 to 7 generate
            keys_with_parity(k)(i*8 to i*8+6) <= keys_sig(k)(i*7 to i*7+6);
            keys_with_parity(k)(i*8+7) <= '0';
        end generate parity_gen;
    end generate keys_with_parity_gen;
    
    -- PC1
    pc1_outs_gen: for p in 1 to 16 generate
        single_pc1_gen: for i in 0 to 55 generate
            pc1_outs(p)(i) <= keys_with_parity(p)(PC1_TABLE(i)-1);
        end generate single_pc1_gen;
    end generate pc1_outs_gen;
    
    -- rounds
    key_rounds_gen: for i in 1 to 16 generate
        rotated_left(i) <= std_logic_vector(rotate_left(unsigned(pc1_outs(i)(0 to 27)), ROT_TABLE_SUM(i)));
        rotated_right(i) <= std_logic_vector(rotate_left(unsigned(pc1_outs(i)(28 to 55)), ROT_TABLE_SUM(i)));
        pc2_inputs(i) <= rotated_left(i) & rotated_right(i);
        
        pc2_gen: for j in 0 to 47 generate
            subkeys(i)(j) <= pc2_inputs(i)(PC2_TABLE(j)-1);
        end generate pc2_gen;
    end generate key_rounds_gen;

end Arch1;
