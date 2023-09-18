#include <mpi.h>
#include <openacc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void printGrid(unsigned char* grid, int height, int width);
void writeGrid(FILE* fp, unsigned char* grid, int height, int width);
void determineState(unsigned char* grid, int height, int width);
void randomizeGrid(unsigned char* grid, int height, int width, int numCells);

int rank = 0, size = 1;
FILE* fp;

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Default values
    // Size of the grid
    unsigned int GRIDSIZE = 32;
    // Number of cells in the grid
    unsigned int CELLCOUNT = 16;
    // Number of generations to simulate
    int SIMULATION_SIZE = 200;
    // Microseconds to wait between generations
    int SIMULATION_SPEED = 100000;
    // Whether to print the initial and final grid to the console
    bool PRINT_GRID = 0;
    // Whether to print the grid every generation to the console
    bool PRINT_GRID_EVERY = 0;
    // Whether to write the initial and final grid to a file
    bool WRITE_GRID = 0;
    // Whether to write every generation to a file
    bool WRITE_GRID_EVERY = 0;
    srand(time(NULL));

    if (rank == 0) {
        int opt;
        while ((opt = getopt(argc, argv, ":g:s:S:p:P:w:W:r:")) != -1) {
            switch (opt) {
                case 'g':
                    GRIDSIZE = atoi(optarg);
                    CELLCOUNT = GRIDSIZE * GRIDSIZE / 2;
                    break;
                case 's':
                    SIMULATION_SIZE = atoi(optarg);
                    break;
                case 'S':
                    SIMULATION_SPEED = atoi(optarg);
                    break;
                case 'p':
                    PRINT_GRID = atoi(optarg);
                    break;
                case 'P':
                    PRINT_GRID_EVERY = atoi(optarg);
                    break;
                case 'w':
                    WRITE_GRID = atoi(optarg);
                    break;
                case 'W':
                    WRITE_GRID_EVERY = atoi(optarg);
                    break;
                case 'r':
                    srand(atoi(optarg));
                    break;
                case ':':
                    printf("Option -%c requires an operand\n", optopt);
                    return 1;
                case '?':
                    printf("Unknown option: -%c\n", optopt);
                    return 1;
            }
        }
    }

    // Broadcast the parsed command-line options to all processes
    MPI_Bcast(&GRIDSIZE, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&SIMULATION_SIZE, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&SIMULATION_SPEED, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&PRINT_GRID, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&PRINT_GRID_EVERY, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&WRITE_GRID, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&WRITE_GRID_EVERY, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Continue with the rest of the program

    int num_devices = acc_get_num_devices(acc_device_nvidia);
#pragma acc set device_num(rank % num_devices) device_type(acc_device_nvidia)

    unsigned char* grid =
        (unsigned char*)calloc(GRIDSIZE * GRIDSIZE, sizeof(unsigned char));

    double duration = 0, averageduration = 0;

    if (rank == 0) {
        if (WRITE_GRID) {
            char filename[100];
            sprintf(filename, "./output/output_%dx%d_%d_%d.txt", GRIDSIZE, GRIDSIZE,
                    size, SIMULATION_SIZE);
            fp = fopen(filename, "w+");
            if (fp == NULL) {
                printf("Error opening file\n");
                return 1;
            }
        }
        printf("Starting simulation of size %dx%d with %d processes and %d GPUs\n",
               GRIDSIZE, GRIDSIZE, size, num_devices);

        randomizeGrid(grid, GRIDSIZE, GRIDSIZE, CELLCOUNT);

        if (PRINT_GRID) {
            printf("Initial Grid:\n");
            printGrid(grid, GRIDSIZE, GRIDSIZE);
            printf("\n");
        }

        if (WRITE_GRID) {
            fprintf(fp, "Starting simulation of size %dx%d with %d processes\n",
                    GRIDSIZE, GRIDSIZE, size);
            fprintf(fp, "Initial Grid:\n");
            writeGrid(fp, grid, GRIDSIZE, GRIDSIZE);
            fprintf(fp, "\n");
        }
    }

    int rowsPerProcess = GRIDSIZE / size;
    unsigned char* subgrid =
        (unsigned char*)calloc((rowsPerProcess + 2) * GRIDSIZE, sizeof(unsigned char));

    MPI_Scatter(&grid[0], rowsPerProcess * GRIDSIZE, MPI_UNSIGNED_CHAR,
                &subgrid[1 * GRIDSIZE], rowsPerProcess * GRIDSIZE, MPI_UNSIGNED_CHAR, 0,
                MPI_COMM_WORLD);
#pragma acc enter data copyin(subgrid[0 : (rowsPerProcess + 2) * GRIDSIZE])

    clock_t startTime, endTime, startTimeAverage, endTimeAverage;

    if (rank == 0) {
        startTime = clock();
    }
    for (int i = 0; i < SIMULATION_SIZE; i++) {
        if (rank == 0) {
            startTimeAverage = clock();
        }
        determineState(subgrid, rowsPerProcess + 2, GRIDSIZE);
        if (rank == 0) {
            endTimeAverage = clock();
            averageduration +=
                (double)(endTimeAverage - startTimeAverage) / CLOCKS_PER_SEC;
        }

        if (PRINT_GRID_EVERY == 1 || WRITE_GRID_EVERY == 1) {
#pragma acc update host(subgrid[0 : (rowsPerProcess + 2) * GRIDSIZE])
            MPI_Gather(&subgrid[1 * GRIDSIZE], rowsPerProcess * GRIDSIZE,
                       MPI_UNSIGNED_CHAR, &grid[0], rowsPerProcess * GRIDSIZE,
                       MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
        }
        if (PRINT_GRID_EVERY == 1) {
            if (rank == 0) {
                printf("Generation %d:\n", i);
                printGrid(grid, GRIDSIZE, GRIDSIZE);
                printf("\n");
                printf("\033[2J\033[1;1H");
                usleep(SIMULATION_SPEED);
            }
        }
        if (WRITE_GRID_EVERY == 1) {
            if (rank == 0) {
                fprintf(fp, "Generation %d:\n", i);
                writeGrid(fp, grid, GRIDSIZE, GRIDSIZE);
                fprintf(fp, "\n");
            }
        }
    }
    if (rank == 0) {
        endTime = clock();
        duration = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    }
#pragma acc update host(subgrid[0 : (rowsPerProcess + 2) * GRIDSIZE])
    MPI_Gather(&subgrid[1 * GRIDSIZE], rowsPerProcess * GRIDSIZE, MPI_UNSIGNED_CHAR,
               &grid[0], rowsPerProcess * GRIDSIZE, MPI_UNSIGNED_CHAR, 0,
               MPI_COMM_WORLD);

    if (rank == 0) {
        if (PRINT_GRID) {
            printf("\nFinal Grid:\n");
            printGrid(grid, GRIDSIZE, GRIDSIZE);
        }
        if (WRITE_GRID) {
            fprintf(fp, "\nFinal Grid:\n");
            writeGrid(fp, grid, GRIDSIZE, GRIDSIZE);
        }
        printf("Simulation done after %d generations.\n", SIMULATION_SIZE);
        printf("Execution time of life: %f seconds\n", duration);
        printf("Average execution time of each generation: %f seconds\n",
               averageduration / SIMULATION_SIZE);
    }

    free(grid);
    free(subgrid);

    MPI_Finalize();

    return 0;
}

void randomizeGrid(unsigned char* grid, int height, int width, int numCells) {
    for (int i = 0; i < numCells; i++) {
        int x = rand() % height;
        int y = rand() % width;
        grid[x * width + y] = 1;
    }
}

void printGrid(unsigned char* grid, int height, int width) {
    for (int a = 0; a < height; a++) {
        for (int b = 0; b < width; b++) {
            if (grid[a * width + b] == 1) {
                printf("O ");
            } else {
                printf(". ");
            }
        }
        printf("\n");
    }
}

void writeGrid(FILE* fp, unsigned char* grid, int height, int width) {
    for (int a = 0; a < height; a++) {
        for (int b = 0; b < width; b++) {
            if (grid[a * width + b] == 1) {
                fprintf(fp, "O ");
            } else {
                fprintf(fp, ". ");
            }
        }
        fprintf(fp, "\n");
    }
}

void determineState(unsigned char* grid, int height, int width) {
    int start = 0, end = height, top = rank - 1, bottom = rank + 1;

    if (rank == 0) {
        start = 1;
        top = MPI_PROC_NULL;
    }

    if (rank == size - 1) {
        end = height - 1;
        bottom = MPI_PROC_NULL;
    }

    MPI_Sendrecv(&grid[(end - 2) * width], width, MPI_UNSIGNED_CHAR, bottom, 0,
                 &grid[start * width], width, MPI_UNSIGNED_CHAR, top, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

    MPI_Sendrecv(&grid[(start + 1) * width], width, MPI_UNSIGNED_CHAR, top, 0,
                 &grid[(end - 1) * width], width, MPI_UNSIGNED_CHAR, bottom, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

// First pass to calculate new state
#pragma acc parallel loop collapse(2) present(grid[0 : height * width])
    for (int a = 1; a < height - 1; a++) {
        for (int b = 0; b < width; b++) {
            int alive = 0;
            for (int c = -1; c <= 1; ++c) {
                for (int d = -1; d <= 1; ++d) {
                    if (c == 0 && d == 0) continue;

                    int x = a + c;
                    int y = b + d;
                    if (x >= start && x < end && y >= 0 && y < width &&
                        (grid[x * width + y] & 1)) {
                        ++alive;
                    }
                }
            }
            if (alive < 2 || alive > 3) {
                grid[a * width + b] &= 1;
            } else if (alive == 3 || grid[a * width + b] & 1) {
                grid[a * width + b] |= 2;
            }
        }
    }

    // Second pass to normalize state back to 0 and 1
#pragma acc parallel loop collapse(2) present(grid[0 : height * width])
    for (int a = start; a < end; a++) {
        for (int b = 0; b < width; b++) {
            // Shift to get the new state into the least significant bit
            grid[a * width + b] >>= 1;
        }
    }
}
