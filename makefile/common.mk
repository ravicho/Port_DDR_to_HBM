
./$(BUILDDIR)/vadd_$(TARGET).xo: $(HOST_SRC_FPGA)
	v++ -c -g -t $(TARGET) -R 1 -k vadd \
		--platform $(PLATFORM) \
                --profile_kernel data:all:all:all \
                --profile_kernel stall:all:all:all \
		--save-temps \
		--temp_dir ./$(BUILDDIR)/temp_dir \
		--report_dir ./$(BUILDDIR)/report_dir \
		--log_dir ./$(BUILDDIR)/log_dir \
		-I$(SRCDIR) \
		--config ./bandwidth.ini \
		-DNDDR_BANKS=$(BANKS) \
		$(HOST_SRC_FPGA) \
		-o ./$(BUILDDIR)/vadd_$(TARGET).xo

#./$(BUILDDIR)/vadd_$(TARGET).xclbin:
./$(BUILDDIR)/vadd_$(TARGET).xclbin: ./$(BUILDDIR)/vadd_$(TARGET).xo
	v++ -l -g -t $(TARGET) -R 1 \
		--platform $(PLATFORM) \
                --profile_kernel data:all:all:all \
                --profile_kernel stall:all:all:all \
		--temp_dir ./$(BUILDDIR)/temp_dir \
		--report_dir ./$(BUILDDIR)/report_dir \
		--log_dir ./$(BUILDDIR)/log_dir \
		--config $(CURRENT_DIR)/$(MEMTYPE)$(BANKS).cfg \
		-I$(SRCDIR) \
		-DNDDR_BANKS=$(BANKS) \
		./$(BUILDDIR)/vadd_$(TARGET).xo \
		-o ./$(BUILDDIR)/vadd_$(TARGET).xclbin

host: $(SRCDIR)/*.cpp 
	mkdir -p $(BUILDDIR)
	g++ -D__USE_XOPEN2K8 -D__USE_XOPEN2K8 \
		-I$(XILINX_XRT)/include/ \
		-I$(SRCDIR) \
		-O3 -Wall -fmessage-length=0 -std=c++11\
		$(HOST_SRC_CPP) \
		-L$(XILINX_XRT)/lib/ \
		-lxilinxopencl -lpthread -lrt \
		-o $(BUILDDIR)/host

emconfig.json:
	cp $(SRCDIR)/emconfig.json .

xclbin: ./$(BUILDDIR)/vadd_$(TARGET).xclbin

xo: ./$(BUILDDIR)/vadd_$(TARGET).xo

clean:
	rm -rf temp_dir log_dir ../build report_dir *log host vadd* *.csv *summary .run .Xil vitis* *jou xilinx*
