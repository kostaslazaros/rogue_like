#include "roguelike.h"

Level* create_level(int level_number) {
  Level* p_new_level;
  p_new_level = malloc(sizeof(Level));
  p_new_level->level_number = level_number;
  p_new_level->room_number = 3;
  p_new_level->p_rooms = room_setup();
  p_new_level->p_tiles = save_pos_level();
  p_new_level->p_player = setup_player();
  add_monsters(p_new_level);
  return p_new_level;
}

// create the map here
Room** room_setup() {
  Room** p_rooms;
  p_rooms = malloc(sizeof(Room) * 6);
  p_rooms[0] = create_room(18, 1, 6, 8);
  room_draw(p_rooms[0]);
  p_rooms[1] = create_room(5, 35, 6, 8);
  room_draw(p_rooms[1]);
  p_rooms[2] = create_room(15, 35, 6, 12);
  room_draw(p_rooms[2]);
  door_connect(p_rooms[0]->p_doors[3], p_rooms[2]->p_doors[1]);
  door_connect(p_rooms[1]->p_doors[2], p_rooms[0]->p_doors[0]);
  return p_rooms;
}

char** save_pos_level() {
  int x, y;
  char** p_positions;
  p_positions = malloc(sizeof(char*) * 25);
  for (y = 0; y < 25; y++) {
    p_positions[y] = malloc(sizeof(char) * 100);
    for (x = 0; x < 100; x++) {
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