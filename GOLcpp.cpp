/*
* This code replicates the standard behaviour from Conway's Game of Life.
*/

#include <iostream>
#include <SDL.h>
#include <memory>
#include <ctime>

using namespace std;

SDL_Window* window = NULL;
SDL_Surface* windowSurface = NULL;

// in pixels
unsigned int cellSize = 4;
// causes issues going above 700 units due to unmanaged stack
unsigned const int gridWidth = 200;
unsigned const int gridHeight = 200;

unsigned int screenWidth = cellSize * gridWidth;
unsigned int screenHeight = cellSize * gridHeight;

unsigned int white = 0xFF;
unsigned int black = 0x00;

// the percentage of cells which will start alive
unsigned int percentageInit = 10;

/**
* Draws a square on the window surface at the given coordinates.
*
* The size of the square is defined by cellSize, in pixels. A colour is applied (currently only using the R value).
*
* @param coordinates of the cell and the colour of that cell.
*/
void DrawCell(unsigned int x, unsigned int y, unsigned int colour) {
	Uint8* pixel_ptr = (Uint8*)windowSurface->pixels + (y * cellSize * screenWidth + x * cellSize) * 4;

	for (unsigned int i = 0; i < cellSize; i++) {
		for (unsigned int j = 0; j < cellSize; j++) {
			*(pixel_ptr + j * 4) = colour;
			*(pixel_ptr + j * 4 + 1) = colour;
			*(pixel_ptr + j * 4 + 2) = colour;
		}
		pixel_ptr += screenWidth * 4;
	}
}

/**
* Initialises the grid with a set number of randomly placed live cells.
*
* @param reference of the grid to be directly modified.
*/
void InitGrid(unsigned char(&g)[gridWidth][gridHeight]) {
	unsigned int seed = (unsigned)time(NULL);
	srand(seed);
	for (int i = 0; i < gridWidth; i++) {
		for (int j = 0; j < gridHeight; j++) {
			g[i][j] = (rand() % (unsigned int)100 < percentageInit) ? 1 : 0;
			if (g[i][j] == 1) {
				DrawCell(i, j, white);
			}
			else {
				DrawCell(i, j, black);
			}
		}
	}
}

/**
* Calculates number of live neighbours at a given set of co-ordinates.
*
* Wraps around the grid to calculate the number of live neighbours.
*
* @param the xy co-ordinates and the referenced grid to use those co-ordinates on.
* @return the count of live neighbours.
*/
int LiveNeighbourCount(int x, int y, unsigned char(&g)[gridWidth][gridHeight]) {
	bool isLeftEdge = (x == 0);
	bool isRightEdge = (x == gridWidth - 1);
	bool isTopEdge = (y == 0);
	bool isBottomEdge = (y == gridHeight - 1);

	int xL = isLeftEdge ? gridWidth - 1 : x - 1;
	int xR = isRightEdge ? 0 : x + 1;
	int yT = isTopEdge ? gridHeight - 1 : y - 1;
	int yB = isBottomEdge ? 0 : y + 1;
	int count = 0;

	count += g[xL][yT] == 1 ? 1 : 0;
	count += g[x][yT] == 1 ? 1 : 0;
	count += g[xR][yT] == 1 ? 1 : 0;
	count += g[xL][y] == 1 ? 1 : 0;
	count += g[xR][y] == 1 ? 1 : 0;
	count += g[xL][yB] == 1 ? 1 : 0;
	count += g[x][yB] == 1 ? 1 : 0;
	count += g[xR][yB] == 1 ? 1 : 0;

	return count;
}

/**
* Calculates the next value of each cell.
* 
* Calculates based on the current grid state. Utilises referencing to avoid duplicating data. The rules for
* the next state of each cell are based on Conway's standard rules:
* 1) Any live cell with fewer than two live neighbors dies as if caused by underpopulation.
* 2) Any live cell with two or three live neighbors lives on to the next generation.
* 3) Any live cell with more than three live neighbors dies, as if by overpopulation.
* 4) Any dead cell with exactly three live neighbors becomes a live cell, as if by reproduction.
* 
* @param references of the base and duplicate grid.
*/
void CalculateNextGeneration(unsigned char(&g)[gridWidth][gridHeight], unsigned char(&gCopy)[gridWidth][gridHeight]) {
	for (int x = 0; x < gridWidth; x++) {
		for (int y = 0; y < gridHeight; y++) {
			unsigned int neighbourCount = LiveNeighbourCount(x, y, g);
			if (g[x][y] == 1) {
				if ((neighbourCount != 2) && (neighbourCount != 3)) {
					gCopy[x][y] = 0;
					DrawCell(x, y, black);
				}
				else {
					gCopy[x][y] = 1;
					DrawCell(x, y, white);
				}
			}
			else {
				if (neighbourCount == 3) {
					gCopy[x][y] = 1;
					DrawCell(x, y, white);
				}
				else {
					gCopy[x][y] = 0;
					DrawCell(x, y, black);
				}
			}
		}
	}
	memcpy(g, gCopy, sizeof(g));
}

/**
* Main method, establishes SDL window and sets up for drawing.
*/
int main(int argc, char** args) {

	// Ensure SDL initialisation succeeds
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		cout << "Error initializing SDL: " << SDL_GetError() << endl;
		system("pause");
		return 1;
	}
	window = SDL_CreateWindow("Game of Life", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
	// Make sure creating the window succeeded
	if (!window) {
		cout << "Error creating window: " << SDL_GetError() << endl;
		system("pause");
		return 1;
	}

	windowSurface = SDL_GetWindowSurface(window);
	SDL_Event e;

	// char is half the bit size of int
	// more performant method would be to create new char arrays to move the data out of the stack - would unlimit the simulation size
	unsigned char grid[gridWidth][gridHeight], gridCopy[gridWidth][gridHeight];

	InitGrid(grid);

	bool quit = false;
	// Perform iterations while there are no errors and the program has not been closed
	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) quit = true;
		}
		CalculateNextGeneration(grid, gridCopy);

		SDL_UpdateWindowSurface(window);
	}

	system("pause");
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}