#include "roguelike.h"

int screen_setup() {
  initscr();
  // printw("Hello world");
  noecho();
  refresh();
  srand(time(NULL));
  return 1;
}

int game_hub_print(Level* p_level) {
  mvprintw(0, 0, "    Level: %d", p_level->level_number);
  printw("    Gold: %d", p_level->p_player->gold);
  printw("    Hp: %d(%d)", p_level->p_player->health,
         p_level->p_player->max_health);
  printw("    Attack: %d", p_level->p_player->attack);
  printw("    Exp: %d", p_level->p_player->exp);
  printw("      ");
  return 1;
}