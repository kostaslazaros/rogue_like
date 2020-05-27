#include "main_menu.h"
#include "roguelike.h"
#include "utils.h"

void menu_loop() {
  int choice;
  char* choices[] = {"Singleplayer", "Exit game"};
  Game game;
  game.current_level = 0;
  while (true) {
    choice = main_menu(2, choices);
    switch (choice) {
      case SINGLEPLAYER:
        game_loop(&game);
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
