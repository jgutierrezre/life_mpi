#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define gridSize 32
#define cellCount 200
#define simulationSize 100
#define stepDelay 100 // in milliseconds
#define PRINT_GRID 1

void printGrid(int grid[gridSize][gridSize]);
void determineState(int grid[gridSize][gridSize]);
void clearScreen(void);
void randomizeGrid(int grid[gridSize][gridSize], int numCells);

int main() {
    int grid[gridSize][gridSize] = {};
    double duration = 0, averageduration = 0;

    srand(time(NULL));

    clearScreen();
    randomizeGrid(grid, cellCount);
    printGrid(grid);

    clock_t startTime = clock();

    for (int i; i < simulationSize; i++) {
        clock_t startTime = clock();
        determineState(grid);
        clock_t endTime = clock();
        duration = (double)(endTime - startTime) / CLOCKS_PER_SEC;
        averageduration += duration;
        printGrid(grid);
        printf("Iteration #%d/%d\n", i, simulationSize);
        printf("Execution time of determineState: %f seconds\n", duration);
        usleep(stepDelay * 1000);  // 700 ms pause between generations
        clearScreen();
    }
    clock_t endTime = clock();
    duration = (double)(endTime - startTime) / CLOCKS_PER_SEC;

    printGrid(grid);
    printf("Simulation done after %d generations.\n", simulationSize);
    printf("Execution time of life: %f seconds\n", duration);
    printf("Average execution time of each generation: %f seconds\n",
           averageduration / simulationSize);

    return 0;
}

void clearScreen(void) {
    printf("\033[2J\033[1;1H");
}

void randomizeGrid(int grid[gridSize][gridSize], int nc) {
    for (int i = 0; i < nc; i++) {
        int x = rand() % gridSize;
        int y = rand() % gridSize;
        grid[x][y] = 1;
    }
}

void printGrid(int grid[gridSize][gridSize]) {
    if (!PRINT_GRID) return;
    for (int a = 0; a < gridSize; a++) {
        for (int b = 0; b < gridSize; b++) {
            if (grid[a][b] == 1) {
                printf("O ");
            } else {
                printf(". ");
            }
            if (b == gridSize - 1) {
                printf("\n");
            }
        }
    }
}

void determineState(int grid[gridSize][gridSize]) {
    // First pass to calculate new state
    for (int a = 0; a < gridSize; a++) {
        for (int b = 0; b < gridSize; b++) {
            int alive = 0;
            for (int c = -1; c <= 1; ++c) {
                for (int d = -1; d <= 1; ++d) {
                    if (c == 0 && d == 0)
                        continue;

                    int x = a + c;
                    int y = b + d;
                    if (x >= 0 && x < gridSize && y >= 0 && y < gridSize &&
                        (grid[x][y] & 1)) {
                        ++alive;
                    }
                }
            }
            if (alive < 2 || alive > 3) {
                grid[a][b] &= 1;
            } else if (alive == 3 || grid[a][b] & 1) {
                grid[a][b] |= 2;
            }
        }
    }

    // Second pass to normalize state back to 0 and 1
    for (int a = 0; a < gridSize; a++) {
        for (int b = 0; b < gridSize; b++) {
            grid[a][b] >>=
                1;  // Shift to get the new state into the least significant bit
        }
    }
}
