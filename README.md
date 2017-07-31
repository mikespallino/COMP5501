# COMP5501

## SuperMAM

SuperMAM is a super computing environment built from a cluster of Raspberry Pis. This repository contains code and setup
scripts for the cluster.

Code for SuperMAM is written in either C or Fortran90. SuperMAM uses TAU (Tuning and Analysis and Utilities) for the performance analysis of parallel programs. One node in the cluster, called the login node, is used to start a program. Portable Batch System / TORQUE is used to distribute the work over the rest of the cluster’s compute nodes. Message passing between the login node and the compute nodes is done through OpenMPI.

All job submission scripts are located in the /submission_scripts.
All compiled binaries are included in /bin.
All source code is included in /src.

## Three Main Programs

There are three main programs included in this repository. An integral solver, a dictionary password attack, and a
basic form of bitcoin mining.
