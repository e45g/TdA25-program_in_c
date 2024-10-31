#include <string.h>

int check_five(char a, char b, char c, char d, char e, char player) {
    int count = (a == player) + (b == player) + (c == player) + (d == player) + (e == player);
    int empty = (a == ' ') + (b == ' ') + (c == ' ') + (d == ' ') + (e == ' ');
    return count == 4 && empty == 1;
}

int can_win_in_one_move(char board[15][15], char player) {
    // Check rows
    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < 11; j++) {
            if (check_five(board[i][j], board[i][j+1], board[i][j+2], board[i][j+3], board[i][j+4], player)) {
                return 1;
            }
        }
    }

    // Check columns
    for (int j = 0; j < 15; j++) {
        for (int i = 0; i < 11; i++) {
            if (check_five(board[i][j], board[i+1][j], board[i+2][j], board[i+3][j], board[i+4][j], player)) {
                return 1;
            }
        }
    }

    // Check diagonals (top-left to bottom-right)
    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 11; j++) {
            if (check_five(board[i][j], board[i+1][j+1], board[i+2][j+2], board[i+3][j+3], board[i+4][j+4], player)) {
                return 1;
            }
        }
    }

    // Check diagonals (top-right to bottom-left)
    for (int i = 0; i < 11; i++) {
        for (int j = 4; j < 15; j++) {
            if (check_five(board[i][j], board[i+1][j-1], board[i+2][j-2], board[i+3][j-3], board[i+4][j-4], player)) {
                return 1;
            }
        }
    }

    return 0;
}

void get_game_state(char *buffer, char board[15][15], char player, int round){
    int can_win = can_win_in_one_move(board, player);
    if(can_win) strcpy(buffer, "end_game");
    else if(round >= 6) strcpy(buffer, "middle_game");
    else strcpy(buffer, "opening");
}
