#ifndef ROGUELIKE_H
#define ROGUELIKE_H


#include <ncurses.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>


typedef struct Level{
  char ** tiles;
  int level_number;
  int room_number;
  struct Room ** rooms;
  struct Monster ** monsters;
  int monster_number;
}Level;


typedef struct Position{
  int x;
  int y;
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


typedef struct Monster{
  char symbol;
  int health;
  int attack;
  int speed;
  int defense;
  int find_path;
  Position position;
}Monster;


int screen_setup();
/* level/map functions */
Room ** room_setup();
char ** save_pos_level();
Level * create_level(int);
void free_level(Level * level);


/* player functions */
Player * setup_player();
Position * handleinput(int input, Player * user);
int pos_check(Position * pos_new, Player * user, char ** level);
int move_player(Position * pos_new, Player * user, char ** level);


/* Room functions */
Room * create_room(int y, int x, int height, int width);
int room_draw(Room * room);
int door_connect(Position * door1, Position * door2);


/* Monster functions */
int add_monsters(Level * level);
Monster * select_monsters(int level_number);
Monster * create_monster(char symbol, int health, int attack, int speed, int defense, int find_path);
int set_start_pos(Monster * monster, Room * room);

#endif
