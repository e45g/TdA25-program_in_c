#ifndef TICTACTOE_H
#define TICTACTOE_H

int can_win_in_one_move(char board[15][15], char player);
void get_game_state(char *buffer, char board[15][15], int x, int o);

#endif
