CURRENT_DIR = $(PWD)
TARGET := hw

PLATFORM :=xilinx_u200_xdma_201830_2
#PLATFORM :=xilinx_u280-es1_xdma_201910_1

SRCDIR := ./../reference_files
MEMTYPE = DDR

# Default Settings here ..
HOST_SRC_CPP := $(SRCDIR)/kernel_global_bandwidth.cpp
BUILDDIR := ./../build/$(MEMTYPE)/bank_$(BANKS)


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

build: compile_host ./$(BUILDDIR)/myKernel_$(TARGET).xclbin $(BUILDDIR)/$(EMCONFIG_FILE)

#####################################################################################################
###   For HW run, use 100,000 words for computation 
###   For Emulation run, use 100 words for computation 
#####################################################################################################

run:   build host
	cp xrt.ini $(BUILDDIR)
ifeq ($(TARGET), hw)
	@echo "************  Use Command Line to run application!  ************"
	cd $(BUILDDIR) && ./host myKernel_$(TARGET).xclbin;
else
	@echo "Running $(TAGET) mode"; 
	emconfigutil --nd 1  --platform $(PLATFORM) --od $(BUILDDIR)
	cd $(BUILDDIR) && XCL_EMULATION_MODE=$(TARGET) ./host myKernel_$(TARGET).xclbin;
endif

run_ddr:
	make run TARGET=hw_emu DEVICE=xilinx_u200_xdma_201830_2 MEMTYPE=DDR BANKS=1;	
	make run TARGET=hw_emu DEVICE=xilinx_u200_xdma_201830_2 MEMTYPE=DDR BANKS=2;	
	make run TARGET=hw_emu DEVICE=xilinx_u200_xdma_201830_2 MEMTYPE=DDR BANKS=3;	
	make run TARGET=hw_emu DEVICE=xilinx_u200_xdma_201830_2 MEMTYPE=DDR BANKS=4;	
	make run TARGET=hw_emu DEVICE=xilinx_u200_xdma_201830_2 MEMTYPE=HBM BANKS=4;	

