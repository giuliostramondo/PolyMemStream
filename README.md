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
To use STREAM for the assessment of PolyMem, we must design the STREAM framework using Maxeler's toolchain and PolyMem. A high-level view of our design  is presented in Figure ?. The host is connected through the PCI-e to our STREAM design, and starts the computation by sending the Vector Sizes and Mode parameters to define the behavior of the Controller. The Controller generates the write and read signals for PolyMem and selects the correct input for PolyMem write port by driving the the two MUXs. The signals Wi, Wj| and Wshape and Ri, Rj, Rshap signals identify the write/read locations for the elements to be stored in/retrieved from PolyMem. Lastly, using the DEMUX, the controller selects the right output stream (from A_OUT, B_OUT, C_OUT) to correctly retrieve the data from the PolyMem.

![STREAM PolyMem](https://raw.githubusercontent.com/giuliostramondo/PolyMemStream/master/images/StreamImplementation.png)
