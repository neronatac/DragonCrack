----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 15.02.2024 12:49:22
-- Design Name: 
-- Module Name: DEScracker_group - Behavioral
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

entity DEScracker_group is
    Generic (NBR_WORKERS : integer range 1 to 61 := 1;
             VARIABLE_PART_WIDTH : integer range 1 to 55
    );
    Port ( resets : in STD_LOGIC_VECTOR (NBR_WORKERS - 1 downto 0);
           clk : in STD_LOGIC;
           enable : in STD_LOGIC;
           plaintext : in STD_LOGIC_VECTOR (63 downto 0);
           keys_fixed : in STD_LOGIC_VECTOR ((56 - VARIABLE_PART_WIDTH) * NBR_WORKERS - 1 downto 0);
           mask1 : in STD_LOGIC_VECTOR (63 downto 0);
           mask2 : in STD_LOGIC_VECTOR (63 downto 0);
           ref1 : in STD_LOGIC_VECTOR (63 downto 0);  -- ref1 must be coherent with mask1 (already masked itself)
           ref2 : in STD_LOGIC_VECTOR (63 downto 0);  -- ref2 must be coherent with mask2 (already masked itself)
           read_results : in STD_LOGIC_VECTOR (NBR_WORKERS - 1 downto 0);
           
           current_keys : out STD_LOGIC_VECTOR (56 * NBR_WORKERS - 1 downto 0);
           ended_outs : out STD_LOGIC_VECTOR (NBR_WORKERS - 1 downto 0);
           results_available : out STD_LOGIC_VECTOR (NBR_WORKERS - 1 downto 0);
           results_full : out STD_LOGIC_VECTOR (NBR_WORKERS - 1 downto 0);
           match_outs : out STD_LOGIC_VECTOR (NBR_WORKERS - 1 downto 0); -- 0 for match1, 1 for match2
           key_outs : out STD_LOGIC_VECTOR (56 * NBR_WORKERS - 1 downto 0));
end DEScracker_group;

architecture Behavioral of DEScracker_group is

    constant FIXED_PART_WIDTH : integer := 56 - VARIABLE_PART_WIDTH;

begin

    workers_gen: for i in 0 to NBR_WORKERS - 1 generate
        worker_inst: entity work.DEScracker_worker
        generic map(
            VARIABLE_PART_WIDTH => VARIABLE_PART_WIDTH
        )
        port map(
            reset => resets(i),
            clk => clk,
            enable => enable,
            plaintext => plaintext,
            key_fixed => keys_fixed(FIXED_PART_WIDTH*(i+1)-1 downto FIXED_PART_WIDTH*i),
            mask1 => mask1,
            mask2 => mask2,
            ref1 => ref1,
            ref2 => ref2,
            read_result => read_results(i),
            
            current_key => current_keys(56*(i+1)-1 downto 56*i),
            ended => ended_outs(i),
            result_available => results_available(i),
            result_full => results_full(i),
            match_out => match_outs(i),
            key_out => key_outs(56*(i+1)-1 downto 56*i)
        );
    end generate workers_gen;


end Behavioral;
