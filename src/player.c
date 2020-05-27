#include "roguelike.h"

Player* setup_player() {
  Player* p_new_player;
  p_new_player = malloc(sizeof(Player));
  p_new_player->p_position = malloc(sizeof(Position));
  p_new_player->health = HEALTH;  // get access to p_player's health
  p_new_player->gold = GOLD;
  p_new_player->attack = ATTACK;
  p_new_player->exp = EXPERIENCE;
  p_new_player->max_health = HEALTH;
  return p_new_player;
}

int place_player_in_room(Room** p_rooms, Player* p_player) {
  p_player->p_position->x = p_rooms[3]->position.x + 1;
  p_player->p_position->y = p_rooms[3]->position.y + 1;
}

int move_player(Position* p_pos_new, Player* p_player, char** level) {
  p_player->p_position->y = p_pos_new->y;
  p_player->p_position->x = p_pos_new->x;
}

int pos_check(Position* p_pos_new, Level* p_level) {
  Player* p_player;
  p_player = p_level->p_player;
  int space;
  switch (mvinch(p_pos_new->y, p_pos_new->x)) {
    case '.':
    case '#':
    case '+':
      move_player(p_pos_new, p_player, p_level->p_tiles);
      break;
    case 'X':
    case 'G':
    case 'T':
      combat(p_player, get_monster_at(p_pos_new, p_level->p_monsters), 1);
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

void player_print(Player* p_player) {
  mvprintw(p_player->p_position->y, p_player->p_position->x, "@");
  move(p_player->p_position->y, p_player->p_position->x);
}
