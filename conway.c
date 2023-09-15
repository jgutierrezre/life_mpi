#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define gridSize 20

void printGrid(int grid[gridSize][gridSize]);
void determineState(int grid[gridSize][gridSize]);
void clearScreen(void);
void randomizeGrid(int grid[gridSize][gridSize], int numCells);

int main() {
    srand(time(NULL));

    clearScreen();

    int grid[gridSize][gridSize] = {};
    int x, y;
    char nc[10];
    char start[2];

    printf(
        "                         THE GAME OF life                       \n");
    printf("Also known simply as life,\n");
    // Rest of the information and rules of the game

    printf("Enter the number of cells: ");
    if (scanf("%s", nc) != 1) {
        printf("Error: Invalid input\n");
        return 1;
    }

    printf("\n");

    // if (nc[0] != 'r') {
    //     randomizeGrid(grid, atoi(nc));
    //     printGrid(grid);
    // } else {
    //     // Rest of the code for reading the initial grid from the user or a
    //     file
    // }

    randomizeGrid(grid, atoi(nc));
    printGrid(grid);

    printf("Grid setup is done. Start the game? (y/n)\n");

    if (scanf("%s", start) != 1) {
        printf("Error: Invalid input\n");
        return 1;
    }

    if (start[0] == 'y' || start[0] == 'Y') {
        while (1) {
            clock_t startTime = clock();
            determineState(grid);
            clock_t endTime = clock();
            double duration = (double)(endTime - startTime) / CLOCKS_PER_SEC;
            printGrid(grid);
            printf("Execution time of determineState: %f seconds\n", duration);
            usleep(700000);  // 700 ms pause between generations
            clearScreen();
        }
    } else {
        clearScreen();
        return 0;
    }

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
    for (int a = 0; a < gridSize; a++) {
        for (int b = 0; b < gridSize; b++) {
            if (grid[a][b] == 1) {
                printf("O");
            } else {
                printf(".");
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
                    if (c == 0 && d == 0) continue;
                    
                    int x = a + c;
                    int y = b + d;
                    if (x >= 0 && x < gridSize && y >= 0 && y < gridSize && (grid[x][y] & 1)) {
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
            grid[a][b] >>= 1;  // Shift to get the new state into the least significant bit
        }
    }
}

