library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

-------------------------------------------------------
--! SoftReset Entity
-------------------------------------------------------
entity SignalManager is
	port (
		clk            : in  std_logic;
		reset          : in  std_logic;            
		sig            : out  std_logic := '0';                                          
		set            : in std_logic; 
		clear          : in std_logic);
end entity SignalManager;

-------------------------------------------------------
--! Architecture of SignalManager
-------------------------------------------------------
architecture Behavioral of SignalManager is
    signal localSignal      : std_logic := '0';

-------------------------------------------------------
--! BEGIN
-------------------------------------------------------
begin
    -------------------------------------------------------
    --! PROCESS: Update
    -------------------------------------------------------
    process (clk, reset, set, clear)
    begin
        if( reset = '1') then
            localSignal <= '0';

        elsif rising_edge(clk) then
            if(clear = '1') then
                localSignal <= '0';
            elsif(set = '1') then
                localSignal <= '1';
            end if;
        end if;
    end process;

    sig <= localSignal;
end architecture Behavioral; 
