# COMP5501 - Senior Project

## SuperMAM
Authors: Matthew Smith, Michael Spallino, Andy Tse


SuperMAM is a super computing environment built from a cluster of Raspberry Pis. This repository contains code and 
setup scripts for the cluster.


Code for SuperMAM is written in either C or Fortran90. SuperMAM uses TAU (Tuning and Analysis and Utilities) for the
performance analysis of parallel programs. One node in the cluster, called the login node, is used to start a program.
Portable Batch System / TORQUE is used to distribute the work over the rest of the cluster’s compute nodes. Message
passing between the login node and the compute nodes is done through OpenMPI.


All job submission scripts are located in the /submission_scripts.


All source code is included in /src.

## Three Main Programs

There are three main programs included in this repository. An integral solver, a dictionary password attack, and a
basic form of bitcoin mining.

## Compiling and Running
Standard compilation
```
gcc      prog.c   -o prog
gfortran prog.f90 -o prog
```

If MPI calls are used
```
mpicc  prog.c   -o prog
mpif90 prof.f90 -o prog
```

If TAU is being used for profiling
```
tau_cc.sh     prog.c   -o prog
tau_mpif90.sh prog.f90 -o prog
```

Pi Miner
```
mpicc pi_miner.c -o pi_miner -lssl -lcrypto
```
