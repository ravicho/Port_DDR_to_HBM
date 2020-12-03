## Application Overview

This tutorial uses a simple example of vector addition. It shows the  kernel reading data from in1 and in2 and producing the result, out.

The origianl application is implemted using DDR. Ports `in1` and `in2` are reading from DDR banks 0 and 1 respectively and port `out` is writing the results in DDR bank 2. The tutorial will walk through the necessary changes to the exisintg application to migrate to HBM.

The kernel code is a simple vector addition with the following function signature. 

```
void vadd(
  const unsigned int *in1, // Read-Only Vector 1
  const unsigned int *in2, // Read-Only Vector 2
  unsigned int *out,       // Output Result
  int size,                // Size in integer
  const unsigned int num_times, // Running the same kernel operations num_times
  bool addRandom           // Address Pattern is random
  )

```
- in1 and in2 : inputs for streaming data from DDR over AXI interfaces
- out         : output for streaming output results of vector addition to be written in DDR over AXI interface
- size        : limits the maximum size of the words can be added per iteration of the loop.
- num_times   : number of times the kernel is being enqueued.
- addRandom   : enables random access if set to 1

For more infomration on the kernel source code, refer to  `./reference_files/kernel.cpp`

The ports to DDR banks connectivity is established using system port mapping option using `--sp` switch. This switch allows the designer to map the kernel ports to specific globarl memory banks. 

Contents of the connectivity file, DDR.cfg

```bash
[connectivity]
sp=vadd_1.in1:DDR[0]
sp=vadd_1.in2:DDR[0]
sp=vadd_1.out:DDR[1]
```

The host code creates three buffers, one each in DDR0, DDR1 and DDR2 and connectivity is established for these bufferes to the kernel arguments. Each buffer is connected to a single DDR bank with capacity of 16GB which is higher than the buffer size used in this application. You should be able to migrate up to 4GB due to limitation on linux kernel. 

The following code creates the 3 buffers of size, vector_size_bytes which can be controlled as an argument during the application runtime. 

```
134 : cl::Buffer(context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, vector_size_bytes, &source_in1[total_data_size]);
135 : cl::Buffer(context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, vector_size_bytes, &source_in2[total_data_size]);
136 : cl::Buffer(context,CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, vector_size_bytes, &source_hw_results[total_data_size]);
```

```
140 : krnl_vector_add.setArg(1, buffer_in2[j]));
141 : krnl_vector_add.setArg(0, buffer_in1[j]));
142 : krnl_vector_add.setArg(2, buffer_output[j]));
```

For more infomration on the kernel source code, refer to  `./reference_files/host.cpp`

Let's run the application on hardware using DDR with buffer size of 600MB, sequential address pattern, enque the kernel 64 times. Here is the makefile command that can be used 


``` bash
make ddr_addSeq
```

The above command essentially expands to the following. 

``` bash
make run TARGET=hw MEMTYPE=DDR DSIZE=600 ADDRNDM=0 KRNL_LOOP=64 BUILDXCLBIN=0
```

BUILDXCLBIN=0 indicates that the application can use existing xclbin available as part of the tutorial package. If the application is run on different platform than used in this application, new xclbin will need to be generated. You should set BUILDXCLBIN=1 for that case. 

TARGET=hw_emu can also be used but this will take significant time to run the application for 600MB size buffer. For this reason, application is run on hardware by using TARGET=hw 


The above commands shows the following results 

```
************  Use Command Line to run application!  ************
cd ./../build/DDR && ./host vadd_hw.xclbin 600 0 64;
Found Platform
Platform Name: Xilinx
DEVICE xilinx_u200_xdma_201830_2
INFO: Reading vadd_hw.xclbin
Loading: 'vadd_hw.xclbin'

 Total Data of 600.000 Mbytes to be written to global memory from host
 
 Kernel is invoked 1 time and repeats iteself 64 times 

kernel_time_in_sec = 148.215
Throughput Achived = 0.253012 GB/s
TEST PASSED
```

The host is sending 600MB data to DDR0 and DDR1. The kernel reads this data using in1, in2 ports from DDR0 and DDR1 respectively. The vector addition is performed by kernel and results are written to DDR2. This results from DDR2 are migrated back to host. The next section goes over the steps required to migrate this DDR based application to HBM.  

## Migration to HBM

The host code and kernel code are agnostic to the memory type used, wheather DDR is used or HBM or even PLRAMs. The only change you will need to make here is to modify the connectivity file. 

Vitis flow make is easy to switch memory connection using `-sp` switches and in this case we need to repleace DDR with HBM. The capacity of each HBM bank is 256MB. Since our application requires 600MB of data to be added, we will need 3 HBM banks as contigous memory. Vitis flow enables this by grouping the memory as shown below in the connectivity file. 
Before we update the connectivity with 3 HBM banks, let's try grouping only 2 memory banks to confirm the application error.  

Update the connectivity file as shown below or use existing connectivity file, HBM2.cfg

```bash
[connectivity]
sp=vadd_1.in1:HBM[0:1]
sp=vadd_1.in2:HBM[2:3]
sp=vadd_1.out:HBM[4:5]
```
Run the following command to use the application with HBM memmory of size 512MB for in1,in2 and out ports.

``` bash
make hbm_addSeq_2
```
The above commands shows the following results 

```
cd ./../build/HBM2 && ./host vadd_hw.xclbin 600 0 64;
Found Platform
Platform Name: Xilinx
DEVICE xilinx_u50_gen3x4_xdma_base_2
INFO: Reading vadd_hw.xclbin
Loading: 'vadd_hw.xclbin'

 Total Data of 600.000 Mbytes to be written to global memory from host
 
 Kernel is invoked 1 time and repeats iteself 64 times 

XRT build version: 2.8.693
Build hash: 0de7e5f37b3325476fe33f2d2fba1e0dfeb09e43
Build date: 2020-11-01 18:51:45
Git branch: master
PID: 4670
UID: 31781
[Thu Nov  5 18:52:19 2020 GMT]
HOST: xcodpeascoe40
EXE: /scratch/ravic/Port_DDR_to_HBM/build/HBM2/host
[XRT] ERROR: std::bad_alloc
./../reference_files/host.cpp:140 Error calling err = krnl_vector_add.setArg(1, buffer_in2[j]), error code is: -5
[XRT] WARNING: Profiling may contain incomplete information. Please ensure all OpenCL objects are released by your host code (e.g., clReleaseProgram()).
Makefile:67: recipe for target 'run' failed
make[1]: *** [run] Error 1
make[1]: Leaving directory '/scratch/ravic/Port_DDR_to_HBM/makefile'
Makefile:80: recipe for target 'hbm_addSeq_2' failed
make: *** [hbm_addSeq_2] Error 2
```

As expected, the application results into error as you are trying to create 600 MB buffer in HBM[0:1]. XRT sees this as a contiguous memory of 256*2 = 512MB but the host is exceeding this size limit and thus resulting into application error. 

Next, we are going to increase the number of HBM banks connected to the ports to 3 banks instead of 2 for each port as in previous case. For simplicity, we have connected all the ports to 32 banks as shown below. 

Let's use the following connectivity file that utilizes 3 HBM memory banks. 

```bash
[connectivity]
sp=vadd_1.in1:HBM[0:2]
sp=vadd_1.in2:HBM[3:5]
sp=vadd_1.out:HBM[6:8]
```

For above connectivity, kernel in1 port is connected to contiguous HBM banks 0, 1 and 2, in2 port is connected to contiguous HBM banks 3, 4 and 5 and out port is connected to HBM banks 6, 7 and 8.

Run the following command to use the application with HBM memmory of size 768MB for in1,in2 and out ports.

``` bash
make hbm_addSeq_3
```

The above command shows the following results

```
************  Use Command Line to run application!  ************
cd ./../build/HBM3 && ./host vadd_hw.xclbin 600 0 64;
Found Platform
Platform Name: Xilinx
DEVICE xilinx_u50_gen3x4_xdma_base_2
INFO: Reading vadd_hw.xclbin
Loading: 'vadd_hw.xclbin'

 Total Data of 600.000 Mbytes to be written to global memory from host
 
 Kernel is invoked 1 time and repeats iteself 64 times 

kernel_time_in_sec = 148.215
Throughput Achived = 0.253012 GB/s
TEST PASSED
```

In the last step, there were 3 ports connected to each input/output kernel port. 


If the application doesn't require the full memory bank, Vitis flow also provide the capability of sharing the memory banks across the ports. Here is one example of connectivity for sharing banks between ports in1 and in2.

```bash
[connectivity]
sp=vadd_1.in1:HBM[0:1]
sp=vadd_1.in2:HBM[1:2]
sp=vadd_1.out:HBM[3:4]
```

The ports in1 and in2 and sharing bank 1 of HBM. So application can create a max of 384MB buffer size for each input. 

Run the following command to use the application with HBM memmory of size 384MB for in1,in2 and out ports.

``` bash
make hbm_addSeq_overlap_banks_2
```



You can also connect all the 32 HBM banks to each of the ports based on application requirement. This way, the whole memory space will be available to all the ports. The overall HBM efficienty will vary based on the access pattern as descibed in the previous module of this tutorial. 


