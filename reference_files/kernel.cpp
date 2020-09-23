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

/*******************************************************************************
Description:
    HLS pragmas can be used to optimize the design : improve throughput, reduce latency and 
    device resource utilization of the resulting RTL code
    This is vector addition example to demonstrate how HLS optimizations are used in kernel. 
*******************************************************************************/


#define BUFFER_SIZE 1024
#define NUM_KRNL 4

#define DATA_SIZE 4096

// TRIPCOUNT identifier
const unsigned int c_size = BUFFER_SIZE;
const unsigned int c_len = DATA_SIZE / BUFFER_SIZE;
const unsigned int c_num = NUM_KRNL * DATA_SIZE / BUFFER_SIZE;


#include <ap_int.h>


unsigned int minRand(unsigned int seed, int load) {
  static ap_uint<32> lfsr;

  if (load == 1)
    lfsr = seed;
  bool b_32 = lfsr.get_bit(32 - 32);
  bool b_22 = lfsr.get_bit(32 - 22);
  bool b_2 = lfsr.get_bit(32 - 2);
  bool b_1 = lfsr.get_bit(32 - 1);
  bool new_bit = b_32 ^ b_22 ^ b_2 ^ b_1;
  lfsr = lfsr >> 1;
  lfsr.set_bit(31, new_bit);

  return lfsr.to_uint();
}

/*
    Vector Addition Kernel Implementation 
    Arguments:
        in1   (input)     --> Input Vector1
        in2   (input)     --> Input Vector2
        out   (output)    --> Output Vector
        size  (input)     --> Size of Vector in Integer
   */
extern "C" {
void vadd(
        const unsigned int *in1, // Read-Only Vector 1
        const unsigned int *in2, // Read-Only Vector 2
        unsigned int *out,       // Output Result
        int size,                // Size in integer
	const unsigned int num_times, // Running the same kernel operations num_times
	bool addRandom           // Address Pattern is random
        )
{

    unsigned int v1_buffer[BUFFER_SIZE];    // Local memory to store vector1
    unsigned int v2_buffer[BUFFER_SIZE];    // Local memory to store vector2
    unsigned int vout_buffer[BUFFER_SIZE];  // Local Memory to store result
    unsigned int seed = 1;
    int i, in_index;


    minRand(16807, 1);
  for (int count = 0; count < num_times; count++) {
#pragma HLS LOOP_TRIPCOUNT min = c_num max = c_num

    //Per iteration of this loop perform BUFFER_SIZE vector addition
    for(i = 0; i < size;  i += BUFFER_SIZE)
    {
#pragma HLS LOOP_TRIPCOUNT min = c_len max = c_len
	seed = minRand(31, 0);
        in_index = addRandom ? (seed % size) : i;
        int chunk_size = BUFFER_SIZE;
        //boundary checks
        if ((in_index + BUFFER_SIZE) > size) chunk_size = size - in_index;

        // burst read of v1 and v2 vector from global memory
        read1: for (int j = 0 ; j < chunk_size ; j++){
#pragma HLS LOOP_TRIPCOUNT min = c_size max = c_size
#pragma HLS PIPELINE II = 1
            v1_buffer[j] = in1[in_index + j];
        }
        read2: for (int j = 0 ; j < chunk_size ; j++){
#pragma HLS LOOP_TRIPCOUNT min = c_size max = c_size
#pragma HLS PIPELINE II = 1
            v2_buffer[j] = in2[in_index + j];
        }

    // PIPELINE pragma reduces the initiation interval for loop by allowing the
    // concurrent executions of operations
        vadd: for (int j = 0 ; j < chunk_size; j ++){
#pragma HLS LOOP_TRIPCOUNT min = c_size max = c_size
#pragma HLS PIPELINE II = 1
            //perform vector addition
            vout_buffer[j] = v1_buffer[j] + v2_buffer[j]; 
        }
        //burst write the result
        write: for (int j = 0 ; j < chunk_size ; j++){
#pragma HLS LOOP_TRIPCOUNT min = c_size max = c_size
#pragma HLS PIPELINE II = 1
            out[in_index + j] = vout_buffer[j];
        }
    }
  }
}
}
