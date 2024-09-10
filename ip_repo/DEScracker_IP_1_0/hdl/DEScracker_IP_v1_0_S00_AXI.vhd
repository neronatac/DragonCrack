library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

entity DEScracker_IP_v1_0_S00_AXI is
	generic (
		-- Users to add parameters here
        NBR_WORKERS: integer range 1 to 61 := 1;
		-- User parameters ends
		-- Do not modify the parameters beyond this line

		-- Width of S_AXI data bus
		C_S_AXI_DATA_WIDTH	: integer	:= 32;
		-- Width of S_AXI address bus
		C_S_AXI_ADDR_WIDTH	: integer	:= 11
	);
	port (
		-- Users to add ports here
        -- for debug
        dbg_plaintext : out STD_LOGIC_VECTOR (63 downto 0);
        dbg_mask1 : out STD_LOGIC_VECTOR (63 downto 0);
        dbg_mask2 : out STD_LOGIC_VECTOR (63 downto 0);
        dbg_ref1 : out STD_LOGIC_VECTOR (63 downto 0);
        dbg_ref2 : out STD_LOGIC_VECTOR (63 downto 0);
		-- User ports ends
		-- Do not modify the ports beyond this line

		-- Global Clock Signal
		S_AXI_ACLK	: in std_logic;
		-- Global Reset Signal. This Signal is Active LOW
		S_AXI_ARESETN	: in std_logic;
		-- Write address (issued by master, acceped by Slave)
		S_AXI_AWADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		-- Write channel Protection type. This signal indicates the
    		-- privilege and security level of the transaction, and whether
    		-- the transaction is a data access or an instruction access.
		S_AXI_AWPROT	: in std_logic_vector(2 downto 0);
		-- Write address valid. This signal indicates that the master signaling
    		-- valid write address and control information.
		S_AXI_AWVALID	: in std_logic;
		-- Write address ready. This signal indicates that the slave is ready
    		-- to accept an address and associated control signals.
		S_AXI_AWREADY	: out std_logic;
		-- Write data (issued by master, acceped by Slave) 
		S_AXI_WDATA	: in std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		-- Write strobes. This signal indicates which byte lanes hold
    		-- valid data. There is one write strobe bit for each eight
    		-- bits of the write data bus.    
		S_AXI_WSTRB	: in std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
		-- Write valid. This signal indicates that valid write
    		-- data and strobes are available.
		S_AXI_WVALID	: in std_logic;
		-- Write ready. This signal indicates that the slave
    		-- can accept the write data.
		S_AXI_WREADY	: out std_logic;
		-- Write response. This signal indicates the status
    		-- of the write transaction.
		S_AXI_BRESP	: out std_logic_vector(1 downto 0);
		-- Write response valid. This signal indicates that the channel
    		-- is signaling a valid write response.
		S_AXI_BVALID	: out std_logic;
		-- Response ready. This signal indicates that the master
    		-- can accept a write response.
		S_AXI_BREADY	: in std_logic;
		-- Read address (issued by master, acceped by Slave)
		S_AXI_ARADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		-- Protection type. This signal indicates the privilege
    		-- and security level of the transaction, and whether the
    		-- transaction is a data access or an instruction access.
		S_AXI_ARPROT	: in std_logic_vector(2 downto 0);
		-- Read address valid. This signal indicates that the channel
    		-- is signaling valid read address and control information.
		S_AXI_ARVALID	: in std_logic;
		-- Read address ready. This signal indicates that the slave is
    		-- ready to accept an address and associated control signals.
		S_AXI_ARREADY	: out std_logic;
		-- Read data (issued by slave)
		S_AXI_RDATA	: out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		-- Read response. This signal indicates the status of the
    		-- read transfer.
		S_AXI_RRESP	: out std_logic_vector(1 downto 0);
		-- Read valid. This signal indicates that the channel is
    		-- signaling the required read data.
		S_AXI_RVALID	: out std_logic;
		-- Read ready. This signal indicates that the master can
    		-- accept the read data and response information.
		S_AXI_RREADY	: in std_logic
	);
end DEScracker_IP_v1_0_S00_AXI;

architecture arch_imp of DEScracker_IP_v1_0_S00_AXI is

	-- AXI4LITE signals
	signal axi_awaddr	: std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
	signal axi_awready	: std_logic;
	signal axi_wready	: std_logic;
	signal axi_bresp	: std_logic_vector(1 downto 0);
	signal axi_bvalid	: std_logic;
	signal axi_araddr	: std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
	signal axi_arready	: std_logic;
	signal axi_rdata	: std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal axi_rresp	: std_logic_vector(1 downto 0);
	signal axi_rvalid	: std_logic;

	-- Example-specific design signals
	-- local parameter for addressing 32 bit / 64 bit C_S_AXI_DATA_WIDTH
	-- ADDR_LSB is used for addressing 32/64 bit registers/memories
	-- ADDR_LSB = 2 for 32 bits (n downto 2)
	-- ADDR_LSB = 3 for 64 bits (n downto 3)
	constant ADDR_LSB  : integer := (C_S_AXI_DATA_WIDTH/32)+ 1;
	constant OPT_MEM_ADDR_BITS : integer := 8;
	------------------------------------------------
	---- Signals for user logic register space example
	--------------------------------------------------
	signal slv_reg_rden	: std_logic;
	signal slv_reg_wren	: std_logic;
	signal reg_data_out	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal byte_index	: integer;
	signal aw_en	: std_logic;
	
	constant NUMBER_OF_REGS : integer := 505;
	constant OPTI_ADDR : integer := integer(ceil(log2(real(NUMBER_OF_REGS)))) - 1;
	type all_regs_t is array (0 to NUMBER_OF_REGS-1) of std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal all_regs : all_regs_t;
	
	signal plaintext, mask1, mask2, ref1, ref2 : STD_LOGIC_VECTOR (63 downto 0);
	signal start_keys, end_keys: STD_LOGIC_VECTOR (56 * NBR_WORKERS - 1 downto 0);
	signal read_results: STD_LOGIC_VECTOR (NBR_WORKERS - 1 downto 0);
	
	signal current_keys, key_outs : STD_LOGIC_VECTOR (56 * NBR_WORKERS - 1 downto 0);
	signal ended_outs, results_available, results_full, match_outs : STD_LOGIC_VECTOR (NBR_WORKERS - 1 downto 0);

begin
	-- I/O Connections assignments

	S_AXI_AWREADY	<= axi_awready;
	S_AXI_WREADY	<= axi_wready;
	S_AXI_BRESP	<= axi_bresp;
	S_AXI_BVALID	<= axi_bvalid;
	S_AXI_ARREADY	<= axi_arready;
	S_AXI_RDATA	<= axi_rdata;
	S_AXI_RRESP	<= axi_rresp;
	S_AXI_RVALID	<= axi_rvalid;
	-- Implement axi_awready generation
	-- axi_awready is asserted for one S_AXI_ACLK clock cycle when both
	-- S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_awready is
	-- de-asserted when reset is low.

	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_awready <= '0';
	      aw_en <= '1';
	    else
	      if (axi_awready = '0' and S_AXI_AWVALID = '1' and S_AXI_WVALID = '1' and aw_en = '1') then
	        -- slave is ready to accept write address when
	        -- there is a valid write address and write data
	        -- on the write address and data bus. This design 
	        -- expects no outstanding transactions. 
	           axi_awready <= '1';
	           aw_en <= '0';
	        elsif (S_AXI_BREADY = '1' and axi_bvalid = '1') then
	           aw_en <= '1';
	           axi_awready <= '0';
	      else
	        axi_awready <= '0';
	      end if;
	    end if;
	  end if;
	end process;

	-- Implement axi_awaddr latching
	-- This process is used to latch the address when both 
	-- S_AXI_AWVALID and S_AXI_WVALID are valid. 

	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_awaddr <= (others => '0');
	    else
	      if (axi_awready = '0' and S_AXI_AWVALID = '1' and S_AXI_WVALID = '1' and aw_en = '1') then
	        -- Write Address latching
	        axi_awaddr <= S_AXI_AWADDR;
	      end if;
	    end if;
	  end if;                   
	end process; 

	-- Implement axi_wready generation
	-- axi_wready is asserted for one S_AXI_ACLK clock cycle when both
	-- S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_wready is 
	-- de-asserted when reset is low. 

	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_wready <= '0';
	    else
	      if (axi_wready = '0' and S_AXI_WVALID = '1' and S_AXI_AWVALID = '1' and aw_en = '1') then
	          -- slave is ready to accept write data when 
	          -- there is a valid write address and write data
	          -- on the write address and data bus. This design 
	          -- expects no outstanding transactions.           
	          axi_wready <= '1';
	      else
	        axi_wready <= '0';
	      end if;
	    end if;
	  end if;
	end process; 

	-- Implement memory mapped register select and write logic generation
	-- The write data is accepted and written to memory mapped registers when
	-- axi_awready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted. Write strobes are used to
	-- select byte enables of slave registers while writing.
	-- These registers are cleared when reset (active low) is applied.
	-- Slave register write enable is asserted when valid address and data are available
	-- and the slave is ready to accept the write address and write data.
	slv_reg_wren <= axi_wready and S_AXI_WVALID and axi_awready and S_AXI_AWVALID ;

	process (S_AXI_ACLK)
	variable w_idx: integer;
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      for i in 0 to NUMBER_OF_REGS-1 loop
	        case i is
	          when 0 =>
	            all_regs(i) <= x"01000000";
	            
	          when 7 to 10 =>
	            all_regs(i) <= (others => '1');
	            
	          when others =>
	            all_regs(i) <= (others => '0');
	        end case;
	      end loop;
	    else
	      w_idx := to_integer(unsigned(axi_awaddr(ADDR_LSB + OPTI_ADDR downto ADDR_LSB)));
	      if (slv_reg_wren = '1') then
	        case w_idx is
	          when 0 =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' and byte_index /= 3 and byte_index /= 2) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 0
	                all_regs(w_idx)(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;

	          when others =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
                    if ( S_AXI_WSTRB(byte_index) = '1' ) then
                        -- Respective byte enables are asserted as per write strobes                   
                        all_regs(w_idx)(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
                    end if;
                end loop;
	        end case;
	      end if;
	    end if;
	  end if;                   
	end process; 

	-- Implement write response logic generation
	-- The write response and response valid signals are asserted by the slave 
	-- when axi_wready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted.  
	-- This marks the acceptance of address and indicates the status of 
	-- write transaction.

	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_bvalid  <= '0';
	      axi_bresp   <= "00"; --need to work more on the responses
	    else
	      if (axi_awready = '1' and S_AXI_AWVALID = '1' and axi_wready = '1' and S_AXI_WVALID = '1' and axi_bvalid = '0'  ) then
	        axi_bvalid <= '1';
	        axi_bresp  <= "00"; 
	      elsif (S_AXI_BREADY = '1' and axi_bvalid = '1') then   --check if bready is asserted while bvalid is high)
	        axi_bvalid <= '0';                                 -- (there is a possibility that bready is always asserted high)
	      end if;
	    end if;
	  end if;                   
	end process; 

	-- Implement axi_arready generation
	-- axi_arready is asserted for one S_AXI_ACLK clock cycle when
	-- S_AXI_ARVALID is asserted. axi_awready is 
	-- de-asserted when reset (active low) is asserted. 
	-- The read address is also latched when S_AXI_ARVALID is 
	-- asserted. axi_araddr is reset to zero on reset assertion.

	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_arready <= '0';
	      axi_araddr  <= (others => '1');
	    else
	      if (axi_arready = '0' and S_AXI_ARVALID = '1') then
	        -- indicates that the slave has acceped the valid read address
	        axi_arready <= '1';
	        -- Read Address latching 
	        axi_araddr  <= S_AXI_ARADDR;           
	      else
	        axi_arready <= '0';
	      end if;
	    end if;
	  end if;                   
	end process; 

	-- Implement axi_arvalid generation
	-- axi_rvalid is asserted for one S_AXI_ACLK clock cycle when both 
	-- S_AXI_ARVALID and axi_arready are asserted. The slave registers 
	-- data are available on the axi_rdata bus at this instance. The 
	-- assertion of axi_rvalid marks the validity of read data on the 
	-- bus and axi_rresp indicates the status of read transaction.axi_rvalid 
	-- is deasserted on reset (active low). axi_rresp and axi_rdata are 
	-- cleared to zero on reset (active low).  
	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then
	    if S_AXI_ARESETN = '0' then
	      axi_rvalid <= '0';
	      axi_rresp  <= "00";
	    else
	      if (axi_arready = '1' and S_AXI_ARVALID = '1' and axi_rvalid = '0') then
	        -- Valid read data is available at the read data bus
	        axi_rvalid <= '1';
	        axi_rresp  <= "00"; -- 'OKAY' response
	      elsif (axi_rvalid = '1' and S_AXI_RREADY = '1') then
	        -- Read data is accepted by the master
	        axi_rvalid <= '0';
	      end if;            
	    end if;
	  end if;
	end process;

	-- Implement memory mapped register select and read logic generation
	-- Slave register read enable is asserted when valid address is available
	-- and the slave is ready to accept the read address.
	slv_reg_rden <= axi_arready and S_AXI_ARVALID and (not axi_rvalid) ;

	process (all_regs, axi_araddr, S_AXI_ARESETN, slv_reg_rden, current_keys, match_outs, key_outs, ended_outs, results_available, results_full)
	variable r_idx: integer;
	variable high_low: integer;  -- 0 when 'high', 1 when 'low'
	variable worker_idx: integer;
	begin
	    -- Address decoding for reading registers
	    r_idx := to_integer(unsigned(axi_araddr(ADDR_LSB + OPTI_ADDR downto ADDR_LSB)));
	    case r_idx is
	    
	      -- current keys
	      when 255 to 376 =>
	        high_low := (r_idx - 255) mod 2;
	        worker_idx := (r_idx - 255 - high_low) / 2;
	        if (worker_idx >= NBR_WORKERS) then
	          reg_data_out <= (others => '0');
	        elsif (high_low = 0) then
	          reg_data_out(31 downto 24) <= (others => '0');
	          reg_data_out(23 downto 0) <= current_keys(56 * worker_idx + 55 downto 56 * worker_idx + 32);
	        else
	          reg_data_out <= current_keys(56 * worker_idx + 31 downto 56 * worker_idx);
	        end if;
	        
	      -- key_outs and match_outs
	      when 383 to 504 =>
	        high_low := (r_idx - 383) mod 2;
	        worker_idx := (r_idx - 383 - high_low) / 2;
	        if (worker_idx >= NBR_WORKERS) then
	          reg_data_out <= (others => '0');
	        elsif (high_low = 0) then
	          reg_data_out(31) <= match_outs(worker_idx);
	          reg_data_out(30 downto 24) <= (others => '0');
	          reg_data_out(23 downto 0) <= key_outs(56 * worker_idx + 55 downto 56 * worker_idx + 32);
	        else
	          reg_data_out <= key_outs(56 * worker_idx + 31 downto 56 * worker_idx);
	        end if;
	        
	      -- ended_outs high
	      when 377 =>
	        reg_data_out(31 downto 29) <= "000";
	        for i in 28 downto 0 loop
	          if (i+32 < NBR_WORKERS) then
	            reg_data_out(i) <= ended_outs(i+32);
	          else
	            reg_data_out(i) <= '0';
	          end if;
	        end loop;
	        
	      -- ended_outs low
	      when 378 =>
	        for i in 31 downto 0 loop
	          if (i < NBR_WORKERS) then
	            reg_data_out(i) <= ended_outs(i);
	          else
	            reg_data_out(i) <= '0';
	          end if;
	        end loop;
	        
	      -- results_available high
	      when 379 =>
	        reg_data_out(31 downto 29) <= "000";
	        for i in 28 downto 0 loop
	          if (i+32 < NBR_WORKERS) then
	            reg_data_out(i) <= results_available(i+32);
	          else
	            reg_data_out(i) <= '0';
	          end if;
	        end loop;
	        
	      -- results_available low
	      when 380 =>
	        for i in 31 downto 0 loop
	          if (i < NBR_WORKERS) then
	            reg_data_out(i) <= results_available(i);
	          else
	            reg_data_out(i) <= '0';
	          end if;
	        end loop;
	        
	      -- results_full high
	      when 381 =>
	        reg_data_out(31 downto 29) <= "000";
	        for i in 28 downto 0 loop
	          if (i+32 < NBR_WORKERS) then
	            reg_data_out(i) <= results_full(i+32);
	          else
	            reg_data_out(i) <= '0';
	          end if;
	        end loop;
	        
	      -- results_full low
	      when 382 =>
	        for i in 31 downto 0 loop
	          if (i < NBR_WORKERS) then
	            reg_data_out(i) <= results_full(i);
	          else
	            reg_data_out(i) <= '0';
	          end if;
	        end loop;
	    
	      when others =>
	        reg_data_out <= all_regs(r_idx);
	    end case;
	end process; 

	-- Output register or memory read data
	process( S_AXI_ACLK ) is
	begin
	  if (rising_edge (S_AXI_ACLK)) then
	    if ( S_AXI_ARESETN = '0' ) then
	      axi_rdata  <= (others => '0');
	    else
	      if (slv_reg_rden = '1') then
	        -- When there is a valid read address (S_AXI_ARVALID) with 
	        -- acceptance of read address by the slave (axi_arready), 
	        -- output the read dada 
	        -- Read address mux
	          axi_rdata <= reg_data_out;     -- register read data
	      end if;   
	    end if;
	  end if;
	end process;


	-- Add user logic here
	-- signals about keys (on 2 registers each)
	sigs_key_gen: for w in 0 to NBR_WORKERS-1 generate
	       start_keys(56 * w + 55 downto 56 * w) <= all_regs(11+w*2)(23 downto 0) & all_regs(11+w*2+1);
	       
	       end_keys(56 * w + 55 downto 56 * w) <= all_regs(133+w*2)(23 downto 0) & all_regs(133+w*2+1);
	end generate sigs_key_gen;
	
	-- read_result signal handling
	process( S_AXI_ACLK, axi_araddr ) is
	  variable r_idx: integer;
	  variable high_low: integer;  -- 0 when 'high', 1 when 'low'
	  variable worker_idx: integer;
	begin
	    r_idx := to_integer(unsigned(axi_araddr(ADDR_LSB + OPTI_ADDR downto ADDR_LSB)));
	    high_low := (r_idx - 383) mod 2;
	    worker_idx := (r_idx - 383 - high_low) / 2;
	    
        if (rising_edge (S_AXI_ACLK)) then
          if ( S_AXI_ARESETN = '0' ) then
	        read_results <= (others => '0');
	      else
            if (slv_reg_rden = '1' and r_idx >= 383 and r_idx <= 504 and high_low = 1) then
                -- read_results is set to '1' only for one clock cycle: when lower register is read
                -- Thus, higher register must be read first!
                read_results(worker_idx) <= '1';
            else
                read_results <= (others => '0');
            end if;  
          end if; 
        end if;     
	end process;
	
	-- 64-bits signals
	plaintext <= all_regs(1) & all_regs(2);
	mask1 <= all_regs(3) & all_regs(4);
	mask2 <= all_regs(5) & all_regs(6);
	ref1 <= all_regs(7) & all_regs(8);
	ref2 <= all_regs(9) & all_regs(10);
	
    descracker_inst : entity work.DEScracker_group
        generic map(NBR_WORKERS => NBR_WORKERS)
        port map(
            reset => all_regs(0)(0),
            clk => S_AXI_ACLK,
            enable => all_regs(0)(1),
            plaintext => plaintext,
            start_keys => start_keys,
            end_keys => end_keys,
            mask1 => mask1,
            mask2 => mask2,
            ref1 => ref1,
            ref2 => ref2,
            read_results => read_results,
            
            current_keys => current_keys,
            ended_outs => ended_outs,
            results_available => results_available,
            results_full => results_full,
            match_outs => match_outs, -- 0 for match1, 1 for match2
            key_outs => key_outs
        );
        
    -- debug ports
    dbg_plaintext <= plaintext;
    dbg_mask1 <= mask1;
    dbg_mask2 <= mask2;
    dbg_ref1 <= ref1;
    dbg_ref2 <= ref2;
    
	-- User logic ends

end arch_imp;
