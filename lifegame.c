#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

const int W = 60;
const int H = 40;

int _sum_around(char board[H][W], int x, int y) {
    int sum = 0;
    int d[8][2]= { {-1,-1}, {0, -1}, {1, -1},
                   {-1, 0},          {1,  0},
                   {-1, 1}, {0,  1}, {1,  1} };

    for (int i= 0; i < 8; i++) {
        int lx = x + d[i][0], ly = y + d[i][1];
        if (0 <= lx && lx < W &&
            0 <= ly && ly < H && board[ly][lx] == '*') {
            sum += 1;
        }
    }

    return sum;
}

void clear()
{
    printf("\033[2J");
}

void home()
{
    printf("\033[0;0f");
}

bool load_board(char *filename, char board[H][W])
{
    FILE* fp;
    int c;

    if ((fp = fopen(filename, "r")) == NULL) {
        return false;
    }
    for (int y = 0; y< H; y++) {
        for (int x = 0; x< W; x++) {
            if ((c = fgetc(fp)) == EOF) {
                fclose(fp);
                return false;
            } else if (c == '\n') { // if line ends before the last column,
                for (; x< W; x++) { // fill remaining to '0'.
                    board[y][x] = ' ';
                }
            } else if (c != ' ') {
                board[y][x] = '*';
            } else {
                board[y][x] = ' ';
            }
        }
	if (c != '\n') {
	    while (fgetc(fp) != '\n') ;
	}
    }
    fclose(fp);
    return true;
}
void initialize_board(char board[H][W])
{
    for (int y = 0; y< H; y++) {
        for (int x = 0; x< W; x++) {
            if (rand() % 100 < 30) {
                board[y][x] = '*';
            } else {
                board[y][x] = ' ';
            }
        }
    }
}

void display_board(char board[H][W])
{
    for (int y = 0; y< H; y++) {
        for (int x = 0; x< W; x++) {
            printf("%c", board[y][x]);
        }
        printf("\n");
    }
}

void refresh_board(char board[H][W])
{
    char c, sum_mtx[H][W];
    int flag = 0;

    for (int y = 0; y< H; y++) {
        for (int x = 0; x< W; x++) {
            sum_mtx[y][x] = _sum_around(board, x, y);
        }
    }
    for (int y = 0; y< H; y++) {
        for (int x = 0; x< W; x++) {
            int sum = sum_mtx[y][x];
            int c = ' ';
            if (// survivor
                (board[y][x] == '*' && sum == 2 || sum == 3)
             || // newborn 
                (board[y][x] == ' ' && sum == 3)
            ) {
                c = '*';
            }
            board[y][x] = c;
        }
    }
}

int main(int argc, char** argv)
{
    char board[H][W];
    int generation = 0;
    srand((unsigned int) time(NULL));

    if (argc == 2) {
        load_board(argv[1], board);
    } else {
        initialize_board(board);
    }

    clear();
    while (true) {
        home();
        printf("GEN=%d\n", ++generation);
        display_board(board);
        refresh_board(board);
        if (generation > 300) {
            printf("\nreset scene\n");
            initialize_board(board);
            generation = 1;
            sleep(2);
            clear();
        }
        usleep(100*1000);
    }
}
