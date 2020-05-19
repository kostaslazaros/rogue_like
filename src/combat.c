#include "roguelike.h"

int combat(Player* p_player, Monster* p_monster, int order) {
  /* Player attacks monster */
  if (order == PLAYER_FIRST) {
    p_monster->health -= p_player->attack;
    if (p_monster->health > 0) {
      p_player->health -= p_monster->attack;
    } else {
      kill_monster(p_monster);
      p_player->exp++;
    }
  } else {
    p_player->health -= p_monster->attack;
    if (p_player->health > 0) {
      p_monster->health -= p_player->attack;
    }
  }
  return 1;
}