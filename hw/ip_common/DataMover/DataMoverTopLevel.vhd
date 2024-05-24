library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

-------------------------------------------------------
--! DataMover Entity
-------------------------------------------------------
entity DataMoverTopLevel is
	port (
        c_arcache : out std_logic_vector(3 downto 0) := (others => '0');
        c_arprot  : out std_logic_vector(2 downto 0) := (others => '0');
        c_aruser  : out std_logic_vector(4 downto 0) := (others => '0');
        c_awcache : out std_logic_vector(3 downto 0) := (others => '0');
        c_awprot  : out std_logic_vector(2 downto 0) := (others => '0');
        c_awuser  : out std_logic_vector(4 downto 0) := (others => '0');

        -- avalon_master
		avalon_master_address       : out std_logic_vector(31 downto 0) := (others => '0'); 
		avalon_master_burstcount    : out std_logic_vector(7 downto 0)  := (others => '0');  
		avalon_master_byteenable    : out std_logic_vector(7 downto 0)  := (others => '0');  
		avalon_master_waitrequest   : in  std_logic;                                          
		avalon_master_write         : out std_logic                     := '0';             
		avalon_master_writedata     : out std_logic_vector(63 downto 0) := (others => '0'); 
		avalon_master_read          : out std_logic                     := '0';             
		avalon_master_readdatavalid : in  std_logic                     := '0';              
		avalon_master_readdata      : in  std_logic_vector(63 downto 0) := (others => '0');  

		avs_s0_address     : in  std_logic_vector(10 downto 0);
		avs_s0_read        : in  std_logic;            
		avs_s0_readdata    : out std_logic_vector(31 downto 0);                   
		avs_s0_waitrequest : out std_logic;                                       
		avs_s0_write       : in  std_logic;            
		avs_s0_writedata   : in  std_logic_vector(31 downto 0);
		ins_irq0_irq       : out std_logic;                                       
		reset              : in  std_logic;            
		clk                : in  std_logic);
end entity DataMoverTopLevel;

-------------------------------------------------------
--! Architecture of DataMover
-------------------------------------------------------
architecture RTL of DataMoverTopLevel is
    -------------------------------------------------------
    --! Constants
    -------------------------------------------------------
    constant MAGIC_NUMBER : std_logic_vector(31 downto 0) := x"2224ABCD";
    constant SOFT_RESET_CODE : std_logic_vector(31 downto 0) := x"FF00FF00";
    constant WRITE_ENABLE_CODE : std_logic_vector(31 downto 0) := x"FFFFFFFF";
    constant READ_ENABLE_CODE : std_logic_vector(31 downto 0) := x"FFFFFFFF";
    constant READ_STOP_FLAG_CODE : std_logic_vector(31 downto 0) := x"12345678";
    constant READ_UNDERRUN_FLAG_IS_SET_CODE : std_logic_vector(31 downto 0) := x"89271203";
    constant CLEAR_UNDERRUN_FLAG_CODE : std_logic_vector(31 downto 0) := x"87654321";
    constant RESET_READ_ADDRESS_CODE : std_logic_vector(31 downto 0) := x"83920394";
    constant READ_IN_PROGRESS_CODE : std_logic_vector(31 downto 0) := x"B0B0A0A0";
    constant READ_NOT_IN_PROGRESS_CODE : std_logic_vector(31 downto 0) := x"A0A0B0B0";
    constant WRITEVAL_MIN : unsigned(7 downto 0) := x"01";
    constant WRITEVAL_MAX : unsigned(7 downto 0) := x"FA";

    constant COMMAND_START_READ : std_logic_vector(31 downto 0) := x"00001111";
    constant COMMAND_START_WRITE : std_logic_vector(31 downto 0) := x"FFFF0000";
    constant COMMAND_STOP : std_logic_vector(31 downto 0) := x"FFFFFFFF";

    -------------------------------------------------------
    --! Avalon Master Latches
    -------------------------------------------------------
    signal avs_master_address_reg, avs_master_address_next : std_logic_vector(31 downto 0) := (others => '0');
    signal avs_master_writedata_reg, avs_master_writedata_next : std_logic_vector(63 downto 0) := (others => '0');
    signal avs_master_write_reg, avs_master_write_next: std_logic := '0';
    signal avs_master_read_reg, avs_master_read_next: std_logic := '0';

    -------------------------------------------------------
    --! DelayCounter Signals
    -------------------------------------------------------
    signal delayCounterClear : std_logic := '0';
    signal delayCounterEnable : std_logic := '0';
    signal delayCounterDone : std_logic := '0';
    signal delayCounterLimit : std_logic_vector(31 downto 0) := (others => '0');

    -------------------------------------------------------
    --!> WRITE Signals
    -------------------------------------------------------
    signal write_addr_reg, write_addr_next : std_logic_vector(31 downto 0) := (others => '0');
    signal write_cnt_reg, write_cnt_next : unsigned(63 downto 0) := (others=>'0');
    signal writeval_reg, writeval_next : unsigned(7 downto 0) := WRITEVAL_MIN;

    -------------------------------------------------------
    --!> READ Signals
    -------------------------------------------------------
    signal read_addr_reg, read_addr_next : std_logic_vector(31 downto 0) := (others => '0');
    signal read_cnt_reg, read_cnt_next : unsigned(63 downto 0) := (others=>'0');
    signal exp_readval_reg, exp_readval_next : unsigned(7 downto 0) := WRITEVAL_MIN;
    signal read_error_cnt_reg, read_error_cnt_next : unsigned(31 downto 0) := (others => '0');
    signal last_received_upper_reg, last_received_upper_next : std_logic_vector(31 downto 0) := (others => '0');
    signal last_received_lower_reg, last_received_lower_next : std_logic_vector(31 downto 0) := (others => '0');

    signal verify_tx_signal : std_logic;

    -------------------------------------------------------
    --!> SoftReset
    -------------------------------------------------------
    signal softResetSignal      : std_logic := '0';
    signal softResetSignalSet   : std_logic := '0';
    signal softResetSignalClear : std_logic := '0';

    -------------------------------------------------------
    --!> TriggerReadOperation
    -------------------------------------------------------
    signal triggerReadSignal      : std_logic := '0';
    signal triggerReadSignalSet   : std_logic := '0';
    signal triggerReadSignalClear : std_logic := '0';
    signal readInProgress_reg, readInProgress_next : std_logic_vector(31 downto 0) := READ_NOT_IN_PROGRESS_CODE;

    -------------------------------------------------------
    --!> UnderrunFlag
    -------------------------------------------------------
    signal underrunFlag             : std_logic := '0';
    signal underrunFlagSet          : std_logic := '0';
    signal underrunFlagClear        : std_logic := '0';

    -------------------------------------------------------
    --!> Read stop flag
    -------------------------------------------------------
    signal readStopFlag             : std_logic := '0';
    signal readStopFlagSet          : std_logic := '0';
    signal readStopFlagClear        : std_logic := '0';


    signal reset_counter : unsigned(31 downto 0) := (others =>'0');
    signal soft_reset_counter : unsigned(31 downto 0) := (others =>'0');

    -------------------------------------------------------
    --! State Machine Setup
    -------------------------------------------------------
    type States is (
        InitState, 
        IdleState, 
        InitReadState,
        StartWriteState, 
        StartReadState, 
        WritingState, 
        ReadingState, 
        WaitState
    ); 
    signal state_reg, state_next : States := InitState;

    -------------------------------------------------------
    --! Register Setup
    -------------------------------------------------------
    type RegisterNames is (
        --Status
        MagicNumber,              
        CurrentWriteAddress,     
        CurrentReadAddress,     
        WritesPerformedUpper,        
        WritesPerformedLower,        
        ReadsPerformedUpper,        
        ReadsPerformedLower,        
        ReadErrorCount,        
        ReadUnderrunFlag,
        LastReceivedReadUpper,        
        LastReceivedReadLower,        
        CurrentState,       
        --Settings
        WriteBufferStartAddress,
        WriteBufferSize,       
        ReadBufferStartAddress,
        ReadBufferSize,       
        ReadStopPointAddress,       
        SetReadStopFlag,
        ClearUnderrunFlag,
        ResetReadBufferOnEnable,
        WriteEnable,          
        ReadEnable,          
        WriteTickInterval,   
        ReadTickInterval,   
        SoftReset,        
        ResetCounter,        
        SoftResetCounter,        
        RegisterLast     
    );
    type RegisterType is array(RegisterNames range <>) of 
        std_logic_vector(31 downto 0);

    signal RegisterMap : RegisterType(RegisterNames) := (
        MAGIC_NUMBER,
        others => (others => '0')
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
    --!> GET_NEXT_WRITEVAL
    -------------------------------------------------------
    impure function GET_NEXT_WRITEVAL(
        writeval : in unsigned(7 downto 0)
    ) return unsigned is
    begin
        if( writeval = WRITEVAL_MAX) then
            return WRITEVAL_MIN;
        else
            return (writeval + to_unsigned(1,8));
        end if;
    end GET_NEXT_WRITEVAL;

    -------------------------------------------------------
    --!> GET_NEXT_ADDRESS
    -------------------------------------------------------
    pure function GET_NEXT_ADDRESS(
        currAddr : in std_logic_vector(31 downto 0);
        startAddr : in std_logic_vector(31 downto 0);
        bufferSize : in std_logic_vector(31 downto 0)
    ) return std_logic_vector is
        constant ADDR_INCREMENT : unsigned(31 downto 0) := to_unsigned(8, 32);
        variable maxAddress : unsigned(31 downto 0);

    begin
        maxAddress := (unsigned(startAddr) + unsigned(bufferSize));
        if ((unsigned(currAddr) + ADDR_INCREMENT) >= maxAddress ) then
            return startAddr;
        else
            return std_logic_vector(unsigned(currAddr) + ADDR_INCREMENT);
        end if;
    end GET_NEXT_ADDRESS;

    -------------------------------------------------------
    --!> GET_RX_DATA
    -------------------------------------------------------
    pure function GET_RX_DATA(
        writeval : in unsigned(7 downto 0)
    ) return std_logic_vector is
    begin
        return  std_logic_vector(writeval) & 
                std_logic_vector(writeval) &
                std_logic_vector(writeval) &
                std_logic_vector(writeval) &
                std_logic_vector(writeval) &
                std_logic_vector(writeval) &
                std_logic_vector(writeval) &
                std_logic_vector(writeval);
    end GET_RX_DATA;

    -------------------------------------------------------
    --!> EXPECTED_TX_DATA
    -------------------------------------------------------
    pure function EXPECTED_TX_DATA(
        exp_readval: in unsigned(7 downto 0)
    ) return std_logic_vector is
    begin
        return  std_logic_vector(exp_readval) & 
                std_logic_vector(exp_readval) &
                std_logic_vector(exp_readval) &
                std_logic_vector(exp_readval) &
                std_logic_vector(exp_readval) &
                std_logic_vector(exp_readval) &
                std_logic_vector(exp_readval) &
                std_logic_vector(exp_readval);
    end EXPECTED_TX_DATA;


-------------------------------------------------------
--! BEGIN
-------------------------------------------------------
begin
    -------------------------------------------------------
    --! PROCESS: Register Write
    -------------------------------------------------------
    process (clk, reset, avs_s0_address, avs_s0_writedata) 
    begin

        if( reset = '1') then
            RegisterMap <= (MAGIC_NUMBER, others => (others => '0'));

        --Only synchronous writes
        elsif rising_edge(clk) then
            underrunFlagClear <= '0';
            readStopFlagSet <= '0';
            softResetSignalSet <= '0';
            triggerReadSignalSet <= '0';

            if avs_s0_write='1' then
                case RegisterNames'Val(to_integer(unsigned(avs_s0_address))) is
                    when SoftReset =>
                        if( avs_s0_writedata = SOFT_RESET_CODE) then
                            softResetSignalSet <= '1';
                            RegisterMap <= (MAGIC_NUMBER, others => (others => '0'));
                            soft_reset_counter <= soft_reset_counter + to_unsigned(1, 32);
                        end if;

                    when ReadEnable =>
                        if( avs_s0_writedata = READ_ENABLE_CODE) then
                            triggerReadSignalSet <= '1';
                        end if;
                    when ClearUnderrunFlag =>
                        if( avs_s0_writedata = CLEAR_UNDERRUN_FLAG_CODE) then
                            underrunFlagClear <= '1';
                        end if;
                    when SetReadStopFlag =>
                        if( avs_s0_writedata = READ_STOP_FLAG_CODE) then
                            readStopFlagSet <= '1';
                        end if;
                    when others =>
                        RegisterMap(RegisterNames'Val(to_integer(unsigned(avs_s0_address)))) <= avs_s0_writedata;
                end case;
            end if;
        end if;
    end process;

    -------------------------------------------------------
    --! PROCESS: Register Read
    -------------------------------------------------------
    process (
        avs_s0_read, avs_s0_address, state_reg, 
        write_addr_reg, write_cnt_reg, reset_counter, soft_reset_counter,
        read_addr_reg, read_cnt_reg, read_error_cnt_reg, last_received_upper_reg,
        last_received_lower_reg, RegisterMap
    ) 
    begin
        avs_s0_readdata <= (others => '0');
        avs_s0_waitrequest <= '0';
        if(avs_s0_read='1') then 
            case RegisterNames'Val(to_integer(unsigned(avs_s0_address))) is
                when CurrentState =>
                    avs_s0_readdata <= std_logic_vector(to_unsigned(States'Pos(state_reg), 32));

                when CurrentWriteAddress =>
                    avs_s0_readdata <= write_addr_reg;

                when WritesPerformedUpper =>
                    avs_s0_readdata <= std_logic_vector(write_cnt_reg(63 downto 32));

                when WritesPerformedLower =>
                    avs_s0_readdata <= std_logic_vector(write_cnt_reg(31 downto 0));

                when ResetCounter =>
                    avs_s0_readdata <= std_logic_vector(reset_counter);

                when SoftResetCounter =>
                    avs_s0_readdata <= std_logic_vector(soft_reset_counter);

                when CurrentReadAddress =>
                    avs_s0_readdata <= read_addr_reg;

                when ReadsPerformedUpper =>
                    avs_s0_readdata <= std_logic_vector(read_cnt_reg(63 downto 32));

                when ReadsPerformedLower =>
                    avs_s0_readdata <= std_logic_vector(read_cnt_reg(31 downto 0));

                when ReadErrorCount =>
                    avs_s0_readdata <= std_logic_vector(read_error_cnt_reg);

                when ReadUnderrunFlag =>
                    if( underrunFlag = '1') then
                        avs_s0_readdata <= READ_UNDERRUN_FLAG_IS_SET_CODE;
                    else
                        avs_s0_readdata <= (others => '0');
                    end if;

                when LastReceivedReadUpper =>
                    avs_s0_readdata <= last_received_upper_reg;

                when LastReceivedReadLower =>
                    avs_s0_readdata <= last_received_lower_reg;

                when ReadEnable =>
                    avs_s0_readdata <= readInProgress_reg;

                when others =>
                    if( IS_VALID_REG(avs_s0_address) = '1') then
                        avs_s0_readdata <= RegisterMap(RegisterNames'Val(to_integer(unsigned(avs_s0_address))));
                    end if;
            end case;
        end if;
    end process;

    -------------------------------------------------------
    --! PROCESS: Main Update
    -------------------------------------------------------
    process(
        clk, reset, state_next, writeval_next, write_cnt_next, softResetSignal,
        read_addr_next, exp_readval_next, read_cnt_next, read_error_cnt_next, 
        last_received_upper_next, last_received_lower_next, avs_master_address_next,
        avs_master_writedata_next, avs_master_write_next, avs_master_read_next,
        readInProgress_next
    )
    begin
        if(rising_edge(clk)) then
            if( reset = '1' or softResetSignal = '1') then
                if( softResetSignal = '1') then
                    softResetSignalClear <= '1';
                end if;

                state_reg <= InitState;

                write_addr_reg <= (others => '0');
                write_cnt_reg <= (others => '0');
                writeval_reg <= WRITEVAL_MIN;

                readInProgress_reg <= READ_NOT_IN_PROGRESS_CODE;
                read_addr_reg <= (others => '0');
                read_cnt_reg <= (others => '0');
                exp_readval_reg <= WRITEVAL_MIN;
                read_error_cnt_reg <= (others => '0');
                last_received_upper_reg <= (others => '0');
                last_received_lower_reg <= (others => '0');
                reset_counter <= reset_counter + to_unsigned(1, 32);

                avs_master_address_reg <= (others => '0');
                avs_master_writedata_reg <= (others => '0');
                avs_master_write_reg <= '0';
                avs_master_read_reg <= '0';
            else
                softResetSignalClear <= '0';
                state_reg <= state_next;

                write_addr_reg <= write_addr_next;
                writeval_reg <= writeval_next;
                write_cnt_reg <= write_cnt_next;

                readInProgress_reg <= readInProgress_next;
                read_addr_reg <= read_addr_next;
                exp_readval_reg <= exp_readval_next;
                read_cnt_reg <= read_cnt_next;
                read_error_cnt_reg <= read_error_cnt_next;
                last_received_upper_reg <= last_received_upper_next;
                last_received_lower_reg <= last_received_lower_next;
                reset_counter <= reset_counter;

                avs_master_address_reg <= avs_master_address_next;
                avs_master_writedata_reg <= avs_master_writedata_next;
                avs_master_write_reg <= avs_master_write_next;
                avs_master_read_reg <= avs_master_read_next;
            end if;
        end if;
    end process;

    -------------------------------------------------------
    --! PROCESS: Next State & Output Logic
    -------------------------------------------------------
    process(
        avs_master_address_reg, avs_master_writedata_reg,
        avs_master_write_reg, avs_master_read_reg,
        state_reg, write_addr_reg, write_cnt_reg, 
        writeval_reg, avalon_master_waitrequest, delayCounterDone,
        read_cnt_reg, exp_readval_reg, read_addr_reg, read_error_cnt_reg,
        avalon_master_readdatavalid, last_received_upper_reg, last_received_lower_reg,
        RegisterMap, triggerReadSignal, avalon_master_readdata, readInProgress_reg
        )
    begin
        -- Defaults
        avs_master_address_next <= avs_master_address_reg;
        avs_master_writedata_next <= avs_master_writedata_reg;
        avs_master_write_next <= avs_master_write_reg;
        avs_master_read_next <= avs_master_read_reg;

        readStopFlagClear <= '0';
        underrunFlagSet <= '0';
        triggerReadSignalClear <= '0';
        delayCounterEnable <= '0';
        delayCounterClear <= '0';
        state_next <= state_reg;
        write_addr_next <= write_addr_reg;
        writeval_next <= writeval_reg;
        write_cnt_next <= write_cnt_reg;

        readInProgress_next <= readInProgress_reg;
        read_addr_next <= read_addr_reg;
        exp_readval_next <= exp_readval_reg;
        read_cnt_next <= read_cnt_reg;
        read_error_cnt_next <= read_error_cnt_reg;
        last_received_upper_next <= last_received_upper_reg;
        last_received_lower_next <= last_received_lower_reg;

        case state_reg is
            -------------------------------------------------------
            --! InitState
            -------------------------------------------------------
            when InitState =>
                write_addr_next <= RegisterMap(WriteBufferStartAddress);
                read_addr_next <= RegisterMap(ReadBufferStartAddress);

                if (triggerReadSignal = '1') or (RegisterMap(WriteEnable) = WRITE_ENABLE_CODE)then
                    state_next <= IdleState;
                end if;

            -------------------------------------------------------
            --! IdleState
            -------------------------------------------------------
            when IdleState =>
                if triggerReadSignal = '1' then
                    triggerReadSignalClear <= '1';
                    state_next <= InitReadState;
                elsif RegisterMap(WriteEnable) = WRITE_ENABLE_CODE then
                    state_next <= StartWriteState;
                end if;

            -------------------------------------------------------
            --! InitReadState
            -------------------------------------------------------
            when InitReadState =>
                if( RESET_READ_ADDRESS_CODE = RegisterMap(ResetReadBufferOnEnable)) then
                    read_addr_next <= RegisterMap(ReadBufferStartAddress);
                end if;
                readInProgress_next <= READ_IN_PROGRESS_CODE;
                state_next <= StartReadState;

            -------------------------------------------------------
            --! StartReadState
            -------------------------------------------------------
            when StartReadState =>
                avs_master_address_next <= read_addr_reg;
                avs_master_read_next <= '1';
                state_next <= ReadingState;

            -------------------------------------------------------
            --! StartWriteState
            -------------------------------------------------------
            when StartWriteState =>
                avs_master_address_next <= write_addr_reg;
                avs_master_writedata_next <= GET_RX_DATA(writeval_reg);
                avs_master_write_next <= '1';
                state_next <= WritingState;

            -------------------------------------------------------
            --! ReadingState
            -------------------------------------------------------
            when ReadingState =>
                if (avalon_master_readdatavalid = '1') then
                    avs_master_read_next <= '0';

                    last_received_upper_next <= avalon_master_readdata(63 downto 32);
                    last_received_lower_next <= avalon_master_readdata(31 downto 0);

                    --In case of error we try to fix the expected value
                    if(avalon_master_readdata /= EXPECTED_TX_DATA(exp_readval_reg)) then
                        read_error_cnt_next <= read_error_cnt_reg + to_unsigned(1, 32);
                        exp_readval_next <= GET_NEXT_WRITEVAL(unsigned(avalon_master_readdata(7 downto 0)));
                    else
                        exp_readval_next <= GET_NEXT_WRITEVAL(exp_readval_reg);
                    end if;

                    read_cnt_next <= read_cnt_reg + to_unsigned(1, 64);
                    read_addr_next <= GET_NEXT_ADDRESS(read_addr_reg, 
                                                        RegisterMap(ReadBufferStartAddress),
                                                        RegisterMap(ReadBufferSize)); 

                    -- Start DelayCounter and go to waitstate
                    delayCounterEnable <= '1';
                    delayCounterClear <= '1';
                    state_next <= WaitState;
                end if;

            -------------------------------------------------------
            --! WritingState
            -------------------------------------------------------
            when WritingState =>
                if (avalon_master_waitrequest = '0') then
                    avs_master_write_next <= '0';
                    writeval_next <= GET_NEXT_WRITEVAL(writeval_reg);

                    write_cnt_next <= write_cnt_reg + to_unsigned(1, 64);
                    write_addr_next <= GET_NEXT_ADDRESS(write_addr_reg, 
                                                        RegisterMap(WriteBufferStartAddress),
                                                        RegisterMap(WriteBufferSize)); 

                    -- Start DelayCounter and go to waitstate
                    delayCounterEnable <= '1';
                    delayCounterClear <= '1';
                    state_next <= WaitState;
                end if;

            -------------------------------------------------------
            --! WaitState
            -------------------------------------------------------
            when WaitState =>
                if( delayCounterDone = '0') then
                    -- If not done, keep asserting counter enable signal
                    delayCounterEnable <= '1';
                else
                    delayCounterEnable <= '0';
                    delayCounterClear <= '1';

                    if (readInProgress_reg = READ_IN_PROGRESS_CODE) then
                        if( read_addr_reg = RegisterMap(ReadStopPointAddress)) then
                            -- Stop flag set when reaching end, All good, go back to idle
                            if(readStopFlag = '1') then 
                                readStopFlagClear <= '1';
                                state_next <= IdleState;
                                readInProgress_next <= READ_NOT_IN_PROGRESS_CODE;
                            
                            -- Stop flag not set, means underrun, wait here until stop flag set or ??
                            else
                                underrunflagSet <= '1';
                            end if;
                        else
                            state_next <= StartReadState;
                        end if;
                    elsif( triggerReadSignal = '1') then
                        state_next <= IdleState;
                    elsif RegisterMap(WriteEnable) = WRITE_ENABLE_CODE then
                        state_next <= StartWriteState;
                    else
                        state_next <= IdleState;
                    end if;
                end if;

            -------------------------------------------------------
            --! Other States
            -------------------------------------------------------
            when others =>
                state_next <= state_reg;
        end case;
    end process;

    -------------------------------------------------------
    --!> AVALON Master Outputs
    -------------------------------------------------------
    avalon_master_address <= avs_master_address_reg;
    avalon_master_write <= avs_master_write_reg;
    avalon_master_writedata <= avs_master_writedata_reg;
    avalon_master_read <= avs_master_read_reg;
    avalon_master_byteenable <= "11111111";
    avalon_master_burstcount <= "00000001";
    c_arcache <= "1111";
    c_arprot  <= "000";
    c_aruser  <= "11111";
    c_awcache <= "1111";
    c_awprot  <= "000";
    c_awuser  <= "11111";

    -------------------------------------------------------
    --!> Delay Counter
    -------------------------------------------------------
    DelayCounter : entity work.ModMCounter(Behavioral)
    port map
    (
        clk => clk,
        reset => (reset or softResetSignal),
        clear => delayCounterClear,
        en => delayCounterEnable,
        m => RegisterMap(WriteTickInterval),
        q => open,
        max_tick => delayCounterDone
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
    --!> Trigger Read Signal Manager
    -------------------------------------------------------
    TriggerReadSignalManager : entity work.SignalManager(Behavioral)
    port map
    (
        clk => clk,
        reset => reset,
        sig => triggerReadSignal,
        set => triggerReadSignalSet,
        clear => triggerReadSignalClear
    );

    -------------------------------------------------------
    --!> Underrun flag signal manager
    -------------------------------------------------------
    UnderrunFlagSignalManager : entity work.SignalManager(Behavioral)
    port map
    (
        clk => clk,
        reset => reset or softResetSignal,
        sig => underrunFlag,
        set => underrunFlagSet,
        clear => underrunFlagClear
    );

    -------------------------------------------------------
    --!> Underrun flag signal manager
    -------------------------------------------------------
    ReadStopFlagSignalManager : entity work.SignalManager(Behavioral)
    port map
    (
        clk => clk,
        reset => reset or softResetSignal,
        sig => readStopFlag,
        set => readStopFlagSet,
        clear => readStopFlagClear
    );


end architecture RTL; 
