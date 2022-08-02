#include "utility.h"
#include <omp.h>

bool sudoku_res = false;

bool solveSuduko(int grid[N][N], int row, int col)
{
    if(sudoku_res) {
        return false;
    }
    
    if (row == N - 1 && col == N) {
        #pragma omp critical
            {
            sudoku_res = true;
					print(grid);
            }
    }
 
    if (col == N) {
        row++;
        col = 0;
    }

    if (grid[row][col] > 0) {
        return solveSuduko(grid, row, col + 1);
    }
 
    for (int num = 1; num <= N; num++)
    {
        if (isSafe(grid, row, col, num))
        {
            #pragma omp task firstprivate(grid, row, col, num) final(row > 0)
            {
                int local_grid[N][N];
                std::copy(&grid[0][0], &grid[0][0]+N*N, &local_grid[0][0]);
                local_grid[row][col] = num;
               
                if (solveSuduko(local_grid, row, col + 1)) {
                    //#pragma omp critical
                    sudoku_res = true;
                }
            }
        }
       
    }

    #pragma omp taskwait
    return sudoku_res;
}

int main()
{   
    int board[N][N];
    
    // This starts the timer.
    std::string line = readInput();;

    int counter = 0;
    char ch;
    std::istringstream iss(line);
    while (iss.get(ch))
    {
        board[counter / N][counter % N] = parseChar(ch);
        counter += 1;
    }

    #pragma omp parallel
    {
    #pragma omp single
    {
    solveSuduko(board, 0, 0);
    }
    }

    //print(GLOBAL_SOL);

    // This stops the timer.
    std::cout << std::endl << "DONE" << std::endl;

    return 0;
}