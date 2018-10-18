//matmul.c
//Matthew Wark & Kevin Le
//Code as of 5/25/2018 at 10:44AM (matt) and later on at 4:39pm (kevin)
//This program demonstrates the speed up achievable by fully utilizing the cache
//through tiled matrices when performing matrix multiplication.
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>


#define SIZE 1024

volatile __uint64_t A[SIZE][SIZE];
volatile __uint64_t B[SIZE][SIZE];
volatile __uint64_t C[SIZE][SIZE];
volatile __uint64_t D[SIZE][SIZE];

//Initializes two SIZE x SIZE matrices
void init(volatile __uint64_t A[][SIZE], volatile __uint64_t B[][SIZE])
{
    int r, c;
    printf("\nInitializing Matrices...\n");

    for (r = 0; r < SIZE; r++) {
        for (c = 0; c < SIZE; c++) {
            A[r][c] = rand();
            B[r][c] = rand();
        }
    }
        //Just used to verify that the size of each element is 8 bytes
        //int s;
        //s = sizeof(A[0][0]);
        //printf("%d", s);
    printf("Done!\n");
}

//Returns 0 if the two matrices are the same, -1 otherwise
int verify(volatile __uint64_t C[][SIZE], volatile __uint64_t D[][SIZE])
{
    int r, c;

    for (c = 0; c < SIZE; c++) {
        for (r = 0; r < SIZE; r++) {
            if (C[r][c] != D[r][c]) {
                printf("error!\n");
                goto out;
            }

        }
    }
    return 0;
out:
    return -1;
}

//Standard matrix multiplication
void matmul(volatile __uint64_t A[][SIZE], volatile __uint64_t B[][SIZE])
{
    printf("\nRunning matmul...\n");
    int rowA, colB, idx;

    for (rowA = 0; rowA < SIZE; rowA++) {
        for (colB = 0; colB < SIZE; colB++) {
            for (idx = 0; idx < SIZE; idx++) {
                C[rowA][colB] += A[rowA][idx] * B[idx][colB];
            }
        }
    }
    printf("Done!\n");
}

void matmul_transposed(volatile __uint64_t A[][SIZE], volatile __uint64_t B[][SIZE])
{
	int rowA, rowB, idx;

	for (rowA = 0; rowA < SIZE; rowA++) {
		for (rowB = 0; rowB < SIZE; rowB++) {
			for (idx = 0; idx < SIZE; idx++) {
				D[rowA][rowB] += A[rowA][idx] * B[rowB][idx];
			}
		}
	}
}

//Takes a standard array A, and a transposed array B and fmultiplies
//the two matrices in groups that will be referred to as tiles, with
//t prefixes on variables and places the result in the global Matrix D.
void matmul_tiles(volatile __uint64_t A[][SIZE], volatile __uint64_t B[][SIZE], int tiles)
{
    printf("\nRunning matmul_tiles...\n");
    //r = row in current tile, c = col in current tiles
    //tr = current tile row, tc = current tile col
    //tileSize = size of each tile.
    int rowA, rowB, c, tileSize;
    tileSize = SIZE / tiles;

    //tr = tile row, tc = tile col
    //tcA = tcB, trC = trA
    int trA, trB, tcA, tcD;
    tcD = 0;
    //This was a nightmare to program.
    for(trA = 0; trA < tiles; trA++) {
        for(trB = 0; trB < tiles; trB++) {
            for(tcA = 0; tcA < tiles; tcA++) {
                for(rowA = 0; rowA < tileSize; rowA++) {
                    for(rowB = 0; rowB < tileSize; rowB++) {
                        for(c = 0; c < tileSize; c++) {
                            //(trX and tcY) * tileSize is just moving the starting points to the starts of the tiles
                            // + rowX or + c is the offset that points to a location in the tile.
                            D[trA * tileSize + rowA][tcD * tileSize + rowB] += A[trA*tileSize + rowA][tcA * tileSize + c] * B[trB * tileSize + rowB][tcA * tileSize + c];
                            /* Print every step of the tile multiplication
                            printf("\n");
                            print_matrix(D);
                            printf("\n%d\n", tcD);*/
                        }
                    }
                }
            }
            //Reset the tile col of the solution matrix once it reaches the far right
            tcD++;
            if(tcD >= tiles ) {
                tcD = 0;
            }
        }
    }
    //print_matrix(D);
}

//Prints a matrix row by row
void print_matrix(volatile __uint64_t A[][SIZE]) {
    int r, c;
    for(r = 0; r < SIZE; r++) {
        for(c = 0; c < SIZE; c++) {
            printf("%lu \t", A[r][c]);
        }
        printf("\n");
    }
}
//Transposes a matrix by swapping each element to it's proper spot
void transpose (volatile __uint64_t B[][SIZE])
{
    printf("\nTransposing Matrix...\n");
    int r, c, temp2;
    for (c = 0; c < SIZE; c++) {
        for(r = c; r < SIZE; r++) {
            temp2 = B[r][c];
            B[r][c] = B[c][r];
            B[c][r] = temp2;
        }
    }
    printf("Done!\n");
}

//Takes one arg that specifies how many tiles will be used
//Runs matmul without tiles, then with and displays their times and the differences
int main(int argc, char *argv[])
{
    if(argc != 2) {
    printf("Error! Please enter the number of tiles per row of the matrix.\nUsage: ./matmul <number of tiles>\n");
    return 1;
    }
    int tiles;
    tiles = atoi(argv[1]);
    printf("Using %d tiles per row/column\n", tiles);
    clock_t t;
    double time_taken1, time_taken2;


    init(A, B);
    /* Necessary for debugging
    printf("\nMatrix A:\n");
    print_matrix(A);
    printf("\nMatrix B:\n");
    print_matrix(B);*/
    memset((__uint64_t**)C, 0, sizeof(__uint64_t) * SIZE * SIZE);

    t = clock();
    matmul(A, B);
    t = clock() - t;
    time_taken1 = ((double)t)/CLOCKS_PER_SEC; // in seconds

    printf("Matmul took %f seconds to execute.\n", time_taken1);

    transpose(B);
    //printf("\nMatrix B Transposed:\n");
    //print_matrix(B);
    t = clock();
    matmul_tiles(A, B, tiles);
    //printf("\nSolution:\n");
    //print_matrix(C);
    //printf("\nMy Answer:\n");
    //print_matrix(D);
    t = clock() - t;
    time_taken2 = ((double)t)/CLOCKS_PER_SEC; // in seconds
    double speed = (int)((time_taken1 - time_taken2) / time_taken2 * 100);
    int speedup = (int) speed;

    printf("Matmul_tiles (with B transposed) took %f seconds to execute\nVerify status %d\nSpeedup of %d%%\n\n", time_taken2, verify(C, D), speedup);
}
