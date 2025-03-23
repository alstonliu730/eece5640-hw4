#include <stdio.h>
#include <stdint.h>
#include "mpi.h"

#define NUM_PROCESSES 64 
int main() {
    int numprocs, rank, value, namelen;
    uint8_t phase = 0;
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
    if (rank == 0 && phase == 0) {
        value = 0;
        printf("Process %d from Node %s received value in phase %d: %d\n", rank, proc_name, phase, value);
        value ++;
        // Send the value and phase
        MPI_Send(&value, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
        MPI_Send(&phase, 1, MPI_UINT8_T, rank + 1, 0, MPI_COMM_WORLD);
    } 

    // first phase 
    else if (phase == 0) {
        // Receive value and phase
        MPI_Recv(&value, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&phase, 1, MPI_UINT8_T, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // print result 
        printf("Process %d from Node %s received value in phase %d: %d\n", rank, proc_name, phase, value);
        
        if (value < NUM_PROCESSES) {
            value++;
            if (rank < NUM_PROCESSES - 1) {
                MPI_Send(&value, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
                MPI_Send(&phase, 1, MPI_UINT8_T, rank + 1, 0, MPI_COMM_WORLD);
            } else {
                // switch to second phase (reverse flow)
                phase = 1;
                MPI_Send(&value, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);
                MPI_Send(&phase, 1, MPI_UINT8_T, rank - 1, 0, MPI_COMM_WORLD);
            }
        }
    } 
    // second phase
    else if (phase == 1) {
        // Receive value and phase from node in front
        MPI_Recv(&value, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&phase, 1, MPI_UINT8_T, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // print result
        printf("Process %d from Node %s received value in phase %d: %d\n", rank, proc_name, phase, value);

        if (value > 0) {
            value -= 2;
            MPI_Send(&value, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);
            MPI_Send(&phase, 1, MPI_UINT8_T, rank - 1, 0, MPI_COMM_WORLD);
        }

    }
    MPI_Finalize();
    return 0;
}
