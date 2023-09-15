#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>  // Para generar números aleatorios
#include <ctime>    // Para la semilla de la generación aleatoria
#include <unistd.h>  // Para usleep
#include <chrono>

#define gridSize 100

void printGrid(bool gridOne[gridSize + 1][gridSize + 1]);
void determineState(bool gridOne[gridSize + 1][gridSize + 1]);
void clearScreen(void);
void randomizeGrid(bool gridOne[gridSize + 1][gridSize + 1], int numCells);


int main() {
    printf("\e[2J");
    std::srand(std::time(nullptr));  // Inicializa la semilla aleatoria

    std::cout << "\033[2J\033[1;1H";  // Limpia la pantalla y mueve el cursor a la posición (1, 1)
    bool gridOne[gridSize + 1][gridSize + 1] = {};
    int x, y;
    std::string nc;
    std::string start;
    std::string filename;

    std::cout << "                         THE GAME OF life                       " << std::endl;
    std::cout << "Also known simply as life," << std::endl;
    // Resto de la información y reglas del juego

    std::cout << "Enter the number of cells : ";
    std::cin >> nc;
    std::cout << std::endl;

    if (nc != "r") {
        randomizeGrid(gridOne, 100);
        printGrid(gridOne);
    } else {
        // Resto del código para leer la cuadrícula inicial desde el usuario o un archivo
    }

    std::cout << "Grid setup is done. Start the game? (y/n)" << std::endl;
    std::cin >> start;
    printf("\e[2J");

    if (start == "y" || start == "Y") {
        while (true) {
	    auto startTime = std::chrono::high_resolution_clock::now();
            printGrid(gridOne);
	    
            auto endTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = endTime - startTime;
            std::cout << "Tiempo de ejecución de determineState: " << duration.count() << " segundos" << std::endl;
            determineState(gridOne);
            usleep(700000);  // Pausa de 200 ms entre generaciones
            clearScreen();
        }
    } else {
        std::cout << "\033[0m";  // Restaura los colores originales de la terminal
        clearScreen();
        return 0;
    }

    return 0;
}

void clearScreen(void) {
    std::cout << "\033[2J\033[1;1H";  // Limpia la pantalla y mueve el cursor a la posición (1, 1)
}

void randomizeGrid(bool gridOne[gridSize + 1][gridSize + 1], int nc) {
    for (int i = 0; i < nc; i++) {
        int x = std::rand() % gridSize + 1;  // Genera una coordenada X aleatoria
        int y = std::rand() % gridSize + 1;  // Genera una coordenada Y aleatoria
        gridOne[x][y] = true;
    }
}

void printGrid(bool gridOne[gridSize + 1][gridSize + 1]) {
    for (int a = 1; a < gridSize; a++) {
        for (int b = 1; b < gridSize; b++) {
            if (gridOne[a][b] == true) {
                std::cout << " O ";
            } else {
                std::cout << " . ";
            }
            if (b == gridSize - 1) {
          	      std::cout << std::endl;
            }
        }
    }
}

void compareGrid (bool gridOne[gridSize+1][gridSize+1], bool gridTwo[gridSize+1][gridSize+1]){
    for(int a =0; a < gridSize; a++)
    {
        for(int b = 0; b < gridSize; b++)
        {
                gridTwo[a][b] = gridOne[a][b];
        }
    }
}
//paralelizar este

void determineState(bool gridOne[gridSize + 1][gridSize + 1]) {
    bool gridTwo[gridSize + 1][gridSize + 1] = {};
    compareGrid(gridOne, gridTwo);

    for (int a = 1; a < gridSize; a++) {
        for (int b = 1; b < gridSize; b++) {
            int alive = 0;
            for (int c = -1; c < 2; c++) {
                for (int d = -1; d < 2; d++) {
                    if (!(c == 0 && d == 0)) {
                        if (gridTwo[a + c][b + d]) {
                            ++alive;
                        }
                    }
                }
            }
            if (alive < 2) {
                gridTwo[a][b] = false;
            } else if (alive == 3) {
                gridTwo[a][b] = true;
            } else if (alive > 3) {
                gridTwo[a][b] = false;
            }
        }
    }

    // Copia el contenido de gridTwo en gridOne
    for (int a = 1; a < gridSize; a++) {
        for (int b = 1; b < gridSize; b++) {
            gridOne[a][b] = gridTwo[a][b];
        }
    }
}


