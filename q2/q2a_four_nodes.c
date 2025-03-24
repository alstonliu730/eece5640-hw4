#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>

#define MAX_VAL 100000
#define MIN_VAL 1
#define DATA_SIZE 8000000
#define NUM_BINS 128
#define ROOT_RANK 0

// Function to generate random integers in the range [1, 100000]
int* generateRandomData(int numElements, time_t seed) {
    int* data = (int*)malloc(numElements * sizeof(int));
    if (data == NULL) {
        printf("Failed to allocate memory for data");
        return NULL;
    }
    srand(seed);
    for (int i = 0; i < numElements; ++i) {
        data[i] = (rand() % 100000) + 1;
    }
    return data;
}

int main(int argc, char** argv) {
    // start benchmark
    double total;

    // Initialize MPI
    MPI_Init(&argc, &argv);
    double localTotal, t1, t2;
    t1 = MPI_Wtime();

    int world_rank, world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // get number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); // get current process id
   
    // Calculate the range of each bin
    double binWidth = (double)(MAX_VAL - MIN_VAL + 1) / NUM_BINS;

    // Generate the data array
    int* data = NULL;
    
    // let the root rank create the data array
    if (world_rank == ROOT_RANK) {
        time_t seed = time(NULL);
        data = generateRandomData(DATA_SIZE, seed);
        if (data == NULL) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Distribute the data among processes
    int localDataSize = DATA_SIZE / world_size;
    int* localData = (int*)malloc(localDataSize * sizeof(int));
    if (localData == NULL) {
        printf("Rank %d: Failed to allocate memory for local data\n", world_rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Scatter(data, localDataSize, MPI_INT,
                localData, localDataSize, MPI_INT,
                ROOT_RANK, MPI_COMM_WORLD);

    // Calculate the local histogram for the bins assigned to this process.
    int* localHistogram = (int*)calloc(NUM_BINS, sizeof(int));
    if (localHistogram == NULL) {
        printf("Rank %d: Failed to allocate memory for local histogram\n", world_rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    for (int i = 0; i < localDataSize; ++i) {
        int binIndex = (int)((localData[i] - MIN_VAL) / binWidth);
        localHistogram[binIndex]++;
    }

    // Gather the local histograms at the root process
    int* globalHistogram = NULL;
    if (world_rank == ROOT_RANK) {
        globalHistogram = (int*)calloc(NUM_BINS, sizeof(int));
         if (globalHistogram == NULL) {
            fprintf(stderr, "Rank %d: Failed to allocate memory for global histogram\n", world_rank);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Reduce(localHistogram, globalHistogram, NUM_BINS, MPI_INT, MPI_SUM, ROOT_RANK, MPI_COMM_WORLD);

    // Print the global histogram on the root process
    if (world_rank == ROOT_RANK) {
        long sum = 0;
        printf("Histogram of %d values into %d bins:\n", DATA_SIZE, NUM_BINS);
        for (int i = 0; i < NUM_BINS; ++i) {
            printf("Bin %d: %d values\n", i + 1, globalHistogram[i]);
            sum += globalHistogram[i];
        }
        printf("Actual total points: %ld\n", sum);
    }

    // Clean up memory
    if (world_rank == ROOT_RANK) {
        free(data);
        free(globalHistogram);
    }
    free(localData);
    free(localHistogram);

    // Benchmark ends
    t2 = MPI_Wtime();
    localTotal = t2 - t1;

    MPI_Reduce(&localTotal, &total, 1, MPI_DOUBLE, MPI_SUM, ROOT_RANK, MPI_COMM_WORLD);
    
    if (world_rank == ROOT_RANK) {
        printf("Time Elapsed: %lfms\n", total);
    }

    // Finalize MPI
    MPI_Finalize();
    return 0;
}
