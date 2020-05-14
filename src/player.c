#include "roguelike.h"


Player * setup_player(){
  Player * new_player;
  new_player = malloc(sizeof(Player));
  new_player->position.x = 2; // get access to player's x position
  new_player->position.y = 19; // get access to player's y position
  new_player->health = 20; // get access to player's health
  new_player->coins = 0;
  move_player(new_player->position.y, new_player->position.x, new_player);
  return new_player;
}


int move_player(int y, int x, Player * user){
  mvprintw(user->position.y, user->position.x, ".");
  user->position.y = y;
  user->position.x = x;
  mvprintw(user->position.y, user->position.x, "@");
  move(user->position.y, user->position.x);
}


int pos_check(int y_new, int x_new, Player * user){
  int space;
  switch(mvinch(y_new, x_new)){
    case '.':
    case '#':
    case '+':
        move_player(y_new, x_new, user);
        break;
    default:
        move(user->position.y, user->position.x);
        break;
  }
}


int handleinput(int input, Player * user){
  int y_new;
  int x_new;
  switch(input){

  // move up
    case 'w':
    case 'W':
        y_new = user->position.y - 1;
        x_new = user->position.x;
        break;

  // move down
    case 's':
    case 'S':
        y_new = user->position.y + 1;
        x_new = user->position.x;
        break;

  // move left
    case 'a':
    case 'A':
        y_new = user->position.y;
        x_new = user->position.x - 1;
        break;

  // move right
    case 'd':
    case 'D':
        y_new = user->position.y;
        x_new = user->position.x + 1;
        break;

    default:
        break;
  }
  pos_check(y_new, x_new, user);
}
