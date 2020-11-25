/**********
Copyright (c) 2018, Xilinx, Inc.
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
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********/

#include "host.hpp"

int main(int argc, char** argv)
{
  if (argc != 5) {
    std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
    return EXIT_FAILURE;
  }

  std::string binaryFile = argv[1];
  bool addRandom = atoi (argv[3]);
  unsigned int krnl_loop = atoi (argv[4]);
  if ((addRandom==1) && (krnl_loop<64)) krnl_loop=64; 

  long unsigned int vector_size_bytes = atoi (argv[2]) * 1024 *1024; // Converted to MB
  long unsigned int total_data_size = vector_size_bytes/sizeof(int) ; // Convert to Meg

  cl_int err;
  unsigned fileBufSize;
  size_t numIter = 1; 

  /* Reducing the data size for emulation mode */
  char *xcl_mode = getenv("XCL_EMULATION_MODE");
  if (xcl_mode != NULL) {
     vector_size_bytes = 1024 * sizeof(uint);
     addRandom=1;
     krnl_loop=1;	
  }

  // Allocate Memory in Host Memory
  std::vector<int,aligned_allocator<int>> source_in1(total_data_size);
  std::vector<int,aligned_allocator<int>> source_in2(total_data_size);
  std::vector<int,aligned_allocator<int>> source_hw_results(total_data_size);
  std::vector<int,aligned_allocator<int>> source_sw_results(total_data_size);

  // Create the test data 
  for(long unsigned int i = 0 ; i < total_data_size ; i++){
     source_in1[i] = rand() % total_data_size;
     source_in2[i] = rand() % total_data_size;
     source_sw_results[i] = source_in1[i] + source_in2[i];
     source_hw_results[i] = 0;
  }


	
// ------------------------------------------------------------------------------------
// Step 1: Get All PLATFORMS, then search for Target_Platform_Vendor (CL_PLATFORM_VENDOR)
//	   Search for Platform: Xilinx 
// Check if the current platform matches Target_Platform_Vendor
// ------------------------------------------------------------------------------------	
  std::vector<cl::Device> devices = get_devices("Xilinx");
  devices.resize(1);
  cl::Device device = devices[0];
  std::cout << "DEVICE " << device.getInfo<CL_DEVICE_NAME>() << std::endl;


// ------------------------------------------------------------------------------------
// Step 1: Create Context
// ------------------------------------------------------------------------------------
  OCL_CHECK(err, cl::Context context(device, NULL, NULL, NULL, &err));
	
// ------------------------------------------------------------------------------------
// Step 1: Create Command Queue
// ------------------------------------------------------------------------------------
  OCL_CHECK(err, cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE, &err));

// ------------------------------------------------------------------
// Step 1: Load Binary File from disk
// ------------------------------------------------------------------		
  char* fileBuf = read_binary_file(binaryFile, fileBufSize);
  cl::Program::Binaries bins{{fileBuf, fileBufSize}};
	
// -------------------------------------------------------------
// Step 1: Create the program object from the binary and program the FPGA device with it
// -------------------------------------------------------------	
  OCL_CHECK(err, cl::Program program(context, devices, bins, NULL, &err));

// -------------------------------------------------------------
// Step 1: Create Kernels
// -------------------------------------------------------------
  OCL_CHECK(err, cl::Kernel krnl_vector_add(program,"vadd", &err));

  cl::Buffer buffer_in1[numIter];
  cl::Buffer buffer_in2[numIter];
  cl::Buffer buffer_output[numIter];

  // Create events for read,compute and write
  std::vector<cl::Event> host_2_device_Wait;
  std::vector<cl::Event> krnl_Wait;
  std::vector<cl::Event> device_2_host_Wait;
  cl::Event host_2_device_Done, krnl_Done, device_2_host_Done;

  printf("\n Total Data of %2.3f Mbytes to be written to global memory from host\n ", (float) vector_size_bytes/1024/1024);
  printf("\n Kernel is invoked %zu time and repeats iteself %d times \n\n", numIter, krnl_loop);


  double kernel_time_in_sec = 0, result = 0;
  std::chrono::duration<double> kernel_time(0);
  std::chrono::duration<double> kernel_time1(0);

  auto kernel_start = std::chrono::high_resolution_clock::now();

  for (size_t j = 0; j < numIter; j++) {

    OCL_CHECK(err, buffer_in1[j] = cl::Buffer(context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, vector_size_bytes/numIter, &source_in1[j*total_data_size/numIter], &err));
    OCL_CHECK(err, buffer_output[j] = cl::Buffer(context,CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, vector_size_bytes/numIter, &source_hw_results[j*total_data_size/numIter], &err));
    OCL_CHECK(err, buffer_in2[j] = cl::Buffer(context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, vector_size_bytes/numIter, &source_in2[j*total_data_size/numIter], &err));

    int size = total_data_size/numIter;

    OCL_CHECK(err, err = krnl_vector_add.setArg(2, buffer_output[j]));
    OCL_CHECK(err, err = krnl_vector_add.setArg(0, buffer_in1[j]));
    OCL_CHECK(err, err = krnl_vector_add.setArg(1, buffer_in2[j]));
    OCL_CHECK(err, err = krnl_vector_add.setArg(3, size));
    OCL_CHECK(err, err = krnl_vector_add.setArg(4, krnl_loop));
    OCL_CHECK(err, err = krnl_vector_add.setArg(5, addRandom));

    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_in1[j], buffer_in2[j]},0/* 0 means from host*/));	
	
    OCL_CHECK(err, err = q.enqueueTask(krnl_vector_add));
	
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_output[j]},CL_MIGRATE_MEM_OBJECT_HOST));
 
  }
  q.finish();
  auto kernel_end = std::chrono::high_resolution_clock::now();
  kernel_time = std::chrono::duration<double>(kernel_end - kernel_start);
  kernel_time_in_sec = kernel_time.count();

  // Compare the results of the Device to the simulation
  bool match = true;
  for (size_t i = 0 ; i < total_data_size ; i++){
    if (source_hw_results[i] != source_sw_results[i]){
      std::cout << "Error: Result mismatch" << std::endl;
      std::cout << "i = " << i << " CPU result = " << source_sw_results[i]
                << " Device result = " << source_hw_results[i] << std::endl;
      match = false;
      break;
   }
  }
	
// ============================================================================
// Step 3: Release Allocated Resources
// ============================================================================
  delete[] fileBuf;

  // Multiplying the actual data size by 4 because four buffers are being
  // used.
  result = vector_size_bytes * krnl_loop;
  result /= 1024;               // to KB
  result /= 1024;               // to MB
  result /= 1024;               // to GB
  std::cout << "kernel_time_in_sec = " << kernel_time_in_sec << std::endl;

  result /= kernel_time_in_sec; // to GBps
  std::cout << "Throughput Achived = " << result << " GB/s" << std::endl;
  std::cout << "TEST " << (match ? "PASSED" : "FAILED") << std::endl; 
  return (match ? EXIT_SUCCESS : EXIT_FAILURE);
}
