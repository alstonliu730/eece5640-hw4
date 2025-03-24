# High Performance Computing Homework 4
In this homework, we work with OpenMPI to take advantage of the number of cores and interconnects a HPC facility can handle. It's a simple library to send messages between processes and nodes. It's efficiently creates programs that can utilize multiple nodes using the Message-Passing Model.

## Running the programs
1. Connect to the explorer cluster at `explorer.northeastern.edu`
2. Load the module that's needed for OpenMPI: `module load OpenMPI`
3. Create the executable file: `make all`
4. Batch the script using the SLURM command: `sbatch q1*.script`
5. Wait until the script is finishing executing
6. Read into the output of the script by finding a `slurm*.out` file in the same directory

## Question 1
In this question, we create a simple MPI program to familiarize ourselves with the syntax and observe how the messages are being passed.

## Question 2
In this question, we create a histogramming program using OpenMPI and C. The histogram will have around 8 million data points to sample in the range 1 - 100,000. The number of bins will vary for each part of the question. We will benchmark each part and create a graph. In each part, we vary using the number of nodes and processes.
