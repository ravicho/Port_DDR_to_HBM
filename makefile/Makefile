CURRENT_DIR = $(PWD)
TARGET := hw

#PLATFORM :=xilinx_u200_xdma_201830_2
PLATFORM :=xilinx_u280_xdma_201920_3

SRCDIR := ./../reference_files
MEMTYPE = DDR

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
	cd $(BUILDDIR) && ./host vadd_$(TARGET).xclbin $(SIZE) $(ADDRNDM);
else
	@echo "Running $(TAGET) mode"; 
	emconfigutil --nd 1  --platform $(PLATFORM) --od $(BUILDDIR)
	cd $(BUILDDIR) && XCL_EMULATION_MODE=$(TARGET) ./host vadd_$(TARGET).xclbin $(SIZE) $(ADDRNDM);
endif

ddr2:
	make run TARGET=hw MEMTYPE=DDR SIZE=4096 ADDRNDM=1 NUM_TIMES=6400
hbm2:
	make run TARGET=hw MEMTYPE=HBM SIZE=4096 ADDRNDM=1 NUM_TIMES=6400

