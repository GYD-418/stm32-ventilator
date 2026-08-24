LIBRARY IEEE;
USE IEEE.std_logic_1164.ALL;

-- 只保留一个实体定义
ENTITY comparator IS
  PORT (
    a     : IN  std_logic_vector(7 DOWNTO 0);
    b     : IN  std_logic_vector(7 DOWNTO 0);
    equal : OUT std_logic
  );
END comparator;

-- 只保留一个架构定义
ARCHITECTURE behavioral OF comparator IS
BEGIN
  comp_proc : PROCESS(a, b)
  BEGIN
    IF a = b THEN
      equal <= '1';
    ELSE
      equal <= '0';
    END IF;
  END PROCESS comp_proc;
END behavioral;
