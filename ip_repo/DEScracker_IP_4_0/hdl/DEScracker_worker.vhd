----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 07.02.2024 22:41:36
-- Design Name: 
-- Module Name: DEScracker_worker - Behavioral
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

entity DEScracker_worker is
    Port ( reset : in STD_LOGIC;
           clk : in STD_LOGIC;
           enable : in STD_LOGIC;
           plaintext : in STD_LOGIC_VECTOR (63 downto 0);
           start_key : in STD_LOGIC_VECTOR (55 downto 0);
           end_key : in STD_LOGIC_VECTOR (55 downto 0);
           mask1 : in STD_LOGIC_VECTOR (63 downto 0);
           mask2 : in STD_LOGIC_VECTOR (63 downto 0);
           ref1 : in STD_LOGIC_VECTOR (63 downto 0);  -- ref1 must be coherent with mask1 (already masked itself)
           ref2 : in STD_LOGIC_VECTOR (63 downto 0);  -- ref2 must be coherent with mask2 (already masked itself)
           read_result : in STD_LOGIC;
           
           current_key : out STD_LOGIC_VECTOR (55 downto 0);
           ended : out STD_LOGIC;
           result_available : out STD_LOGIC;
           result_full : out STD_LOGIC;
           match_out : out STD_LOGIC; -- 0 for match1, 1 for match2
           key_out : out STD_LOGIC_VECTOR (55 downto 0));
end DEScracker_worker;

architecture Arch1 of DEScracker_worker is
    -- Arch1: no output FIFO, it stops when a result is pending
    
    type state_t is (idle, running, full, finished, wait_to_run);
    signal run_state : state_t := idle;
    
    ------------------------------------------------
    -- Signals
    ------------------------------------------------
    signal ciphertext : std_logic_vector (63 downto 0);
    signal key, key_out_sig, key_out_from_DES : std_logic_vector (55 downto 0);
    signal match1, match2, valid, DES_match, match_out_sig : std_logic;
    signal enable_DES : std_logic;

begin

    -- DES instance
    DES_inst: entity work.DES
    port map(
        plaintext => plaintext,
        key => key,
        clk => clk,
        reset => reset,
        enable => enable_DES,
        valid => valid,
        ciphertext => ciphertext,
        key_out => key_out_from_DES
    );
    
    -- matching
    match1 <= '1' when (ciphertext and mask1) = ref1 else '0';
    match2 <= '1' when (ciphertext and mask2) = ref2 else '0';
    DES_match <= '1' when valid = '1' and (match1 = '1' or match2 = '1') else '0';
    
    -- result output
    result_available <= '1' when run_state = full else '0';
    result_full <= '1' when run_state = full else '0';
    match_out <= match_out_sig;
    key_out <= key_out_sig;
    
    -- stop DES when necessary
    process(run_state)
    begin
        if run_state = running then
            enable_DES <= '1';
        else
            enable_DES <= '0';
        end if;
    end process;
    
    -- run FSM
    process(clk, reset, start_key)
    begin
        if rising_edge(clk) then
            if reset = '1' then
                run_state <= idle;
                key <= start_key;
                key_out_sig <= (others => '0');
                match_out_sig <= '0';
            else
                case run_state is
                    when idle =>
                        if enable = '1' then
                            run_state <= running;
                        else
                            run_state <= idle;
                        end if;
                     
                    when running =>
                        if enable = '0' then
                            run_state <= idle;
                        elsif key_out_from_DES > end_key then
                            run_state <= finished;
                        elsif DES_match = '1' then
                            run_state <= full;
                            key_out_sig <= key_out_from_DES;
                            if match2 = '1' then
                                match_out_sig <= '1';
                            else
                                match_out_sig <= '0';
                            end if;
                            key <= std_logic_vector(unsigned(key) + 1);
                        else
                            key <= std_logic_vector(unsigned(key) + 1);
                            run_state <= running;
                        end if;

                    when full =>
                        if read_result = '1' then
                            run_state <= wait_to_run;
                        else
                            run_state <= full;
                        end if;
                        
                    when wait_to_run =>
                        if read_result = '0' and enable = '0' then
                            run_state <= idle;
                        elsif read_result = '0' and enable = '1' then
                            run_state <= running;
                        else
                            run_state <= wait_to_run;
                        end if;
                        
                    when finished =>
                        run_state <= finished;
                        
                    when others =>
                        run_state <= idle;
                        
                end case;
            end if;
        end if;
    end process;
    
    -- signals associated to FSM
    ended <= '1' when run_state = finished else '0';
    
    current_key <= key_out_from_DES;

end Arch1;
