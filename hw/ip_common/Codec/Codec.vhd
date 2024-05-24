library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

-------------------------------------------------------
--! Codec Entity
-------------------------------------------------------
entity Codec is
    generic (
        FIFO_WIDTH         : integer := 32;
        FIFO_DEPTH         : integer := 16;
        DELAY              : integer := 0
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
end entity Codec;

-------------------------------------------------------
--! Architecture of DataMover
-------------------------------------------------------
architecture Behavioral of Codec is
    -------------------------------------------------------
    --! Constants
    -------------------------------------------------------
    constant MAGIC_NUMBER       : std_logic_vector(31 downto 0) := x"2224ABCD";
    constant SOFT_RESET_CODE    : std_logic_vector(31 downto 0) := x"FF00FF00";
    constant FIFO_IS_EMPTY_CODE : std_logic_vector(31 downto 0) := x"0F0F0F0F";
    constant FIFO_IS_FULL_CODE  : std_logic_vector(31 downto 0) := x"F0F0F0F0";
    constant CODEC_SETTING_INTERPOLATE_CODE  : std_logic_vector(31 downto 0) := x"0000FFFF";
    constant CODEC_SETTING_DECIMATE_CODE  : std_logic_vector(31 downto 0) := x"FFFF0000";


    -------------------------------------------------------
    --!> Stats
    -------------------------------------------------------
    signal write_cnt : unsigned(63 downto 0) := (others=>'0');
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
    --!> CodecSetting
    -------------------------------------------------------
    type CodecSettingType is (CodecSettingUndefined, CodecSettingDecimate, CodecSettingInterpolate);
    signal CodecSettingReg : CodecSettingType := CodecSettingUndefined;
    signal CodecFactorReg : unsigned(7 downto 0);
    signal codec_factor_cnt_reg, codec_factor_cnt_next : unsigned(7 downto 0) := (others=>'0');
    signal decimate_cnt : unsigned(7 downto 0) := (others=>'0');

    signal current_decimated_word : std_logic_vector(31 downto 0) := (others => '0');


    signal doWriteSignal, doWriteSignalSet, doWriteSignalClear : std_logic := '0';


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
        CodecSetting,        
        CodecFactor,        
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
    --!> GET_NIBBLE
    -------------------------------------------------------
    function GET_NIBBLE(
        idx  : in integer;  -- 0 is msb
        word : in std_logic_vector(31 downto 0)
    ) return std_logic_vector is
        variable ret :std_logic_vector(3 downto 0);
    begin
        ret  := word((idx)*4 + 3) 
              & word((idx)*4 + 2) 
              & word((idx)*4 + 1) 
              & word((idx)*4 + 0);
        return ret;
    end function;

    -------------------------------------------------------
    --!> GET_INTERPOLATED_BYTE
    -------------------------------------------------------
    function GET_BYTE(
        idx  : in integer; 
        word                : in std_logic_vector(31 downto 0)
    ) return std_logic_vector is
        variable ret :std_logic_vector(7 downto 0);
    begin
        ret  := word((3 - idx)*8 + 7) 
              & word((3 - idx)*8 + 6) 
              & word((3 - idx)*8 + 5) 
              & word((3 - idx)*8 + 4) 
              & word((3 - idx)*8 + 3) 
              & word((3 - idx)*8 + 2) 
              & word((3 - idx)*8 + 1) 
              & word((3 - idx)*8 + 0);        
        return ret;
    end function;

    -------------------------------------------------------
    --!> GET_INTERPOLATED_WORD
    -------------------------------------------------------
    function GET_INTERPOLATED_WORD(
        interpolationCount  : in unsigned(7 downto 0); 
        word                : in std_logic_vector(31 downto 0);
        interpolationFactor : in unsigned(7 downto 0)
    ) return std_logic_vector is
        variable ret :std_logic_vector(31 downto 0);
    begin
        if( interpolationFactor = to_unsigned(8, interpolationFactor'length)) then
            ret :=  GET_NIBBLE((to_integer(interpolationCount)), word ) &
                    GET_NIBBLE((to_integer(interpolationCount)), word ) &
                    GET_NIBBLE((to_integer(interpolationCount)), word ) &
                    GET_NIBBLE((to_integer(interpolationCount)), word ) &
                    GET_NIBBLE((to_integer(interpolationCount)), word ) &
                    GET_NIBBLE((to_integer(interpolationCount)), word ) &
                    GET_NIBBLE((to_integer(interpolationCount)), word ) &
                    GET_NIBBLE((to_integer(interpolationCount)), word );

        elsif( interpolationFactor = to_unsigned(4, interpolationFactor'length)) then
        ret :=  GET_NIBBLE((to_integer(interpolationCount)*2)+1, word ) &
                GET_NIBBLE((to_integer(interpolationCount)*2), word ) &
                GET_NIBBLE((to_integer(interpolationCount)*2)+1, word ) &
                GET_NIBBLE((to_integer(interpolationCount)*2), word ) &
                GET_NIBBLE((to_integer(interpolationCount)*2)+1, word ) &
                GET_NIBBLE((to_integer(interpolationCount)*2), word ) &
                GET_NIBBLE((to_integer(interpolationCount)*2)+1, word ) &
                GET_NIBBLE((to_integer(interpolationCount)*2), word );

        elsif( interpolationFactor = to_unsigned(2, interpolationFactor'length)) then
            ret :=  GET_NIBBLE((to_integer(interpolationCount)*4)+3, word ) &
                    GET_NIBBLE((to_integer(interpolationCount)*4)+2, word ) &
                    GET_NIBBLE((to_integer(interpolationCount)*4)+3, word ) &
                    GET_NIBBLE((to_integer(interpolationCount)*4)+2, word ) &
                    GET_NIBBLE((to_integer(interpolationCount)*4)+1, word ) &
                    GET_NIBBLE((to_integer(interpolationCount)*4)+0, word ) &
                    GET_NIBBLE((to_integer(interpolationCount)*4)+1, word ) &
                    GET_NIBBLE((to_integer(interpolationCount)*4)+0, word );

        else 
            ret := (others => '0');
        end if;
        return ret;
    end GET_INTERPOLATED_WORD;

    -------------------------------------------------------
    --!> GET_DECIMATED_WORD
    -------------------------------------------------------
    function GET_DECIMATED_WORD(
        decimationCount  : in unsigned(7 downto 0); 
        currentWord      : in std_logic_vector(31 downto 0);
        currentInput     : in std_logic_vector(31 downto 0);
        decimationFactor : in unsigned(7 downto 0)
    ) return std_logic_vector is
        variable ret :std_logic_vector(31 downto 0);
    begin
        if( decimationFactor = to_unsigned(4, decimationFactor'length)) then
            if (decimationCount = to_unsigned(0, decimationCount'length)) then
                ret :=  std_logic_vector(to_unsigned(0, 24)) & 
                        GET_NIBBLE(7, currentInput ) &
                        GET_NIBBLE(6, currentInput );
            elsif (decimationCount = to_unsigned(1, decimationCount'length)) then
                ret :=  std_logic_vector(to_unsigned(0, 16)) & 
                        GET_NIBBLE(7, currentInput ) &
                        GET_NIBBLE(6, currentInput ) &
                        currentWord(7 downto 0);
            elsif (decimationCount = to_unsigned(2, decimationCount'length)) then
                ret :=  std_logic_vector(to_unsigned(0, 8)) &
                        GET_NIBBLE(7, currentInput ) &
                        GET_NIBBLE(6, currentInput ) &
                        currentWord(15 downto 0);
            elsif (decimationCount = to_unsigned(3, decimationCount'length)) then
                ret :=  GET_NIBBLE(7, currentInput ) &
                        GET_NIBBLE(6, currentInput ) &
                        currentWord(23 downto 0); 
            end if;
        else 
            ret := (others => '1');
        end if;
        return ret;
    end GET_DECIMATED_WORD;

    -------------------------------------------------------
    --!> GET_NEXT_CODEC_COUNT
    -------------------------------------------------------
    pure function GET_NEXT_CODEC_COUNT(
        currCount : in unsigned(7 downto 0);
        maxCount : in unsigned(7 downto 0)
    ) return unsigned is
        variable ret :unsigned(7 downto 0);
    begin
        if ((currCount + 1) >= maxCount ) then
            ret := to_unsigned(0, 8);
        else
            ret := (currCount + 1);
        end if;

        return ret;
    end GET_NEXT_CODEC_COUNT;

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
            if( reset = '1') then
                -- current_decimated_word <= (others => '0');
                decimate_cnt <= (others => '0');
                write_cnt <= (others => '0');
            end if;

            -- write_cnt_next <= write_cnt_reg;
            softResetSignalSet <= '0';
            -- fifo_write_en <= '0';
            doWriteSignalSet <= '0';
            -- decimate_cnt_next <= decimate_cnt_reg;

            if avs_s0_write='1' then
                case RegisterNames'Val(to_integer(unsigned(avs_s0_address))) is
                    when SoftReset =>
                        if( avs_s0_writedata = SOFT_RESET_CODE) then
                            softResetSignalSet <= '1';
                            soft_reset_counter <= soft_reset_counter + to_unsigned(1, 32);

                            decimate_cnt <= (others => '0');
                            write_cnt <= (others => '0');
                        end if;
                    when WriteData =>
                        if( fifo_full = '0') then
                            if( (CodecSettingReg = CodecSettingDecimate) and CodecFactorReg /= to_unsigned(1,8) ) then
                                current_decimated_word <= GET_DECIMATED_WORD(decimate_cnt, current_decimated_word, avs_s0_writedata, CodecFactorReg);
                                decimate_cnt <= GET_NEXT_CODEC_COUNT(decimate_cnt, CodecFactorReg);

                                if( (unsigned(decimate_cnt) = (CodecFactorReg - 1))) then
                                    doWriteSignalSet <= '1';
                                end if;
                                write_cnt <= write_cnt + to_unsigned(1, 64);
                            else
                                doWriteSignalSet <= '1';
                                current_decimated_word <= avs_s0_writedata;
                                write_cnt <= write_cnt + to_unsigned(1, 64);
                            end if;
                        end if;
                    when CodecSetting =>
                        if( avs_s0_writedata = CODEC_SETTING_INTERPOLATE_CODE) then
                            CodecSettingReg <= CodecSettingInterpolate;
                        elsif( avs_s0_writedata = CODEC_SETTING_DECIMATE_CODE) then
                            CodecSettingReg <= CodecSettingDecimate;
                        end if;
                    when CodecFactor =>
                            CodecFactorReg <= unsigned(avs_s0_writedata(7 downto 0));
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
        avs_s0_read, avs_s0_address, write_cnt, reset_counter, soft_reset_counter,
        read_cnt_reg, fifo_full, fifo_empty, fifo_read_data, CodecSettingReg, CodecFactorReg
    ) 
    begin
        codec_factor_cnt_next <= codec_factor_cnt_reg;
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
                    avs_s0_readdata <= std_logic_vector(write_cnt(63 downto 32));
                when WritesPerformedLower =>
                    avs_s0_readdata <= std_logic_vector(write_cnt(31 downto 0));
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
                        if( (CodecSettingReg = CodecSettingInterpolate) and CodecFactorReg /= to_unsigned(1,8) ) then
                            avs_s0_readdata <= GET_INTERPOLATED_WORD(codec_factor_cnt_reg, fifo_read_data, CodecFactorReg);
                            codec_factor_cnt_next <= GET_NEXT_CODEC_COUNT(codec_factor_cnt_reg, CodecFactorReg);

                            if( (unsigned(codec_factor_cnt_next) = to_unsigned(0, codec_factor_cnt_next'length))) then
                                fifo_read_en_next <= '1';
                            end if;
                        else
                            fifo_read_en_next <= '1';
                            avs_s0_readdata <= fifo_read_data;
                        end if;
                        read_cnt_next <= read_cnt_reg + to_unsigned(1, 64);
                    end if;

                when CodecSetting =>
                    case CodecSettingReg is
                        when CodecSettingUndefined =>
                            avs_s0_readdata <= (others => '0');
                        when CodecSettingDecimate =>
                            avs_s0_readdata <= CODEC_SETTING_DECIMATE_CODE;
                        when CodecSettingInterpolate =>
                            avs_s0_readdata <= CODEC_SETTING_INTERPOLATE_CODE;
                    end case;
                when CodecFactor =>
                    avs_s0_readdata <= x"000000" & std_logic_vector(CodecFactorReg);
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

                read_cnt_reg <= (others => '0');
                reset_counter <= reset_counter + to_unsigned(1, 32);
                fifo_read_en_reg <= '0';
                codec_factor_cnt_reg <= (others => '0');
            else
                softResetSignalClear <= '0';

                read_cnt_reg <= read_cnt_next;

                reset_counter <= reset_counter;
                fifo_read_en_reg <= fifo_read_en_next;

                codec_factor_cnt_reg <= codec_factor_cnt_next;
            end if;
        end if;
    end process;

    -------------------------------------------------------
    --! PROCESS: Do Write
    -------------------------------------------------------
    process(
        clk, reset
    )
    begin
        if(rising_edge(clk)) then
            if( doWriteSignal = '1' and fifo_write_en /= '1') then
                doWriteSignalClear <= '1';
                fifo_write_data <= current_decimated_word;
                fifo_write_en <= '1';
            elsif( doWriteSignal = '1') then
                doWriteSignalClear <= '1';
                fifo_write_en <= '0';
            else
                fifo_write_en <= '0';
                doWriteSignalClear <= '0';
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

    -------------------------------------------------------
    --!> Do Write Signal Manager
    -------------------------------------------------------
    DoWriteSignalManager : entity work.SignalManager(Behavioral)
    port map
    (
        clk => clk,
        reset => reset,
        sig => doWriteSignal,
        set => doWriteSignalSet,
        clear => doWriteSignalClear
    );

end architecture Behavioral; 
