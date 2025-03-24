#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>

// Function to generate random integers in the range [1, 100000]
int* generateRandomData(int numElements, int seed) {
    int* data = (int*)malloc(numElements * sizeof(int));
    if (data == NULL) {
        perror("Failed to allocate memory for data");
        return NULL;
    }
    srand(seed);
    for (int i = 0; i < numElements; ++i) {
        data[i] = (rand() % 100000) + 1;
    }
    return data;
}

int main(int argc, char** argv) {
    // Initialize MPI
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Parameters
    const int dataSize = 8000000;
    const int numBins = 128;
    const int rootRank = 0;
    const int maxProcsPerNode = 16;

    // Determine the number of nodes
    int numNodes = (world_size + maxProcsPerNode - 1) / maxProcsPerNode;

    // Check if the number of nodes is 2 or 4
    if (numNodes != 2 && numNodes != 4) {
        if (world_rank == rootRank) {
            fprintf(stderr, "Error: The number of nodes must be 2 or 4.  Currently %d nodes.\n", numNodes);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Calculate bins per process.  With this many processes, we can't assign
    // one bin per process.
    int binsPerProcess = numBins / world_size;
    int extraBins = numBins % world_size;  // Handle uneven distribution
    int myNumBins;
    int myStartBin;

    // Determine the bins for this process
    if (world_rank < extraBins) {
        myNumBins = binsPerProcess + 1;
        myStartBin = world_rank * (binsPerProcess + 1);
    } else {
        myNumBins = binsPerProcess;
        myStartBin = extraBins * (binsPerProcess + 1) + (world_rank - extraBins) * binsPerProcess;
    }
    // Calculate the range of each bin
    int minVal = 1;
    int maxVal = 100000;
    double binWidth = (double)(maxVal - minVal + 1) / numBins;

    // Generate the data
    int* data = NULL;
    int seed = 42;
    if (world_rank == rootRank) {
        data = generateRandomData(dataSize, seed);
         if (data == NULL) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Distribute the data among processes
     int localDataSize = dataSize / world_size;
    int* localData = (int*)malloc(localDataSize * sizeof(int));
    if (localData == NULL) {
        perror("Rank : Failed to allocate memory for local data\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Scatter(data, localDataSize, MPI_INT,
                localData, localDataSize, MPI_INT,
                rootRank, MPI_COMM_WORLD);

    // Calculate the local histogram for the bins assigned to this process.
    int* localHistogram = (int*)calloc(myNumBins, sizeof(int));
     if (localHistogram == NULL) {
        perror("Rank : Failed to allocate memory for local histogram\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    for (int i = 0; i < localDataSize; ++i) {
        int binIndex = (int)((localData[i] - minVal) / binWidth);
        if (binIndex >= myStartBin && binIndex < myStartBin + myNumBins) {
             localHistogram[binIndex - myStartBin]++;
        }
    }
    // Gather the local histograms at the root process
    int* globalHistogram = NULL;
    if (world_rank == rootRank) {
        globalHistogram = (int*)calloc(numBins, sizeof(int));
         if (globalHistogram == NULL) {
            perror("Rank : Failed to allocate memory for global histogram\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Use MPI_Gatherv to gather the variable-sized local histograms
    int* recvcounts = NULL;
    int* displs = NULL;
    if (world_rank == rootRank) {
        recvcounts = (int*)malloc(world_size * sizeof(int));
        displs = (int*)malloc(world_size * sizeof(int));
        int k;
        for(k=0; k< world_size; k++){
            if(k < extraBins){
                recvcounts[k] = binsPerProcess + 1;
                displs[k] = k * (binsPerProcess + 1);
            }
            else{
                recvcounts[k] = binsPerProcess;
                displs[k] = extraBins * (binsPerProcess + 1) + (k - extraBins) * binsPerProcess;
            }
        }
    }
    MPI_Gatherv(localHistogram, myNumBins, MPI_INT,
                  globalHistogram, recvcounts, displs, MPI_INT,
                  rootRank, MPI_COMM_WORLD);

    // Print the global histogram on the root process
    if (world_rank == rootRank) {
        printf("Histogram of %d values into %d bins:\n", dataSize, numBins);
        for (int i = 0; i < numBins; ++i) {
            printf("Bin %d: %d values\n", i + 1, globalHistogram[i]);
        }
    }

    // Clean up memory
    if (world_rank == rootRank) {
        free(data);
        free(globalHistogram);
        free(recvcounts);
        free(displs);
    }
    free(localData);
    free(localHistogram);

    // Finalize MPI
    MPI_Finalize();
    return 0;
}
