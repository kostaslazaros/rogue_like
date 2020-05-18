#include "roguelike.h"


Level * create_level(int level_number){
    Level * new_level;
    new_level = malloc(sizeof(Level));
    new_level->level_number = level_number;
    new_level->room_number = 3;
    new_level->rooms = room_setup();
    new_level->tiles = save_pos_level();
    add_monsters(new_level);
    return new_level;
}


// create the map here
Room ** room_setup(){
  Room ** rooms;
  rooms = malloc(sizeof(Room) * 6);
  rooms[0] = create_room(18, 1, 6, 8);
  room_draw(rooms[0]);
  rooms[1] = create_room(5, 35, 6, 8);
  room_draw(rooms[1]);
  rooms[2] = create_room(15, 35, 6, 12);
  room_draw(rooms[2]);
  door_connect(rooms[0]->doors[3], rooms[2]->doors[1]);
  door_connect(rooms[1]->doors[2], rooms[0]->doors[0]);
  return rooms;
}


char ** save_pos_level(){
    int x, y;
    char ** positions;
    positions = malloc(sizeof(char *)* 25);
    for(y = 0; y < 25; y++){
        positions[y] = malloc(sizeof(char) * 100);
        for(x = 0; x < 100; x++){
            positions[y][x] = mvinch(y, x);
        }
    }
    return positions;
}


void free_level(Level * level){
    free(level->tiles);
    free(level->rooms);
    free(level);
}