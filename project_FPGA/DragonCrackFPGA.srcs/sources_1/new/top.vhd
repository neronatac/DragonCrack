library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity top is
    port (
        -- from Block Design
        -- DDR
        DDR_addr:    inout std_logic_vector(14 downto 0);
        DDR_ba:      inout std_logic_vector(2 downto 0);
        DDR_cas_n:   inout std_logic;
        DDR_ck_n:    inout std_logic;
        DDR_ck_p:    inout std_logic;
        DDR_cke:     inout std_logic;
        DDR_cs_n:    inout std_logic;
        DDR_dm:      inout std_logic_vector(3 downto 0);
        DDR_dq:      inout std_logic_vector(31 downto 0);
        DDR_dqs_n:   inout std_logic_vector(3 downto 0);
        DDR_dqs_p:   inout std_logic_vector(3 downto 0);
        DDR_odt:     inout std_logic;
        DDR_ras_n:   inout std_logic;
        DDR_reset_n: inout std_logic;
        DDR_we_n:    inout std_logic;

        -- PS peripherals
        FIXED_IO_ddr_vrn:  inout std_logic;
        FIXED_IO_ddr_vrp:  inout std_logic;
        FIXED_IO_mio:      inout std_logic_vector(53 downto 0);
        FIXED_IO_ps_clk:   inout std_logic;
        FIXED_IO_ps_porb:  inout std_logic;
        FIXED_IO_ps_srstb: inout std_logic;

        -- from HDL sources
        -- ADC
        adc_cdcs_o: out std_logic; -- ADC clock duty cycle stabilizer

        -- LEDs
        leds : out std_logic_vector(7 downto 0)
    );
end top;


architecture Behavioral of top is

begin

    -- block design instance
    block_design_inst : entity work.main_bd_wrapper
    port map(
        DDR_addr => DDR_addr,
        DDR_ba => DDR_ba,
        DDR_cas_n => DDR_cas_n,
        DDR_ck_n => DDR_ck_n,
        DDR_ck_p => DDR_ck_p,
        DDR_cke => DDR_cke,
        DDR_cs_n => DDR_cs_n,
        DDR_dm => DDR_dm,
        DDR_dq => DDR_dq,
        DDR_dqs_n => DDR_dqs_n,
        DDR_dqs_p => DDR_dqs_p,
        DDR_odt => DDR_odt,
        DDR_ras_n => DDR_ras_n,
        DDR_reset_n => DDR_reset_n,
        DDR_we_n => DDR_we_n,
        FIXED_IO_ddr_vrn => FIXED_IO_ddr_vrn,
        FIXED_IO_ddr_vrp => FIXED_IO_ddr_vrp,
        FIXED_IO_mio => FIXED_IO_mio,
        FIXED_IO_ps_clk => FIXED_IO_ps_clk,
        FIXED_IO_ps_porb => FIXED_IO_ps_porb,
        FIXED_IO_ps_srstb => FIXED_IO_ps_srstb
    );

    ----------------------------------------------------------
    --- ADC
    ----------------------------------------------------------
    adc_cdcs_o <= '1'; -- enable clock duty cyle enable

    ----------------------------------------------------------
    --- LEDs
    ----------------------------------------------------------
    leds <= (others => '0');

end Behavioral;
