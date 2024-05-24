library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

-------------------------------------------------------
--! Decoder Entity
-------------------------------------------------------
entity Decoder is
    generic (
        DECIMATION         : integer := 4;
        FIFO_WIDTH         : integer := 32;
        FIFO_DEPTH         : integer := 16;
        OUTPUT_DELAY       : integer := 0
    );
	port (
		clk                : in  std_logic;
		reset              : in  std_logic;            

		avs_s0_address     : in  std_logic_vector(10 downto 0);
		avs_s0_read        : in  std_logic;            
		avs_s0_readdata    : out std_logic_vector(31 downto 0);                   
		avs_s0_waitrequest : out std_logic;                                       
		avs_s0_write       : in  std_logic;            
		avs_s0_writedata   : in  std_logic_vector(31 downto 0)
    );
end entity Decoder;

-------------------------------------------------------
--! Architecture of DataMover
-------------------------------------------------------
architecture Behavioral of Decoder is
    -------------------------------------------------------
    --! Constants
    -------------------------------------------------------
    constant MAGIC_NUMBER       : std_logic_vector(31 downto 0) := x"2224ABCD";
    constant SOFT_RESET_CODE    : std_logic_vector(31 downto 0) := x"FF00FF00";
    constant FIFO_IS_EMPTY_CODE : std_logic_vector(31 downto 0) := x"01010101";
    constant FIFO_IS_FULL_CODE  : std_logic_vector(31 downto 0) := x"F0F0F0F0";


    -------------------------------------------------------
    --!> Stats
    -------------------------------------------------------
    signal write_cnt_reg, write_cnt_next : unsigned(63 downto 0) := (others=>'0');
    signal read_cnt_reg, read_cnt_next : unsigned(63 downto 0) := (others=>'0');

    -------------------------------------------------------
    --!> FIFO
    -------------------------------------------------------
    signal fifo_write_en : std_logic := '0';
    signal fifo_write_data : std_logic_vector(FIFO_WIDTH-1 downto 0) := (others => '0');
    signal fifo_full : std_logic := '0';
    signal fifo_read_en_reg, fifo_read_en_next : std_logic := '0';
    signal fifo_read_data : std_logic_vector(FIFO_WIDTH-1 downto 0) := (others => '0');
    signal fifo_empty : std_logic := '0';

    -------------------------------------------------------
    --!> SoftReset
    -------------------------------------------------------
    signal softResetSignal      : std_logic := '0';
    signal softResetSignalSet   : std_logic := '0';
    signal softResetSignalClear : std_logic := '0';

    signal reset_counter : unsigned(31 downto 0) := (others =>'0');
    signal soft_reset_counter : unsigned(31 downto 0) := (others =>'0');

    -------------------------------------------------------
    --! Register Setup
    -------------------------------------------------------
    type RegisterNames is (
        --Status
        MagicNumber,              
        ResetCounter,        
        SoftResetCounter,        
        WritesPerformedUpper,        
        WritesPerformedLower,        
        ReadsPerformedUpper,        
        ReadsPerformedLower,        
        IsFull,
        IsEmpty,
        --Commands
        ReadData,     
        WriteData,
        SoftReset,        
        RegisterLast     
    );

    -------------------------------------------------------
    --!> IS VALID REG 
    -------------------------------------------------------
    function IS_VALID_REG(
        address: in std_logic_vector(10 downto 0)
    ) return std_ulogic is
    begin
        if (to_integer(unsigned(address)) < RegisterNames'Pos(RegisterLast)) then
            return '1';
        else
            return '0';
        end if;
    end IS_VALID_REG;

-------------------------------------------------------
--! BEGIN
-------------------------------------------------------
begin
    -------------------------------------------------------
    --! PROCESS: Register Write
    -------------------------------------------------------
    process (clk, reset, avs_s0_address, avs_s0_writedata) 
    begin
        if rising_edge(clk) then
            write_cnt_next <= write_cnt_reg;
            softResetSignalSet <= '0';
            fifo_write_en <= '0';

            if avs_s0_write='1' then
                case RegisterNames'Val(to_integer(unsigned(avs_s0_address))) is
                    when SoftReset =>
                        if( avs_s0_writedata = SOFT_RESET_CODE) then
                            softResetSignalSet <= '1';
                            soft_reset_counter <= soft_reset_counter + to_unsigned(1, 32);
                        end if;
                    when WriteData =>
                        if( fifo_full = '0') then
                            fifo_write_data <= avs_s0_writedata;
                            fifo_write_en <= '1';
                            write_cnt_next <= write_cnt_reg + to_unsigned(1, 64);
                        end if;
                    when others =>
                        null;
                end case;
            end if;
        end if;
    end process;

    -------------------------------------------------------
    --! PROCESS: Register Read
    -------------------------------------------------------
    process (
        avs_s0_read, avs_s0_address, write_cnt_reg, reset_counter, soft_reset_counter,
        read_cnt_reg, fifo_full, fifo_empty, fifo_read_data
    ) 
    begin
        read_cnt_next <= read_cnt_reg;
        avs_s0_readdata <= (others => '0');
        avs_s0_waitrequest <= '0';
        fifo_read_en_next <= '0';

        if(avs_s0_read='1') then 
            case RegisterNames'Val(to_integer(unsigned(avs_s0_address))) is
                when MagicNumber =>
                    avs_s0_readdata <= MAGIC_NUMBER;
                when ResetCounter =>
                    avs_s0_readdata <= std_logic_vector(reset_counter);
                when SoftResetCounter =>
                    avs_s0_readdata <= std_logic_vector(soft_reset_counter);
                when WritesPerformedUpper =>
                    avs_s0_readdata <= std_logic_vector(write_cnt_reg(63 downto 32));
                when WritesPerformedLower =>
                    avs_s0_readdata <= std_logic_vector(write_cnt_reg(31 downto 0));
                when ReadsPerformedUpper =>
                    avs_s0_readdata <= std_logic_vector(read_cnt_reg(63 downto 32));
                when ReadsPerformedLower =>
                    avs_s0_readdata <= std_logic_vector(read_cnt_reg(31 downto 0));
                when IsFull =>
                    if( fifo_full = '0') then
                        avs_s0_readdata <= (others => '0');
                    else
                        avs_s0_readdata <= FIFO_IS_FULL_CODE;
                    end if;
                when IsEmpty =>
                    if( fifo_empty = '0') then
                        avs_s0_readdata <= (others => '0');
                    else
                        avs_s0_readdata <= FIFO_IS_EMPTY_CODE;
                    end if;
                when ReadData =>
                    if( fifo_empty = '0') then
                        read_cnt_next <= read_cnt_reg + to_unsigned(1, 64);
                        fifo_read_en_next <= '1';
                        avs_s0_readdata <= fifo_read_data;
                    end if;
                when others =>
                    null;
            end case;
        end if;
    end process;

    -------------------------------------------------------
    --! PROCESS: Main Update
    -------------------------------------------------------
    process(
        clk, reset
    )
    begin
        if(rising_edge(clk)) then
            if( reset = '1' or softResetSignal = '1') then
                if( softResetSignal = '1') then
                    softResetSignalClear <= '1';
                end if;

                write_cnt_reg <= (others => '0');
                read_cnt_reg <= (others => '0');
                reset_counter <= reset_counter + to_unsigned(1, 32);
                fifo_read_en_reg <= '0';
            else
                softResetSignalClear <= '0';

                read_cnt_reg <= read_cnt_next;
                write_cnt_reg <= write_cnt_next;

                reset_counter <= reset_counter;
                fifo_read_en_reg <= fifo_read_en_next;
            end if;
        end if;
    end process;

    -------------------------------------------------------
    --!> FIFO
    -------------------------------------------------------
    dataFifo : entity work.Fifo(Behavioral)
    generic map
    (
        g_width => FIFO_WIDTH,
        g_depth => FIFO_DEPTH
    )
    port map
    (
        i_clk => clk,
        i_rst_sync => (reset or softResetSignal),
        i_wr_en => fifo_write_en,
        i_wr_data => fifo_write_data,
        o_full => fifo_full,
        i_rd_en => fifo_read_en_reg,
        o_rd_data => fifo_read_data,
        o_empty => fifo_empty
    );

    -------------------------------------------------------
    --!> Soft Reset Signal Manager
    -------------------------------------------------------
    SoftResetSignalManager : entity work.SignalManager(Behavioral)
    port map
    (
        clk => clk,
        reset => reset,
        sig => softResetSignal,
        set => softResetSignalSet,
        clear => softResetSignalClear
    );

end architecture Behavioral; 
