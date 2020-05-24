#include "roguelike.h"
#include "utils.h"

Level* create_level(int level_number) {
  Level* p_new_level;
  p_new_level = malloc(sizeof(Level));
  p_new_level->level_number = level_number;
  p_new_level->room_number = MAX_NUMBER_OF_ROOMS;
  p_new_level->p_rooms = room_setup();
  connect_doors(p_new_level);
  p_new_level->p_tiles = save_pos_level();
  p_new_level->p_player = setup_player();
  place_player_in_room(p_new_level->p_rooms, p_new_level->p_player);
  add_monsters(p_new_level);
  return p_new_level;
}

// create the map here
Room** room_setup() {
  int x;
  Room** p_rooms;
  // int number_of_rooms = rand() % 4 + 2;
  int number_of_rooms = MAX_NUMBER_OF_ROOMS;
  p_rooms = malloc(sizeof(Room) * number_of_rooms);
  for (x = 0; x < number_of_rooms; x++) {
    p_rooms[x] = create_room(x, 4);
    room_draw(p_rooms[x]);
  }
  // find_path(p_rooms[0]->p_doors[3], p_rooms[1]->p_doors[1]);
  return p_rooms;
}

void connect_doors(Level* p_level) {
  int i, j;
  int random_room, random_door;
  int count;
  for (i = 0; i < p_level->room_number; i++) {
    for (j = 0; j < p_level->p_rooms[i]->number_of_doors; j++) {
      if (p_level->p_rooms[i]->p_doors[j]->connected == 1) {
        continue;
      }
      count = 0;
      while (count < 1) {
        random_room = rand() % p_level->room_number;
        random_door = rand() % p_level->p_rooms[random_room]->number_of_doors;

        if (p_level->p_rooms[random_room]->p_doors[random_door]->connected ==
                1 ||
            random_room == i) {
          count++;
          continue;
        }
        find_path(
            &p_level->p_rooms[random_room]->p_doors[random_door]->position,
            &p_level->p_rooms[i]->p_doors[j]->position);
        p_level->p_rooms[random_room]->p_doors[random_door]->connected = 1;
        p_level->p_rooms[i]->p_doors[j]->connected = 1;
        break;
      }
    }
  }
}

char** save_pos_level() {
  int x, y;
  char** p_positions;
  p_positions = malloc(sizeof(char*) * ROWS);
  for (y = 0; y < ROWS; y++) {
    p_positions[y] = malloc(sizeof(char) * COLUMNS);
    for (x = 0; x < COLUMNS; x++) {
      p_positions[y][x] = mvinch(y, x);
    }
  }
  return p_positions;
}

void free_level(Level* p_level) {
  free(p_level->p_tiles);
  free(p_level->p_rooms);
  free(p_level);
}