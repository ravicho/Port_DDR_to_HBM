#!/bin/sh

# 
# v++(TM)
# runme.sh: a v++-generated Runs Script for UNIX
# Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
# 

if [ -z "$PATH" ]; then
  PATH=/proj/xbuilds/SWIP/2020.2_1109_1932/installs/lin64/Vitis_HLS/2020.2/bin:/proj/xbuilds/SWIP/2020.2_1109_1932/installs/lin64/Vitis/2020.2/bin:/proj/xbuilds/SWIP/2020.2_1109_1932/installs/lin64/Vitis/2020.2/bin
else
  PATH=/proj/xbuilds/SWIP/2020.2_1109_1932/installs/lin64/Vitis_HLS/2020.2/bin:/proj/xbuilds/SWIP/2020.2_1109_1932/installs/lin64/Vitis/2020.2/bin:/proj/xbuilds/SWIP/2020.2_1109_1932/installs/lin64/Vitis/2020.2/bin:$PATH
fi
export PATH

if [ -z "$LD_LIBRARY_PATH" ]; then
  LD_LIBRARY_PATH=
else
  LD_LIBRARY_PATH=:$LD_LIBRARY_PATH
fi
export LD_LIBRARY_PATH

HD_PWD='/wrk/xsjhdnobkup5/ravic/work/Port_DDR_to_HBM/build/HBM3/temp_dir/vadd_hw/vadd'
cd "$HD_PWD"

HD_LOG=runme.log
/bin/touch $HD_LOG

ISEStep="./ISEWrap.sh"
EAStep()
{
     $ISEStep $HD_LOG "$@" >> $HD_LOG 2>&1
     if [ $? -ne 0 ]
     then
         exit
     fi
}

EAStep vitis_hls -f vadd.tcl -messageDb vitis_hls.pb
