library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity ModMCounter is
	port (
        clk : in std_logic;
        reset : in std_logic;
        clear : in std_logic;
        en : in std_logic;
        m : in std_logic_vector(31 downto 0);
        q : out std_logic_vector(31 downto 0);
        max_tick : out std_logic);
end entity ModMCounter;

architecture Behavioral of ModMCounter is

    signal r_reg, r_next : unsigned(31 downto 0) := (others => '0');

begin

    process(clk, reset)
    begin
        if( reset = '1') then
            r_reg <= (others => '0');
        elsif(rising_edge(clk)) then
            if( clear = '1') then
                r_reg <= (others => '0');
            elsif( en = '1') then
                r_reg <= r_next;
            end if;
        end if;
    end process;

    r_next <= (others => '0') when r_reg = (unsigned(m) - 1) else
            r_reg + 1;

    max_tick <= '1' when r_reg = (unsigned(m) -1) else
                '0';

    q <= std_logic_vector(r_reg);

end Behavioral;