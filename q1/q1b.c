#include <stdio.h>
#include "mpi.h"

#define NUM_PROCESSES 64 
int main() {
    int numprocs, rank, value, namelen;
    char proc_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(proc_name, &namelen);
    // if the number of processes is less than the minimum
    if (numprocs < NUM_PROCESSES) {
        if (rank == 0) {
            printf("Error: This program requires at least %d processes.\n", NUM_PROCESSES);
        }
        MPI_Finalize();
        return 1;
    }

    // Initial process
    if (rank == 0) {
        value = 0;
        printf("Process %d from Node %s received value: %d\n", rank, proc_name, value);
        value ++;
        MPI_Send(&value, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    } else {
        MPI_Recv(&value, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process %d from Node %s received value: %d\n", rank, proc_name, value);
        
        if (value < NUM_PROCESSES) {
            value++;
            if (rank < NUM_PROCESSES - 1) {
                MPI_Send(&value, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
            }
        } else {
            value -= 2;
            if (rank > 0) {
                MPI_Send(&value, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);
            }
        }
    }
    MPI_Finalize();
    return 0;
}
