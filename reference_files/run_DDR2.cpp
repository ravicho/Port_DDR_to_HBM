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

int main(int argc, char **argv) {

  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
    return EXIT_FAILURE;
  }

  std::string binaryFile = argv[1];

  cl_int err;

  // Boilerplate code to load the FPGA binary, create the kernel and command queue
  //std::vector<cl::Device> devices = xcl::get_xil_devices();

  auto devices = xcl::get_xil_devices();
  cl::Device device = devices[0];

  auto fileBuf = xcl::read_binary_file(binaryFile);

  cl::Context context(device);
  cl::CommandQueue q(context,device, CL_QUEUE_PROFILING_ENABLE|CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);

  std::string kernel_name = "myKernel";
  const char* kernel_name_charptr = kernel_name.c_str();

  std::string run_type = xcl::is_emulation()?(xcl::is_hw_emulation()?"hw_emu":"sw_emu"):"hw";
  std::string binary_file = kernel_name + "_" + run_type + ".xclbin";
  //cl::Program::Binaries bins = xcl::import_binary_file(binary_file);

  cl::Program::Binaries bins {{ fileBuf.data(), fileBuf.size() } };
  cl::Program program(context, devices, bins);

  cl::Kernel krnl_global_bandwidth(program,kernel_name_charptr,NULL);

//   ---------------------------------------------------------------------------------------------

  size_t globalbuffersize = 1024 * 1024 * 256; /* 256 MB */

  /* Reducing the data size for emulation mode */
  char *xcl_mode = getenv("XCL_EMULATION_MODE");
  if (xcl_mode != NULL) {
    globalbuffersize = 1024 * 1024; /* 1MB */
  }

  /* Input buffer */
  unsigned char *input_host = ((unsigned char *)malloc(globalbuffersize));
  if (input_host == NULL) {
    printf("Error: Failed to allocate host side copy of OpenCL source "
           "buffer of size %zu\n",
           globalbuffersize);
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < globalbuffersize; i++) {
    input_host[i] = i % 256;
  }

  short ddr_banks = NDDR_BANKS;

  /* prepare data to be written to the device */
  unsigned char *map_input_buffer0 = (unsigned char*)aligned_alloc(4096, globalbuffersize*sizeof(unsigned char));
  unsigned char *map_output_buffer0 = (unsigned char*)aligned_alloc(4096, globalbuffersize*sizeof(unsigned char));
  for (size_t i = 0; i < globalbuffersize; i++) {
    map_input_buffer0[i] = input_host[i];
  }

  /* Index for the ddr pointer array: 4=4, 3=3, 2=2, 1=2 */
  char num_buffers = ddr_banks;
  if (ddr_banks == 1)
    num_buffers = ddr_banks + (ddr_banks % 2);

  /* buffer[0] is input0
   * buffer[1] is output0
  */
  cl::Buffer *buffer[num_buffers];

  OCL_CHECK(err, buffer[0] = new cl::Buffer(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, globalbuffersize, map_input_buffer0, &err));
  OCL_CHECK(err, buffer[1] = new cl::Buffer(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, globalbuffersize, map_output_buffer0, &err));

  /* Set the kernel arguments */
  cl_ulong num_blocks = globalbuffersize / 64;

  OCL_CHECK(err, err = krnl_global_bandwidth.setArg(0, *(buffer[0])));
  OCL_CHECK(err, err = krnl_global_bandwidth.setArg(1, *(buffer[1])));
  OCL_CHECK(err, err = krnl_global_bandwidth.setArg(2, num_blocks));

  double dbytes = globalbuffersize;
  double dmbytes = dbytes / (((double)1024) * ((double)1024));
  printf("Starting kernel to read/write %.0lf MB bytes from/to global " "memory... \n", dmbytes);

  // Create events for read,compute and write

  std::vector<cl::Event> host_2_device_Wait;
  std::vector<cl::Event> krnl_Wait;
  std::vector<cl::Event> flagWait;
  cl::Event host_2_device_Done, krnl_Done, device_2_host_Done;
  unsigned long start, end, nsduration;

  /* Migrate the buffer Data to Device */
  q.enqueueMigrateMemObjects({*(buffer[0])}, 0,NULL,&host_2_device_Done);
  host_2_device_Wait.push_back(host_2_device_Done);

  /* Execute Kernel */
  OCL_CHECK(err, err = q.enqueueTask(krnl_global_bandwidth, &host_2_device_Wait, &krnl_Done));
  krnl_Wait.push_back(krnl_Done);

  /* Migrate the buffer Data from Device */
  OCL_CHECK(err, err = krnl_Done.wait());
  q.enqueueMigrateMemObjects({*buffer[1]}, CL_MIGRATE_MEM_OBJECT_HOST,&krnl_Wait,&device_2_host_Done);

  end = OCL_CHECK(err, krnl_Done.getProfilingInfo<CL_PROFILING_COMMAND_END>(&err));
  start = OCL_CHECK(err, krnl_Done.getProfilingInfo<CL_PROFILING_COMMAND_START>(&err));
  nsduration = end - start;

  OCL_CHECK(err, err = q.finish());

  std::cout << "Kernel Duration..." << nsduration << " ns" << std::endl;

  /* Check the results of output0 */
  for (size_t i = 0; i < globalbuffersize; i++) {
    if (map_output_buffer0[i] != input_host[i]) {
      printf("ERROR : kernel failed to copy entry %zu input %i output %i\n", i,
             input_host[i], map_output_buffer0[i]);
      return EXIT_FAILURE;
    }
  }
  delete (buffer[0]);
  delete (buffer[1]);

  /* Profiling information */
  double dnsduration = ((double)nsduration);
  double dsduration = dnsduration / ((double)1000000000);

  double bpersec = (dbytes / dsduration);
  double gbpersec = bpersec / ((double)1024 * 1024 * 1024) * ddr_banks;

  printf("Kernel completed read/write %.0lf MB bytes from/to global memory.\n",
         dmbytes);
  printf("Execution time = %f (sec) \n", dsduration);
  printf("Concurrent Read and Write Throughput = %f (GB/sec) \n", gbpersec);

  printf("TEST PASSED\n");
  return EXIT_SUCCESS;
}
