#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

const int W = 60;
const int H = 40;

/// utility routines for terminal control
void _clear() { printf("\033[2J"); }
void _home() { printf("\033[0;0f"); }
void _color(int c) {
    if (c < 0) { printf("\033[0m");
    } else     { printf("\033[48;5;%dm", c); }
}

/// [private] Function to count the number of live points in the 
//  8-neighborhood of specified point.
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

/// [private] Function to count the number of live points in the
//  8-neighborhood of each point.
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

bool load_board(char *filename, char board[H][W])
{
    FILE* fp;
    int x,y;
    int c;

    if ((fp = fopen(filename, "r")) == NULL) {
        return false;
    }
    for (y = 0; y< H; y++) {
        for (x = 0; x< W; x++) {
            if ((c = fgetc(fp)) == EOF) break;
            else if (c == '\n') { // if line ends before the last column,
                for (; x< W; x++) { // fill remaining to '0'.
                    board[y][x] = 0;
                }
            } else if (c != ' ') {
                board[y][x] = 1;
            } else {
                board[y][x] = 0;
            }
        }
        if (c == EOF) break;
        else if (c != '\n') {
            while (fgetc(fp) != '\n') ;
        }
    }
    fclose(fp);
    for (; y< H; y++) { // fill remaining lines all ' '(blank)
        for (x=0; x< W; x++) { board[y][x] = 0; }
    }
    return true;
}

#define N 16      // the size of ring buffer (MUSTBE even)
const int MAX_STRIDE = N/2;

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

/// Check whether the board is converged or not.
//  The method for checking is,
//   1) calculate the fingerprint(integer value) of the board
//      [_hash_board()]
//   2) store it to the ring buffer
//   3) check iteration from 1 to MAX_STRIDE in the ring
//      [_check_stride()]
//      
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

int _color_pallete(int col, int cbank)
{
    return col*2 + 0x10*(cbank&0x0f);
}

void display_board(char board[H][W], int cbank)
{
    char sum_mtx[H][W];

    if (cbank) {
        _count_board(board, sum_mtx);
    }
    for (int y = 0; y< H; y++) {
        for (int x = 0; x< W; x++) {
            int c = 0;
            if (board[y][x]) {
                if (cbank) {c = _color_pallete(sum_mtx[y][x], cbank);}
                else       {c = 255; }
            } 
            _color(c);
            printf("　");
        }
        _color(-1);
        printf("\n");
    }
}

/// Function to advance the game state by one move.
void update_board_state(char board[H][W])
{
    char sum_mtx[H][W];

    _count_board(board, sum_mtx); // count living points for each point 
                                  //
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

int _shift_array(char* ary[], int size, int n_remove)
{
    int new_size = size - n_remove;
    for (int i = 0; i< new_size; i++) {
        ary[i] = ary[i+n_remove];
    }
    return new_size;
}

#define _MS *1000  // dirty...
#define SHIFT_ARGS(n)  {argc = _shift_array(argv, argc, n);}

int main(int argc, char** argv)
{
    char board[H][W];
    int generation = 1;
    int wait = 50;  // ms
    int cbank = 0;

    srand((unsigned int) time(NULL));

    SHIFT_ARGS(1); // cut executable name
    while (argc > 0) {
        if (argv[0][0] == '-') {
            char opt = argv[0][1];
            if (opt == 'w') {
                wait = atoi(argv[1]);
                SHIFT_ARGS(2);
            } else 
            if (opt == 'c') {
                if (isdigit(argv[1][0])) {
                    cbank = atoi(argv[1]);
                    SHIFT_ARGS(2);
                } else { // choose color randomly
                    cbank = -(rand() % 15+1);
                    SHIFT_ARGS(1);
                }
            }
        } else break;
    }

    if (argc > 0) {
        if (!load_board(argv[0], board)) {
            fprintf(stderr, "something wrong with '%s'... abort\n", argv[1]);
            return -1;
        }
    } else {
        initialize_board(board);
    }

    _clear();
    while (true) {
        _home();
        printf("GEN=%5d\n", generation);
        display_board(board, cbank);
        update_board_state(board);
        usleep(wait _MS);

        if (++generation > 5000 || check_converged(board)) {
            printf("\n*** CONVERGED!! ***\n");
            initialize_board(board);
            generation = 1;
            if (cbank < 0) {
                cbank = -(rand() % 0x0f+1);
            }
            sleep(2);
            _clear();
        }
    }
}
