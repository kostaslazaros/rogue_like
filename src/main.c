#include "roguelike.h"


int screen_setup(){
  initscr();
  //printw("Hello world");
  noecho();
  refresh();
  srand(time(NULL));
  return 1;
}


int main(){
  Player * player;
  int ch;
  Position * pos_new;
  Level * level;
  screen_setup(); // seting up the screen
  level = create_level(1);
  player = setup_player(); //player setup call
  /*main game loop*/
  while((ch = getch()) != 'q'){
    pos_new = handleinput(ch, player);
    pos_check(pos_new, player, level->tiles);
  }
  endwin(); // close it all down
  free(player);
  free_level(level);
  free(pos_new);
  return 0;
}
