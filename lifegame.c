#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

const int W = 60;
const int H = 40;
const int PERIOD = 1000;

int _sum_around(char board[W][H], int x, int y) {
	int sum = 0;
	for (int ly= y-1; ly <= y+1; ly++) {
		for (int lx= x-1; lx <= x+1; lx++) {
			if (0 <= lx && lx < W &&
			    0 <= ly && ly < H && board[ly][lx] == '*') {
				sum += 1;
			}	
		}
	}
	if (board[y][x] == '*') {
		sum -= 1;
	}
	return sum;
}

void clear()
{
	printf("\033[2J");
}

void initialize_board(char board[W][H])
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

void display_board(char board[W][H])
{
	for (int y = 0; y< H; y++) {
		for (int x = 0; x< W; x++) {
			printf("%c", board[y][x]);
		}
		printf("\n");
	}
}

int refresh_board(char board[W][H])
{
	char c, sum_mtx[W][H];
	int flag = 0;

	for (int y = 0; y< H; y++) {
		for (int x = 0; x< W; x++) {
			sum_mtx[y][x] = _sum_around(board, x, y);
		}
	}
	for (int y = 0; y< H; y++) {
		for (int x = 0; x< W; x++) {
			int sum = sum_mtx[y][x];
			if (board[y][x] == '*') {
				if (sum == 2 || sum == 3) {
					c = '*';
				} else {
					flag += 1;
					c = ' ';
				}
			} else {
				if (sum == 3) {
					flag += 1;
					c = '*';
				} else {
					c = ' ';
				}
			}
			board[y][x] = c;
		}
	}
	return flag;
}

int main(void)
{
	char board[W][H];
	int generation = 0, variance, prev = 1000;
	srand((unsigned int) time(NULL));

	initialize_board(board);
	while (true) {
		clear();
		printf("GEN=%d\n", ++generation);
		display_board(board);
		variance = refresh_board(board);
	   	if (generation > 300 || prev < 5 && prev == variance) {
			generation = 0;
			initialize_board(board);
		}
		prev = variance;
		usleep(100*1000);
	}
}
