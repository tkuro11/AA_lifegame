import random
import time

# ゲームボードのサイズ設定
WIDTH = 50
HEIGHT = 20

# ゲームボードの初期化
board = [[random.choice([0, 1]) for _ in range(WIDTH)] for _ in range(HEIGHT)]


def _home(): print("\033[0;0f]")

# メインのゲームループ
while True:
    # ゲームボードの表示
    _home()
    for row in board:
        print(''.join(['#' if cell else ' ' for cell in row]))
    print('-' * WIDTH)

    # 新しいゲームボードの生成
    new_board = [[0 for _ in range(WIDTH)] for _ in range(HEIGHT)]

    # ゲームルールに基づいて新しい状態を計算
    for i in range(HEIGHT):
        for j in range(WIDTH):
            # セルの周囲の生存セルの数を数える
            neighbors = sum([
                board[(i-1) % HEIGHT][(j-1) % WIDTH],
                board[(i-1) % HEIGHT][j],
                board[(i-1) % HEIGHT][(j+1) % WIDTH],
                board[i][(j-1) % WIDTH],
                board[i][(j+1) % WIDTH],
                board[(i+1) % HEIGHT][(j-1) % WIDTH],
                board[(i+1) % HEIGHT][j],
                board[(i+1) % HEIGHT][(j+1) % WIDTH]
            ])

            # ゲームルールに基づいて次の状態を決定
            if board[i][j]:
                if neighbors in (2, 3):
                    new_board[i][j] = 1
            else:
                if neighbors == 3:
                    new_board[i][j] = 1

    # ゲームボードの更新
    board = new_board

    # ゲームの更新間隔
    time.sleep(0.02)

