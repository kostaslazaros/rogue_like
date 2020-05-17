#include "roguelike.h"


Player * setup_player(){
  Player * new_player;
  new_player = malloc(sizeof(Player));
  new_player->position.x = 2; // get access to player's x position
  new_player->position.y = 19; // get access to player's y position
  new_player->health = 20; // get access to player's health
  new_player->coins = 0;
  mvprintw(new_player->position.y, new_player->position.x, "@");
  move(new_player->position.y, new_player->position.x);
  return new_player;
}


int move_player(Position * pos_new, Player * user, char ** level){

  char buffer[10];
  sprintf(buffer, "%c", level[user->position.y][user->position.x]);
  mvprintw(user->position.y, user->position.x, buffer);
  user->position.y = pos_new->y;
  user->position.x = pos_new->x;
  mvprintw(user->position.y, user->position.x, "@");
  move(user->position.y, user->position.x);
}


int pos_check(Position * pos_new, Player * user, char ** level){
  int space;
  switch(mvinch(pos_new->y, pos_new->x)){
    case '.':
    case '#':
    case '+':
        move_player(pos_new, user, level);
        break;
    default:
        move(user->position.y, user->position.x);
        break;
  }
}


Position * handleinput(int input, Player * user){
  Position * pos_new;
  pos_new = malloc(sizeof(Position));
  switch(input){

  // move up
    case 'w':
    case 'W':
        pos_new->y = user->position.y - 1;
        pos_new->x = user->position.x;
        break;

  // move down
    case 's':
    case 'S':
        pos_new->y = user->position.y + 1;
        pos_new->x = user->position.x;
        break;

  // move left
    case 'a':
    case 'A':
        pos_new->y = user->position.y;
        pos_new->x = user->position.x - 1;
        break;

  // move right
    case 'd':
    case 'D':
        pos_new->y = user->position.y;
        pos_new->x = user->position.x + 1;
        break;

    default:
        break;
  }
  return pos_new;
}
