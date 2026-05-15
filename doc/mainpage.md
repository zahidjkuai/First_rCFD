# Introduction {#mainpage}

[TOC]

## About Recurrence CFD

Recurrence CFD (rCFD) is a method for efficient time-extrapolation of pseudo-periodic
CFD simulations<sup>[1](#ref1),[2](#ref2)</sup>.
This library is a reference implementation of the transport-based recurrence CFD
method developed at the Department of Particulate Flow Modelling,
Johannes Kepler University, Linz, Austria.


## Building rCFD

If you have a [GitHub account setup with SSH](https://docs.github.com/en/authentication/connecting-to-github-with-ssh),
just do this:

    git clone git@github.com:ParticulateFlow/rCFD-PFM.git

Otherwise, clone the repository via HTTPS:

    git clone https://github.com/ParticulateFlow/rCFD-PFM.git


### Building with ANSYS Fluent UDF System

#### Using the TUI

For compiled UDFs on Linux systems, three ANSYS Fluent files are required to build
your shared UDF library: **makefile.udf**, **makefile.udf2**, and **user.udf**.

1. Navigate to your case directory and make a directory that will store your UDF
library (e.g., **libudf_rcfd_run**).

2. Copy **makefile.udf2** from **path/ansys_inc/{version}/fluent/fluent{version}/src/udf**
to the library directory (e.g., **libudf_rcfd_run**), and name it **makefile**.
Note that path is the directory in which you have installed ANSYS Fluent.

3. In the library directory you just created in step 1, make a directory that
will store your source files and name it **src**.

4. Copy all source files from **rCFD-PFM/src** and your case-specific **rCFD_user.h** file
to that source directory (**libudf_rcfd_run/src**).

5. Copy **makefile.udf** from **path/ansys_inc/{version}/fluent/fluent{version}/src/udf**
to the **libudf_rcfd_run/src** directory, and name it **makefile**.

6. Identify the architecture name of the machine on which you are running (e.g.,
**lnamd64**). This can be done by either typing the command (fluent-arch) in the
ANSYS Fluent TUI window, or running the ANSYS Fluent utility program *fluent_arch*
at the command line of a Linux shell.

7. In the library directory (e.g., **libudf_rcfd_run**), create an *architecture*
directory that is named after the architecture identifier determined in the
previous step (e.g., **lnamd64**).

8. In the *architecture* directory, create directories named after the ANSYS
Fluent versions for which you want to build shared libraries (e.g.,
**lnamd64/3ddp_node** and **lnamd64/3ddp_host**). Possible versions are:  
    | Node/Host Version Names     | Meaning             |
    |-----------------------------|---------------------|
    | *2d_node* and *2d_host*     | single-precision 2D |
    | *3d_node* and *3d_host*     | single-precision 3D |
    | *2ddp_node* and *2ddp_host* | double-precision 2D |
    | *3ddp_node* and *3ddp_host* | double-precision 3D |

9. Edit the **user_run.udf** file in **rCFD-PFM/src/user_run.udf** to set the
ANSYS Fluent path.

    FLUENT_INC =
        The path to your ANSYS Fluent directory (e.g., path/ansys_inc/{version}/fluent).

10. Copy **user_run.udf** which you edited in the previous step from **rCFD-PFM/src/user_run.udf**
to all the version sub-folders that you have made (e.g., **libudf_rcfd_run/lnamd64/3ddp_node**
and **libudf_rcfd_run/lnamd64/3ddp_host**), and name it **user.udf**.

11. In your library directory (e.g., **libudf_rcfd_run**), execute the Makefile
by typing a command that begins with *make* and includes the architecture of the
machine you will run ANSYS Fluent on, which you identified in a previous step.
For example, for the Linux (**lnamd64**) architecture type:

    make "FLUENT_ARCH=lnamd64"

### Building Standalone Shared Library

Simply build with:
```
cd rCFD-PFM
mkdir -p src-build
cd src-build
cmake ../src/
make
```

Optionally, install the library and header file for system wide use afterwards:
```
sudo make install
```


#### Build Switches

| CMake Switch | Options       | Default | Comment                                      |
|--------------|---------------|---------|----------------------------------------------|
| `USE_MPI`    | `ON` \| `OFF` | `OFF`   | If `ON`, rCFD will be built with MPI support |


## Dependencies

* A C99-compatible C compiler.
* [ANSYS Fluent](https://www.ansys.com/) or a recent version of [CMake](https://cmake.org/) for the standalone library
* An MPI installation such as [OpenMPI](https://www.open-mpi.org/) for a parallel version of the standalone library
* A [Doxygen](https://www.doxygen.nl/) installation for generating this documentation.

## References

###### [1] [T. Lichtenegger and S. Pirker, Chem. Eng. Sci., 153 (2016)](https://doi.org/10.1016/j.ces.2016.07.036) {#ref1}
###### [2] [S. Pirker and T. Lichtenegger, Chem. Eng. Sci., 188 (2018)](https://doi.org/10.1016/j.ces.2018.04.059) {#ref2}
