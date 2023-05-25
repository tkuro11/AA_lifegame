#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

const int W = 60;
const int H = 40;

#define N 16      // the size of hash buffer (MUSTBE even)
const int MAX_STRIDE = N/2;

void _clear() { printf("\033[2J"); }
void _home() { printf("\033[0;0f"); }

int _sum_surrounding(char board[H][W], int x, int y) {
    int sum = 0;
    int d[8][2]= { {-1,-1}, {0, -1}, {1, -1},
                   {-1, 0},          {1,  0},
                   {-1, 1}, {0,  1}, {1,  1} };

    for (int i= 0; i < 8; i++) {
        int lx = x+d[i][0], ly = y+d[i][1];
        if (0 <= lx && lx < W &&
            0 <= ly && ly < H && board[ly][lx] == 1) {
            sum += 1;
        }
    }

    return sum;
}


void _count_board(char board[H][W], char ret[H][W])
{
    for (int y = 0; y< H; y++) {
        for (int x = 0; x< W; x++) {
            ret[y][x] = _sum_surrounding(board, x, y);
        }
    }
}

/// [private] Function to get "fingerprint" of the board situation
//  (in the current impl. it's just by counting live point.)
long _hash_board(char board[H][W])
{
    unsigned int ret = 0;

    for (int y = 0; y< H; y++) {
        for (int x = 0; x< W; x++) {
            ret += board[y][x];
        }
    }
    return ret;
    
}

bool _check_stride(unsigned int ary[], int stride)
{
    for (int j = 0; j < stride; j++) {
        int elem = ary[j];
        for (int i = j+stride; i < N; i+=stride) {
            if (elem != ary[i]) return false;
        }
    }
    return true;
}

bool check_converged(char board[H][W])
{
    static unsigned int hash[N];
    static int hash_ptr = 0;
    bool ret = true;

    hash[hash_ptr] = _hash_board(board);
    hash_ptr = (hash_ptr + 1) % N;

    for (int s = 1; s< MAX_STRIDE; s++) {
        if (_check_stride(hash, s)) return true;
    }
    return false;
}
void initialize_board(char board[H][W])
{
    for (int y = 0; y< H; y++) {
        for (int x = 0; x< W; x++) {
            if (rand() % 100 < 30) {
                board[y][x] = 1;
            } else {
                board[y][x] = 0;
            }
        }
    }
}

void display_board(char board[H][W])
{
    char c[] = {' ', '#'};
    for (int y = 0; y< H; y++) {
        for (int x = 0; x< W; x++) {
            printf("%c", c[board[y][x]]);
        }
        printf("\n");
    }
}

void refresh_board(char board[H][W])
{
    char  sum_mtx[H][W];

    _count_board(board, sum_mtx); // at first, make a counting matrix.
    for (int y = 0; y< H; y++) {
        for (int x = 0; x< W; x++) {
            int sum = sum_mtx[y][x];
            int c = 0;
            if (// survivor
                (board[y][x] == 1 && sum == 2 || sum == 3)
             || // newborn 
                (board[y][x] == 0 && sum == 3)
            ) {
                c = 1;
            }
            board[y][x] = c;
        }
    }
}

int main(void)
{
    char board[H][W];
    int generation = 1;
    srand((unsigned int) time(NULL));

    _clear();
    initialize_board(board);
    while (true) {
        _home();
        printf("GEN=%5d\n", generation);
        display_board(board);
        refresh_board(board);
        usleep(100*1000);

        if (++generation > 5000 || check_converged(board)) {
            printf("\n*** CONVERGED!! ***\n");
            initialize_board(board);
            generation = 1;
            sleep(2);
            _clear();
        }
    }
}
