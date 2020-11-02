# Application Overview

This tutorial uses a simple example of vector addition. It shows the vadd kernel reading data from in1 and in2 and producing the result, out.

The origianl application is implemted using DDR. Ports `in1` and `in2` are reading from DDR banks 0 and 1 respectively and port `out` is writing the results in DDR bank 2. The tutorial will walk through the necessary changes to the exisintg application to migrate to HBM.

The kernel code is available in `./reference_files/kernel.cpp`
The host code is available in `./reference_files/host.cpp`

The kernel code is a simple vector addition with the main functionality shown as below. 

```
```


The ports to DDR banks connectivity is established using system port mapping option using `--sp` switch. This switch allows the designer to map the kernel ports to specific globarl memory banks. 

Contents of the connectivity file, DDR.cfg

```bash

```
To run the application using DDR

``` bash
make ddr_addSeq
```

The above commands shows the following results 

```

```

The host is sending 600MB data to each input port, in1 and in2. 

# Migration to HBM

Vitis flow make is easy to switch memory connection using `-sp` switches and in this case we need to repleace DDR with HBM. The host code and kernel code requires no change at all for this application.

Update the connectivity file as shown below or use existing connectivity file, HBM2.cfg

```bash
[connectivity]
sp=vadd_1.in1:HBM[0:1]
sp=vadd_1.in2:HBM[2:3]
sp=vadd_1.out:HBM[4:5]
```

To run the application using HBM

```bash
[connectivity]
sp=vadd_1.in1:HBM[0:1]
sp=vadd_1.in2:HBM[2:3]
sp=vadd_1.out:HBM[4:5]
```

The above command shows the following results





The application results into error as you are trying to create 600 MB buffer in HBM[1:2]. XRT sees this as a contiguous memory of 256*2 = 512MB but the host is exceeding this size limit and thus resulting into application error. 

Next, we are going to increase the numbe of HBM banks connected to the ports to 3 banks instead of 2 for each port as in previous case. For simplicity, we have connected all the ports to 32 banks as shown below. 

```bash
[connectivity]
sp=vadd_1.in1:HBM[0:31]
sp=vadd_1.in2:HBM[0:31]
sp=vadd_1.out:HBM[0:31]
```

For above connectivity, XRT will allocate the first buffer connected to in1 port infrom the host into banks 0:2

Explain the difference here if its all the banks or 3 per port????s

To run the application 

```bash
make hbm_addSeq_2
```

The above command shows the following results
