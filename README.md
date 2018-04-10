PolyMem STREAM Implementation
=============================
STREAM is the de facto industry standard benchmark for measuring sustained memory bandwidth. The STREAM benchmark is a simple synthetic benchmark program that measures sustainable memory bandwidth (in MB/s) and the corresponding computation rate for simple vector kernels. [1](http://www.cs.virginia.edu/stream/ref.html)
This repository contains our Maxj implementation of the STREAM kernels linked to PolyMem. We use this implementation to benchmark the performance of PolyMem.

STREAM Description
==================
The STREAM benchmark contains four applications: Copy, Scale, Sum, and Triad. 
The benchmark uses three vectors - A, B and C - in all its applications. The Copy application performs a vector copy operation c(i)=a(i), which involves one read and one write for each element copied. The Scale application performs the scaling of a vector and stores its result in another vector a(i) = q*b(i); thus performing two memory accesses (a read and a write) and one floating point multiplication per element processed. The Sum application performs the sum of two vectors,  a(i) = b(i) + c(i), featuring two read, one write, and a floating point addition per element. Finally, the Triad application is a combination of the Scale and Sum, a(i) = b(i) + q*c(i), thus featuring two reads, one write, and the two floating point operations, a multiplication and an addition.
Each instruction of this application consists of two read operation, one floating point multiplication, a floating point addition and a write operation ( a(i) = b(i) + q*c(i)).

STREAM Kernel | Description
--------------|------------
COPY          | c[i]=a[i]
SCALE         | a[i]=q*b[i]
SUM           | a[i]=b[i]+c[i]
TRIAD         | a[i]=b[i]+q*c[i]


PolyMem STREAM Implementation
=============================
To use STREAM for the assessment of PolyMem, we must design the STREAM framework using Maxeler's toolchain and PolyMem. A high-level view of our design  is presented in figure below. The host is connected through the PCI-e to our STREAM design, and starts the computation by sending the Vector Sizes and Mode parameters to define the behavior of the Controller. The Controller generates the write and read signals for PolyMem and selects the correct input for PolyMem write port by driving the the two MUXs. The signals Wi, Wj| and Wshape and Ri, Rj, Rshap signals identify the write/read locations for the elements to be stored in/retrieved from PolyMem. Lastly, using the DEMUX, the controller selects the right output stream (from A_OUT, B_OUT, C_OUT) to correctly retrieve the data from the PolyMem.

![STREAM PolyMem](https://raw.githubusercontent.com/giuliostramondo/PolyMemStream/master/images/StreamImplementation.png)

Usage
=====

* Configuration of PolyMem

Our PRF implementation allow the user to customize the capacity, number of lanes, mapping scheme and frequency at wich the design is synthesized. All those paramethers can be set in the PolyMem configuration file PRFConfiguration.maxj located in /EngineCode/src/prfstream/PRFConstants.maxj


* Compilation

To compile the whole project - STREAM PolyMem bitstream and testbench - go in the CPUCode folder and run:
```
make RUNRULE=DFE distclean
make RUNRULE=DFE build
```

This process could take more than one hour if the bitstream has not been synthesized yet.

Once the design is compiled the testbench binary will be located in /RunRules/DFE/binaries/ and called PRFStream. 

* Execute

The following command line parameters can be used:
```
./PRFStream <vector size> <number of executions>
```

Where <vector size> specifies the length of the A, B and C vectors, and <number of executions> determines how many time each kernel will be executed (it is recommended to set the number of executions between 1000 and 10000 in order to reduce the overhead related to starting a computation on the FPGA and get more accurate results)

* Output 

After the execution the results of the STREAM measurements will be shown. An example of the output is shown below.

```

Function    Best Rate MB/s  Avg time     Min time     Max time     Bytes
Load:             235.5     0.013568     0.008871     0.016283  2088960.000000
Offload:          230.7     0.011302     0.009054     0.015617  2088960.000000
Copy:           11919.6     0.000119     0.000117     0.000123  1392640.000000
Scale:          12026.4     0.000120     0.000116     0.000123  1392640.000000
Add:            17884.0     0.000122     0.000117     0.000124  2088960.000000
Triad:          17931.6     0.000121     0.000116     0.000124  2088960.000000
-------------------------------------------------------------

```
