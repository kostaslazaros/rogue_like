#ifndef ROGUELIKE_H
#define ROGUELIKE_H

#include <ctype.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROWS 25
#define COLUMNS 100
#define VISIBILITY 3
#define HEALTH 20
#define ATTACK 1
#define GOLD 0
#define EXPERIENCE 0
#define MAX_MONSTERS 6
#define MAX_NUMBER_OF_ROOMS 6

/* structure definitions */

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
  struct Door** p_doors;
  int number_of_doors;
  // Monster ** p_monsters;
  // Item ** items;
} Room;

typedef struct Door {
  Position position;
  int connected;
} Door;

// player structure with player's attributes
typedef struct Player {
  Position* p_position;
  int health;
  int gold;
  int attack;
  int max_health;
  int exp;
  // Room * room;
} Player;

enum LifeState { DEAD = 0, ALIVE = 1 };
enum PathType { SEEKING = 0, RANDOM_MOVE = 1 };
enum OrderOfAttack { MONSTER_FIRST = 0, PLAYER_FIRST = 1 };

typedef struct Monster {
  char string[2];
  char symbol;
  int health;
  int attack;
  int speed;
  int defense;
  enum PathType find_path;
  enum LifeState alive;
  Position* p_position;
} Monster;

/* screen functions */
int screen_setup();
int game_hub_print(Level* p_level);

/* level/map functions */
Room** room_setup();
char** save_pos_level();
Level* create_level(int);
void free_level(Level* p_level);
void connect_doors(Level* p_level);

/* player functions */
Player* setup_player();
Position* handleinput(int input, Player* user);
int pos_check(Position* p_pos_new, Level* p_level);
int move_player(Position* p_pos_new, Player* user, char** p_level);
int place_player_in_room(Room** p_rooms, Player* p_player);

/* room functions */
Room* create_room(int grid, int number_of_doors);
int room_draw(Room* p_room);

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
