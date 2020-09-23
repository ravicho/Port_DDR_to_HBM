CURRENT_DIR = $(PWD)
TARGET := hw
MEMTYPE := DDR


#PLATFORM :=xilinx_u200_xdma_201830_2
PLATFORM :=xilinx_u280_xdma_201920_3
PLATFORM :=xilinx_u50lv_gen3x4_xdma_base_2
PLATFORM :=xilinx_u50lv_gen3x4_xdma_2_202010_1

SRCDIR := ./../reference_files

ifeq ($(MEMTYPE), DDR)
PLATFORM :=xilinx_u200_xdma_201830_2
else
PLATFORM :=xilinx_u50lv_gen3x4_xdma_2_202010_1
endif


# Default Settings here ..
HOST_SRC_CPP := $(SRCDIR)/run_$(MEMTYPE).cpp
BUILDDIR := ./../build/$(MEMTYPE)


# Common Source Code for FPGA and Host
HOST_SRC_FPGA := $(SRCDIR)/kernel.cpp
HOST_SRC_CPP += $(SRCDIR)/xcl2.cpp

# SoC variables
RUN_APP_SCRIPT = run_app.sh
PACKAGE_OUT = package.$(TARGET)

LAUNCH_EMULATOR = $(PACKAGE_OUT)/launch_$(TARGET).sh
RESULT_STRING = TEST PASSED

VPP := v++
SDCARD := sd_card

include common.mk

#####################################################################################################
### Build the host executable. This step is always executed
#####################################################################################################
compile_host:  host
	@echo $(HOST_SRC_FPGA) is being used as source for Generating Kernel; \
	mkdir -p $(BUILDDIR)

build: compile_host ./$(BUILDDIR)/vadd_$(TARGET).xclbin $(BUILDDIR)/$(EMCONFIG_FILE)

#####################################################################################################
###   For HW run, use 100,000 words for computation 
###   For Emulation run, use 100 words for computation 
#####################################################################################################

run:   build host
	cp xrt.ini $(BUILDDIR)
ifeq ($(TARGET), hw)
	@echo "************  Use Command Line to run application!  ************"
	cd $(BUILDDIR) && ./host vadd_$(TARGET).xclbin $(DSIZE) $(ADDRNDM) $(KRNL_LOOP);
else
	@echo "Running $(TAGET) mode"; 
	emconfigutil --nd 1  --platform $(PLATFORM) --od $(BUILDDIR)
	cd $(BUILDDIR) && XCL_EMULATION_MODE=$(TARGET) ./host vadd_$(TARGET).xclbin $(DSIZE) $(ADDRNDM) $(KRNL_LOOP);
endif

ddr_addRandom:
	make run TARGET=hw MEMTYPE=DDR DSIZE=4096 ADDRNDM=1 KRNL_LOOP=64
hbm_addRandom:
	make run TARGET=hw MEMTYPE=HBM DSIZE=4096 ADDRNDM=1 KRNL_LOOP=64

help:
	@echo  " "
	@echo  " "
	@echo  " Makefile Usage:"
	@echo  " "
	@echo  "   Use the following command to Target application for sending DSize(kb) from Host to Device"
	@echo  "   ADDRNDM determines address to be random(1) or sequential(0) for Kernel accessing MEMTYPE" 
	@echo  "   KRNL_LOOP can be set to any num for repeating kernel execution # of times"
	@echo  " > make <build/run>    TARGET=<sw_emu/hw_emu/hw> MEMTYPE=<DDR/HBM> DSIZE=<From Host> ADDRNDM=<0/1> KRNL_LOOP=<#>"
	@echo  " "
	@echo  " > make run TARGET=hw MEMTYPE=HBM BANKS=2 DSIZE=4096 ADDRNDM=1 KRNL_LOOP=64"
	@echo  " "

