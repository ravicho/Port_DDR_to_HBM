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
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
		return EXIT_FAILURE;
	}

    std::string binaryFile = argv[1];
    //size_t total_data_size = sizeof(uint) * DATA_SIZE;
    long unsigned int total_data_size = atoi (argv[2]) ; 
    size_t vector_size_bytes = sizeof(int) * total_data_size;
    cl_int err;
    unsigned fileBufSize;
    size_t numIter = 4; 

    /* Reducing the data size for emulation mode */
    char *xcl_mode = getenv("XCL_EMULATION_MODE");
    if (xcl_mode != NULL) {
      //totalbuffersize = numIter*1024; /* 1MB */
    }

    // Allocate Memory in Host Memory
    std::vector<int,aligned_allocator<int>> source_in1(total_data_size);
    std::vector<int,aligned_allocator<int>> source_in2(total_data_size);
    std::vector<int,aligned_allocator<int>> source_hw_results(total_data_size);
    std::vector<int,aligned_allocator<int>> source_sw_results(total_data_size);

    // Create the test data 
    for(size_t i = 0 ; i < total_data_size ; i++){
        source_in1[i] = rand() % total_data_size;
        source_in2[i] = rand() % total_data_size;
        source_sw_results[i] = source_in1[i] + source_in2[i];
        source_hw_results[i] = 0;
    }

// OPENCL HOST CODE AREA START
	
// ------------------------------------------------------------------------------------
// Step 1: Get All PLATFORMS, then search for Target_Platform_Vendor (CL_PLATFORM_VENDOR)
//	   Search for Platform: Xilinx 
// Check if the current platform matches Target_Platform_Vendor
// ------------------------------------------------------------------------------------	
    std::vector<cl::Device> devices = get_devices("Xilinx");
    devices.resize(1);
    cl::Device device = devices[0];

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

  cl::Buffer *buffer_in1 = new cl::Buffer[numIter];
  cl::Buffer *buffer_in2 = new cl::Buffer[numIter];
  cl::Buffer *buffer_output = new cl::Buffer[numIter];

  // Create events for read,compute and write
  std::vector<cl::Event> host_2_device_Wait;
  std::vector<cl::Event> krnl_Wait;
  std::vector<cl::Event> device_2_host_Wait;
  cl::Event host_2_device_Done, krnl_Done, device_2_host_Done;

printf("Data_size = %zu\n", total_data_size);
printf("\n Total Data of %lf kB bytes Written to global memory... split into chunks of %zu \n\n ", vector_size_bytes/((double)1024), numIter);

for (size_t j = 0; j < numIter; j++) {

// ================================================================
// Step 2: Setup Buffers and run Kernels
// ================================================================
//   o) Allocate Memory to store the results 
//   o) Create Buffers in Global Memory to store data
// ================================================================

// ------------------------------------------------------------------
// Step 2: Create Buffers in Global Memory to store data
//             o) buffer_in1 - stores source_in1
//             o) buffer_in2 - stores source_in2
//             o) buffer_ouput - stores Results
// ------------------------------------------------------------------	

// .......................................................
// Allocate Global Memory for source_in1
// .......................................................	
    //OCL_CHECK(err, cl::Buffer buffer_in1   (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, vector_size_bytes, source_in1.data(), &err));
    OCL_CHECK(err, buffer_in1[j] = cl::Buffer(context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, vector_size_bytes/numIter, &source_in1[j*total_data_size/numIter], &err));
// .......................................................
// Allocate Global Memory for source_in2
// .......................................................
    //OCL_CHECK(err, cl::Buffer buffer_in2   (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, vector_size_bytes, source_in2.data(), &err));
    OCL_CHECK(err, buffer_in2[j] = cl::Buffer(context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, vector_size_bytes/numIter, &source_in2[j*total_data_size/numIter], &err));
// .......................................................
// Allocate Global Memory for sourcce_hw_results
// .......................................................
    //OCL_CHECK(err, cl::Buffer buffer_output(context,CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, vector_size_bytes, source_hw_results.data(), &err));
    OCL_CHECK(err, buffer_output[j] = cl::Buffer(context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, vector_size_bytes/numIter, &source_hw_results[j*total_data_size/numIter], &err));

// ============================================================================
// Step 2: Set Kernel Arguments and Run the Application
//         o) Set Kernel Arguments
//              ----------------------------------------------------
//              Kernel Argument  Description
//              ----------------------------------------------------
//              in1   (input)     --> Input Vector1
//              in2   (input)     --> Input Vector2
//              out   (output)    --> Output Vector
//              size  (input)     --> Size of Vector in Integer
//         o) Copy Input Data from Host to Global Memory on the device
//         o) Submit Kernels for Execution
//         o) Copy Results from Global Memory, device to Host
// ============================================================================	
    int size = total_data_size;
    OCL_CHECK(err, err = krnl_vector_add.setArg(0, buffer_in1[j]));
    OCL_CHECK(err, err = krnl_vector_add.setArg(1, buffer_in2[j]));
    OCL_CHECK(err, err = krnl_vector_add.setArg(2, buffer_output[j]));
    OCL_CHECK(err, err = krnl_vector_add.setArg(3, size));

// ------------------------------------------------------
// Step 2: Copy Input data from Host to Global Memory on the device
// ------------------------------------------------------
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_in1[j], buffer_in2[j]},0/* 0 means from host*/));	
	
// ----------------------------------------
// Step 2: Submit Kernels for Execution
// ----------------------------------------
    OCL_CHECK(err, err = q.enqueueTask(krnl_vector_add));
	
// --------------------------------------------------
// Step 2: Copy Results from Device Global Memory to Host
// --------------------------------------------------
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_output[j]},CL_MIGRATE_MEM_OBJECT_HOST));
 
}

    q.finish();
	
// OPENCL HOST CODE AREA END

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

    std::cout << "TEST " << (match ? "PASSED" : "FAILED") << std::endl; 
    return (match ? EXIT_SUCCESS : EXIT_FAILURE);
}

