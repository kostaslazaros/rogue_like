#include "roguelike.h"

Player* setup_player() {
  Player* p_new_player;
  p_new_player = malloc(sizeof(Player));
  p_new_player->p_position = malloc(sizeof(Position));
  p_new_player->p_position->x = 2;   // get access to p_player's x p_position
  p_new_player->p_position->y = 19;  // get access to p_player's y p_position
  p_new_player->health = 20;         // get access to p_player's health
  p_new_player->coins = 0;
  mvprintw(p_new_player->p_position->y, p_new_player->p_position->x, "@");
  move(p_new_player->p_position->y, p_new_player->p_position->x);
  return p_new_player;
}

int move_player(Position* p_pos_new, Player* p_player, char** level) {
  char buffer[10];
  sprintf(buffer, "%c",
          level[p_player->p_position->y][p_player->p_position->x]);
  mvprintw(p_player->p_position->y, p_player->p_position->x, buffer);
  p_player->p_position->y = p_pos_new->y;
  p_player->p_position->x = p_pos_new->x;
  mvprintw(p_player->p_position->y, p_player->p_position->x, "@");
  move(p_player->p_position->y, p_player->p_position->x);
}

int pos_check(Position* p_pos_new, Player* p_player, char** level) {
  int space;
  switch (mvinch(p_pos_new->y, p_pos_new->x)) {
    case '.':
    case '#':
    case '+':
      move_player(p_pos_new, p_player, level);
      break;
    default:
      move(p_player->p_position->y, p_player->p_position->x);
      break;
  }
}

Position* handleinput(int input, Player* p_player) {
  Position* p_pos_new;
  p_pos_new = malloc(sizeof(Position));
  switch (input) {
      // move up
    case 'w':
    case 'W':
      p_pos_new->y = p_player->p_position->y - 1;
      p_pos_new->x = p_player->p_position->x;
      break;

      // move down
    case 's':
    case 'S':
      p_pos_new->y = p_player->p_position->y + 1;
      p_pos_new->x = p_player->p_position->x;
      break;

      // move left
    case 'a':
    case 'A':
      p_pos_new->y = p_player->p_position->y;
      p_pos_new->x = p_player->p_position->x - 1;
      break;

      // move right
    case 'd':
    case 'D':
      p_pos_new->y = p_player->p_position->y;
      p_pos_new->x = p_player->p_position->x + 1;
      break;

    default:
      break;
  }
  return p_pos_new;
}
