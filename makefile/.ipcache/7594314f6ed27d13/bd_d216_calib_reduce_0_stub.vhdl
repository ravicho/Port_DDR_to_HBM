-- Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2020.2.0 (lin64) Build 2963987 Mon Aug 10 20:20:07 MDT 2020
-- Date        : Tue Aug 11 10:03:47 2020
-- Host        : xsjrdevl110 running 64-bit CentOS Linux release 7.4.1708 (Core)
-- Command     : write_vhdl -force -mode synth_stub -rename_top decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix -prefix
--               decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix_ bd_d216_calib_reduce_0_stub.vhdl
-- Design      : bd_d216_calib_reduce_0
-- Purpose     : Stub declaration of top-level module interface
-- Device      : xcu200-fsgd2104-2-e
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix is
  Port ( 
    Op1 : in STD_LOGIC_VECTOR ( 2 downto 0 );
    Res : out STD_LOGIC
  );

end decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix;

architecture stub of decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix is
attribute syn_black_box : boolean;
attribute black_box_pad_pin : string;
attribute syn_black_box of stub : architecture is true;
attribute black_box_pad_pin of stub : architecture is "Op1[2:0],Res";
attribute X_CORE_INFO : string;
attribute X_CORE_INFO of stub : architecture is "util_reduced_logic_v2_0_4_util_reduced_logic,Vivado 2020.2.0";
begin
end;
