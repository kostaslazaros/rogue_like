#include "main_menu.h"
#include "roguelike.h"
#include "utils.h"

int game_loop() {
  int ch;
  Position* p_pos_new;
  Level* p_level;
  p_level = create_level(1);
  // we need this two times because of a special effect
  game_hub_print(p_level);
  /*main game loop*/
  while ((ch = getch()) != 'q') {
    game_hub_print(p_level);
    p_pos_new = handleinput(ch, p_level->p_player);
    pos_check(p_pos_new, p_level);
    monster_move(p_level);
    move(p_level->p_player->p_position->y, p_level->p_player->p_position->x);
    if (p_level->p_player->health <= 0) {
      return -1;
    }
  }
}

void menu_loop() {
  int choice;
  char* choices[] = {"Singleplayer", "Exit game"};
  while (true) {
    choice = main_menu(2, choices);
    switch (choice) {
      case SINGLEPLAYER:
        game_loop();
        clear();
        break;
      case QUIT_GAME:
        return;
        break;
    }
  }
}

int main() {
  screen_setup();  // seting up the screen
  menu_loop();
  endwin();  // close it all down
  return 0;
}
