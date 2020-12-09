# Introduction
This tutorial covers essential steps how to migrate an existing application using DDR memory to HBM memory.  You begin this tutorial by explaining the structural differences between DDR and achievable theoratical bandwidth. The next section uses a simple example with necessary steps to migration an application from DDR to HBM. 

# Before You Begin

>**TIP**: This tutorial takes approximately two hours to complete.

The labs in this tutorial use:

* BASH Linux shell commands.
* 2020.2 Vitis core development kit release and the *xilinx_u200_xdma_201830_2* platform. If necessary, it can be easily ported to other versions and platforms.

This tutorial guides you to run the designed accelerator on the FPGA; therefore, the expectation is that you have an Xilinx® Alveo™ U200 Data Center accelerator card set up to run this tutorial. Because it can take several (six or seven) hours to generate the multiple `xclbin` files needed to run the accelerator, pregenerated `xclbin` files are provided for the U200 card. To use these pregenerated files, when building the hardware kernel or running the accelerator on hardware, you need to add the `SOLUTION=1` argument. 

>**IMPORTANT:**  
>
> * Before running any of the examples, make sure you have installed the Vitis core development kit as described in [Installation](https://www.xilinx.com/cgi-bin/docs/rdoc?v=2020.1;t=vitis+doc;d=vhc1571429852245.html) in the Application Acceleration Development flow of the Vitis Unified Software Platform Documentation (UG1416).
>* If you run applications on Alveo cards, ensure the card and software drivers have been correctly installed by following the instructions on the [Alveo Portfolio page](https://www.xilinx.com/products/boards-and-kits/alveo.html).

## Accessing the Tutorial Reference Files

1. To access the tutorial content, enter the following in a terminal: `git clone http://github.com/Xilinx/Vitis-Tutorials`.
2. Navigate to the `Port_DDR_to_HBM` directory.
    * `Makefile` under the `makefile` directory explains the commands used in this lab. Use the `PLATFORM` variable if targeting different platforms.
    * `reference_file` contains the modified kernel and host-related files for achieving higher performance.


## Tutorial Overview

1. [HBM Overview](1_overview.md) : Provides a brief overivew on Structural Differences between DDR and HBM and theoratical maximum bandwidth by HBM.

2. [Migration from DDR to HBM](2_Migrating_to_HBM.md) : Walks through the steps of migrating existing DDR based application to HBM
### 



<p align="center"><b>
Start the next step: <a href="./Migrate_the_Application.md"> Migrate the application to HBM</a>
</b></p>
</br>
<hr/>
<p align="center"><b><a href="/docs/vitis-getting-started/README.md">Return to Getting Started Pathway</a> — <a href="docs/README.md">Return to Start of Tutorial</a></b></p>

<p align="center"><sup>Copyright&copy; 2020 Xilinx</sup></p>




