#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NUM_INTEGERS 8000000
#define NUM_BINS 128
#define MAX_VAL 100000
#define ROOT_RANK 0
#define MAX_PROCS_PER_NODE 16

// fill data array with random numbers
void generate_random_data(int *data, int size, time_t seed) {
    // allocate dynamic memory for the size
    data = (int*)malloc(size * sizeof(int));
    if (data == NULL) {
        printf("Failed to allocate memory for data.\n");
        return NULL;
    }

    // initialize pseudo random number generator
    srand(seed);

    // Generate integers bwt. 1 and MAX_VAL
    for (int i = 0; i < size; i++) {
        data[i] = rand() % MAX_VAL + 1; 
    }
}

int main(int argc, char** argv) {
    int world_size, world_rank;
    char proc_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    // Determine number of nodes
    int numNodes = (world_size + MAX_PROCS_PER_NODE - 1) / MAX_PROCS_PER_NODE;

    // calculate the bin range per process
    int binsPerProcess = NUM_BINS / world_size;
    int extraBins = NUM_BINS % world_size;
    int currNumBins;
    int myStartBin;

    // Determine the bins for this process
    if (world_rank < extraBins) {
        myNumBins = binsPerProcess + 1;
        myStartBin = world_rank * (binsPerProcess + 1);
    } else {
        myNumBins = binsPerProcess;
        myStartBin = world_rank * (binsPerProcess + 1) + (world_rank - extraBins) * binsPerProcess;
    }

    // Calculate the range of each bin
    double binWidth = (double)(MAX_VAL) / NUM_BINS;

    // Generate random data
    int* data = NULL;
    if (world_rank == ROOT_RANK) {
        time_t seed = time(NULL);
        generate_random_data(data, NUM_INTEGERS, seed);
        if (data == NULL) {
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
    }

    // Scatter a part of the data array to all processes
    int localDataSize = NUM_INTEGERS / world_size;
    int* localData = (int*) malloc(localDataSize * sizeof(int));
    if (localData == NULL) {
        printf("Rank %d: Failed to allocate memory for local data\n", world_rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Scatter(data, localDataSize, MPI_INT, 
            localData, localDataSize, MPI_INT, ROOT_RANK, MPI_COMM_WORLD);
    
    // Create the local histogram array
    int* localHistogram = (int*) calloc(NUM_BINS, sizeof(int));
    if (localHistogram == NULL) {
        printf("Rank %d: Failed to allocate memory for local histogram\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Calculate the local histogram
    for (int i = 0; i < localDataSize; ++i) {
        int binindex = (int)((localData[i] - minVal) / binWidth);
        // Handle the edge case where val == MAX_VAL
        if (binindex == numBins) {
            binIndex = numBins - 1;
        }

        // increment the bin associated with the data
        localHistogram[binIndex]++;
    }

    // Gather the local histogram at the root process
    int* globalHistogram = NULL;
    if (world_rank == ROOT_RANK) {
        globalHistogram = (int*) calloc(NUM_BINS, sizeof(int));
        if (globalHistogram == NULL) {
            printf("Rank %d: Failed to allocate memory for global histogram\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Reduce(localHistogram, globalHistogram, NUM_BINS, MPI_INT, MPI_SUM, ROOT_RANK, MPI_COMM_WORLD);

    // Free all memory
    if (world_rank == ROOT_RANK) {
        free(data);
        free(globalHistogram);
    }
    free(localData);
    free(localHistogram);
    
    MPI_Finalize();
    return 0;
}