#include "roguelike.h"

int screen_setup() {
  initscr();
  noecho();
  keypad(stdscr, TRUE);  // allow access to system keys
  refresh();
  srand(time(NULL));
  return 1;
}

int game_hub_print(Level* p_level) {
  mvprintw(ROWS, 0, "    Level: %d", p_level->level_number);
  printw("    Gold: %d", p_level->p_player->gold);
  printw("    Hp: %d(%d)", p_level->p_player->health,
         p_level->p_player->max_health);
  printw("    Attack: %d", p_level->p_player->attack);
  printw("    Exp: %d", p_level->p_player->exp);
  printw("      ");
  return 1;
}