#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

const int W = 60;
const int H = 40;

// game flags
bool random_color = false, debug = false, torus = false;

/// utility routines for terminal control
static void _clear() { printf("\033[2J"); }
static void _home() { printf("\033[0;0f"); }
static void _color(int c) {
    if (c < 0) { printf("\033[0m");
    } else     { printf("\033[48;5;%dm", c); }
}
static void _reset_terminal() { _color(-1); }

static int _color_pallete(int col, int cbank)
{
    if (col == 0)
        return 0;
    else
        return (col-1) + 0x8*(cbank-1) + 1;
}

void usage(char *progname)
{
    printf( "Usage:   %s [-h] [-w wait] [-c colormap#] [-d] [-t] [filename]\n"
            "ColormapList:", progname);
    for (int i = 1; i<= 32; i++) {
        printf("\nMAP %2d: ", i);
        for (int j = 1; j<= 8; j++) {
            _color(_color_pallete(j, i));
            printf("　");
        }
        _reset_terminal();
    }
    printf("\n");

    exit(1);
}

/// [private] Function to count the number of live points in the 
//  8-neighborhood of specified point.
static int _sum_surrounding(char board[H][W], int x, int y) {
    int sum = 0;
    int d[8][2]= { {-1,-1}, {0, -1}, {1, -1},
                   {-1, 0},          {1,  0},
                   {-1, 1}, {0,  1}, {1,  1} };

    for (int i= 0; i < 8; i++) {
        int lx = x+d[i][0], ly = y+d[i][1];
        if (torus) {
            lx = (lx + W) % W;
            ly = (ly + H) % H;
        }
        if (0 <= lx && lx < W &&
            0 <= ly && ly < H && board[ly][lx] == 1) {
            sum += 1;
        }
    }

    return sum;
}

/// [private] Function to count the number of live points in the
//  8-neighborhood of each point.
static void _count_board(char board[H][W], char ret[H][W])
{
    for (int y = 0; y< H; y++) {
        for (int x = 0; x< W; x++) {
            ret[y][x] = _sum_surrounding(board, x, y);
        }
    }
}

/// [private] Function to get "fingerprint" of the board situation
//  (in the current impl. it's just by counting live point.)
static long _hash_board(char board[H][W])
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

    if ((fp = fopen(filename, "rt")) == NULL) {
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

static int _check_stride(unsigned int ary[], int stride)
{
    int level = 0;
    for (int j = 0; j < stride; j++) {
        int elem = ary[j];
        level += 1;
        for (int i = j+stride; i < N; i+=stride) {
            if (elem == ary[i]) {
                level += 1;
            }
        }
    }
    return level;
}

/// Check whether the board is converged or not.
//  The method for checking is,
//   1) calculate the fingerprint(integer value) of the board
//      [_hash_board()]
//   2) store it to the ring buffer
//   3) check iteration from 1 to MAX_STRIDE in the ring
//      [_check_stride()]
//      
int check_converged(char board[H][W])
{
    static unsigned int hash[N];
    static int hash_ptr = 0;
    int max = 0;

    hash[hash_ptr] = _hash_board(board);
    hash_ptr = (hash_ptr + 1) % N;

    for (int s = 1; s<= MAX_STRIDE-1; s++) {
        int v = _check_stride(hash, s);
        if (v > max) {max = v;}
    }
    return max;
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
        _reset_terminal();
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

static int _shift_array(char* ary[], int size, int n_remove)
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
    char *progname = argv[0];

    atexit(_reset_terminal);
    srand((unsigned int) time(NULL));

    SHIFT_ARGS(1); // cut argv[0](program name)
    while (argc > 0) {
        if (argv[0][0] == '-') {
            char opt = argv[0][1];
            if (opt == 'h') {
                usage(progname);
            } else
            if (opt == 'd') {
                debug = true;
                SHIFT_ARGS(1);
            } else
            if (opt == 't') {
                torus = true;
                SHIFT_ARGS(1);
            } else 
            if (opt == 'w') {
                wait = atoi(argv[1]);
                SHIFT_ARGS(2);
            } else 
            if (opt == 'c') {
                if (isdigit(argv[1][0])) {
                    cbank = atoi(argv[1]);
                    SHIFT_ARGS(2);
                } else { // choose a color at random
                    cbank = (rand() % 32+1);
                    random_color = true;
                    SHIFT_ARGS(1);
                }
            } else {
                fprintf(stderr, "invalid option!\n");
                usage(progname);
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

        int match = check_converged(board);
        if (debug) {
            for (int i = 0; i< W; i++) {
                if (i > match * W/N) {
                    _color(-1);
                } else {
                    _color(10);
                }
                printf("　");
            }
            _color(-1);
        }
        if (++generation > 5000 || match >= N) {
            printf("\n*** CONVERGED!! ***\n");
            initialize_board(board);
            generation = 1;
            if (random_color) {
                cbank = rand() % 32+1;
            }
            sleep(2);
            _clear();
        }
    }
}
