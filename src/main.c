#include "roguelike.h"

int screen_setup() {
  initscr();
  // printw("Hello world");
  noecho();
  refresh();
  srand(time(NULL));
  return 1;
}

int main() {
  int ch;
  Position* p_pos_new;
  Level* p_level;
  screen_setup();  // seting up the screen
  p_level = create_level(1);
  /*main game loop*/
  while ((ch = getch()) != 'q') {
    p_pos_new = handleinput(ch, p_level->p_player);
    pos_check(p_pos_new, p_level);
    monster_move(p_level);
    move(p_level->p_player->p_position->y, p_level->p_player->p_position->x);
  }
  endwin();  // close it all down
  free(p_level->p_player);
  free_level(p_level);
  free(p_pos_new);
  return 0;
}
