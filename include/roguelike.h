#include <ncurses.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>


typedef struct Position{
  int x;
  int y;
  //
}Position;


typedef struct Room{
  Position position;
  int height;
  int width;
  Position ** doors;
  // Monster ** monsters;
  // Item ** items;
}Room;


// player structure with player's attributes
typedef struct Player{
  Position position;
  int health;
  int coins;
  // Room * room;
}Player;


int screen_setup();
Room ** map_setup();
Player * setup_player();
int handleinput(int input, Player * user);
int pos_check(int y_new, int x_new, Player * user);
int move_player(int y, int x, Player * user);


// Room functions
Room * create_room(int y, int x, int height, int width);
int room_draw(Room * room);
int door_connect(Position * door1, Position * door2);