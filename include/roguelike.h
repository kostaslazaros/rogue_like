#ifndef ROGUELIKE_H
#define ROGUELIKE_H

#include <ctype.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Level {
  char** p_tiles;
  int level_number;
  int room_number;
  struct Room** p_rooms;
  struct Monster** p_monsters;
  struct Player* p_player;
  int monster_number;
} Level;

typedef struct Position {
  int x;
  int y;
} Position;

typedef struct Room {
  Position position;
  int height;
  int width;
  Position** p_doors;
  // Monster ** p_monsters;
  // Item ** items;
} Room;

// player structure with player's attributes
typedef struct Player {
  Position* p_position;
  int health;
  int coins;
  int attack;
  // Room * room;
} Player;

typedef struct Monster {
  char string[2];
  char symbol;
  int health;
  int attack;
  int speed;
  int defense;
  int find_path;
  int alive;
  Position* p_position;
} Monster;

int screen_setup();
/* level/map functions */
Room** room_setup();
char** save_pos_level();
Level* create_level(int);
void free_level(Level* p_level);

/* player functions */
Player* setup_player();
Position* handleinput(int input, Player* user);
int pos_check(Position* p_pos_new, Level* p_level);
int move_player(Position* p_pos_new, Player* user, char** p_level);

/* room functions */
Room* create_room(int y, int x, int height, int width);
int room_draw(Room* p_room);
int door_connect(Position* p_door1, Position* p_door2);

/* monster functions */
int add_monsters(Level* level);
Monster* select_monsters(int level_number);
Monster* create_monster(char symbol,
                        int health,
                        int attack,
                        int speed,
                        int defense,
                        int find_path);
int set_start_pos(Monster* p_monster, Room* p_room);
int monster_move(Level* p_level);
int pathfind_seek(Position* p_start, Position* p_destination);
int pathfind_random(Position* position);
Monster* get_monster_at(Position* p_position, Monster** p_monsters);
/* combat functions */
int combat(Player* p_player, Monster* p_monster, int order);
int kill_monster(Monster* p_monster);

#endif
