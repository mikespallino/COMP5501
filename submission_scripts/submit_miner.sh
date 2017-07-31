#!/bin/bash
# set the number of nodes and processes per node
#PBS -l nodes=3:ppn=1

# set max wallclock time
#PBS -l walltime=100:00:00

# set name of job
#PBS -N pi_miner_test

# mail alert at start, end and abortion of execution
#PBS -m bea

# send mail to this address
#PBS -M andy@login-node

# use submission environment
#PBS -V

# start job from the directory it was submitted
cd /nfs/supermam

# run through the mpirun launcher
mpirun -np 3 pi_miner