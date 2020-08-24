/**********
Copyright (c) 2020, Xilinx, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********/
/*****************************************************************************************
*  GUI Flow :
*
*  By default this example supports 1DDR execution in GUI mode for
*  all the XSAs. To make this example to work with multi DDR XSAs
*  please follow steps mentioned below.
*
*  ***************************************************************************************
*  XSA  (2DDR):
*
*  1. Add a .ini file in the <Project>/src folder with the following entries:
*  	    [Connecttivity]
*  	    sp=bandwidth_1.m_axi_gmem0:DDR[0]
*     	    sp=bandwidth_1.m_axi_gmem1:DDR[1]
*
*     For more number of DDR connections add more sp tags as shown above in the
*ini file.
*     Note : Replace DDR[0] with HP0, DDR[1] with HP1 for embedded platforms(zc)
*  2.<Vitis Project> > Properties > C/C++ Build > Settings > Vitis V++ Kernel
*Linker
*                  > Miscellaneous > Other flags
*     --config ../src/<config_file>.ini
*  3. Default number of banks for CLI flow is 2 banks, for GUI flow is 1 bank.
*     For 3 or 4 DDR connections, "#define NDDR_BANKS <3 or 4>" at the top of
*kernel.cpp
* *****************************************************************************************
*
*  CLI Flow:
*
*  In CLI flow makefile detects the DDR of a device on the basis of ddr_banks
*variable in config.mk file
*  and based on that automatically it adds all the flags that are necessary.
*This example can be
*  used similar to other examples in CLI flow, extra setup is not needed.
*
*********************************************************************************************/

#include <vector>
#include "xcl2.hpp"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NDDR_BANKS
#define NDDR_BANKS 2
#endif

using namespace std;
using namespace std::chrono;


int main(int argc, char **argv) {

  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
    return EXIT_FAILURE;
  }

  std::string binaryFile = argv[1];

  cl_int err;

  // Boilerplate code to load the FPGA binary, create the kernel and command queue

  auto devices = xcl::get_xil_devices();
  cl::Device device = devices[0];

  auto fileBuf = xcl::read_binary_file(binaryFile);

  cl::Context context(device);
  cl::CommandQueue q(context,device, CL_QUEUE_PROFILING_ENABLE|CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);

  std::string kernel_name = "myKernel";
  const char* kernel_name_charptr = kernel_name.c_str();

  std::string run_type = xcl::is_emulation()?(xcl::is_hw_emulation()?"hw_emu":"sw_emu"):"hw";
  std::string binary_file = kernel_name + "_" + run_type + ".xclbin";

  cl::Program::Binaries bins {{ fileBuf.data(), fileBuf.size() } };
  cl::Program program(context, devices, bins);

  cl::Kernel krnl_global_bandwidth(program,kernel_name_charptr,NULL);

//   ---------------------------------------------------------------------------------------------

  size_t numIter = 4; 
  size_t globalbuffersize = 4*4*4*64*1024*256 ; /* 256 MB */
  size_t totalbuffersize = numIter*globalbuffersize; /* 256 MB */

  /* Reducing the data size for emulation mode */
  char *xcl_mode = getenv("XCL_EMULATION_MODE");
  if (xcl_mode != NULL) {
    totalbuffersize = numIter*1024; /* 1MB */
  }

  /* Input buffer */
  uint8_t *input_host = ((uint8_t *)malloc(totalbuffersize));
  if (input_host == NULL) {
    printf("Error: Failed to allocate host side copy of OpenCL source " "buffer of size %zu\n", totalbuffersize);
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < totalbuffersize; i++) {
    //printf("i = %d\n", i);
    input_host[i] = i % 256;
  }

  //short ddr_banks = NDDR_BANKS;

  /* prepare data to be written to the device */
  std::vector<uint8_t, aligned_allocator<uint8_t>> map_input_buffer(totalbuffersize);
  std::vector<uint8_t, aligned_allocator<uint8_t>> map_output_buffer(totalbuffersize);
  for (size_t i = 0; i < totalbuffersize; i++) {
    map_input_buffer[i] = input_host[i];
  }

  double dbytes = totalbuffersize;
  double dmbytes = dbytes / (((double)1024) * ((double)1024));

  // Create events for read,compute and write
  std::vector<cl::Event> host_2_device_Wait;
  std::vector<cl::Event> krnl_Wait;
  std::vector<cl::Event> device_2_host_Wait;
  cl::Event host_2_device_Done, krnl_Done, device_2_host_Done;

  chrono::high_resolution_clock::time_point t1, t2;
  t1 = chrono::high_resolution_clock::now();

  cl::Buffer *inputBuffer = new cl::Buffer[numIter];
  cl::Buffer *outputBuffer = new cl::Buffer[numIter];

  printf("Total Data for read/write %.0lf MB bytes from/to global memory... split into %zu chunks \n", dmbytes, numIter);

  for (size_t j = 0; j < numIter; j++) {
  printf("Enqueuing kernel to read/write %zu MB bytes from/to global " "memory... \n", (totalbuffersize)/(numIter));

  OCL_CHECK(err, inputBuffer[j] = cl::Buffer(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, totalbuffersize/numIter, &map_input_buffer[j*totalbuffersize/numIter], &err));
  OCL_CHECK(err, outputBuffer[j] = cl::Buffer(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, totalbuffersize/numIter, &map_output_buffer[j*totalbuffersize/numIter], &err));

  /* Set the kernel arguments */
  cl_ulong num_blocks = totalbuffersize / 64;   // 64 Bytes

  OCL_CHECK(err, err = krnl_global_bandwidth.setArg(0, inputBuffer[j]));
  OCL_CHECK(err, err = krnl_global_bandwidth.setArg(1, outputBuffer[j]));
  OCL_CHECK(err, err = krnl_global_bandwidth.setArg(2, num_blocks));

  /* Migrate the buffer Data to Device */
  q.enqueueMigrateMemObjects({inputBuffer[j]}, 0,NULL,&host_2_device_Done);
  host_2_device_Wait.push_back(host_2_device_Done);

  /* Execute Kernel */
  OCL_CHECK(err, err = q.enqueueTask(krnl_global_bandwidth, &host_2_device_Wait, &krnl_Done));
  krnl_Wait.push_back(krnl_Done);

  /* Migrate the buffer Data from Device */
  q.enqueueMigrateMemObjects({outputBuffer[j]}, CL_MIGRATE_MEM_OBJECT_HOST,&krnl_Wait,&device_2_host_Done);
  device_2_host_Wait.push_back(device_2_host_Done);
  //device_2_host_Wait[j].wait();

  }
  //for (size_t j = 0; j < numIter; j++) {
   //device_2_host_Wait[j].wait();
  //}
//host_2_device_Wait.clear();
//krnl_Wait.clear();
//device_2_host_Wait.clear();

  OCL_CHECK(err, err = q.finish());

  t2 = chrono::high_resolution_clock::now();
  chrono::duration<double> perf_all_sec  = chrono::duration_cast<duration<double>>(t2-t1);

  cl_ulong f1 = 0;
  cl_ulong f2 = 0;
  host_2_device_Wait.front().getProfilingInfo(CL_PROFILING_COMMAND_QUEUED, &f1);
  device_2_host_Wait.back().getProfilingInfo(CL_PROFILING_COMMAND_END, &f2);
  double perf_hw_ms = (f2 - f1)/1000000.0;


  /* Check the results of output0 */
  //printf("size = %d\n",totalbuffersize);
  for (size_t i = 0; i < totalbuffersize; i++) {
    if (map_output_buffer[i] != input_host[i]) {
      printf("ERROR : kernel failed to copy entry %zu input %i output %i\n", i, input_host[i], map_output_buffer[i]);
      return EXIT_FAILURE;
    }
    else {
      //printf("GOOD : kernel failed to copy entry %zu input %i output %i\n", i, input_host[i], map_output_buffer[i]);
    }
  }
  
  printf("Kernel completed read/write %.0lf MB bytes from/to global memory.\n", dmbytes);
  if (xcl::is_emulation()) {
     if (xcl::is_hw_emulation()) {
        printf(" Emulated FPGA accelerated version  | run 'vitis_analyzer xclbin.run_summary' for performance estimates");
     } else {
        printf(" Emulated FPGA accelerated version  | (performance not relevant in SW emulation)");
     }
  } else {
        printf(" Executed FPGA accelerated version  | %10.4f ms   ( FPGA %.3f ms )", 1000*perf_all_sec.count(), perf_hw_ms);
  }
  printf("\n");

  delete [] inputBuffer;
  delete [] outputBuffer;



  printf("TEST PASSED\n");
  return EXIT_SUCCESS;
}
