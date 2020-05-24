#include "roguelike.h"

int room_draw(Room* p_room) {
  int x;
  int y;
  // draw the floor and the ceiling of the p_room
  for (x = p_room->position.x; x < p_room->position.x + p_room->width; x++) {
    mvprintw(p_room->position.y, x, "-");  // ceiling of p_room
    mvprintw(p_room->position.y + p_room->height - 1, x, "-");
  }
  // draw the walls
  for (y = p_room->position.y + 1; y < p_room->position.y + p_room->height - 1;
       y++) {
    mvprintw(y, p_room->position.x, "|");
    mvprintw(y, p_room->position.x + p_room->width - 1, "|");
    for (x = p_room->position.x + 1; x < p_room->position.x + p_room->width - 1;
         x++) {
      mvprintw(y, x, ".");
    }
  }

  // draw p_doors here
  mvprintw(p_room->p_doors[0]->position.y, p_room->p_doors[0]->position.x, "+");
  mvprintw(p_room->p_doors[1]->position.y, p_room->p_doors[1]->position.x, "+");
  mvprintw(p_room->p_doors[2]->position.y, p_room->p_doors[2]->position.x, "+");
  mvprintw(p_room->p_doors[3]->position.y, p_room->p_doors[3]->position.x, "+");

  return 1;
}

Room* create_room(int grid, int number_of_doors) {
  Room* new_room;
  new_room = malloc(sizeof(Room));
  new_room->number_of_doors = number_of_doors;
  switch (grid) {
    case 0:
      new_room->position.x = 0;
      new_room->position.y = 0;
      break;
    case 1:
      new_room->position.x = 27;
      new_room->position.y = 0;
      break;
    case 2:
      new_room->position.x = 54;
      new_room->position.y = 0;
      break;
    case 3:
      new_room->position.x = 0;
      new_room->position.y = 14;
      break;
    case 4:
      new_room->position.x = 27;
      new_room->position.y = 14;
      break;
    case 5:
      new_room->position.x = 54;
      new_room->position.y = 14;
      break;
  }
  new_room->height = rand() % 6 + 4;
  new_room->width = rand() % 14 + 4;

  /* offset; room can be printed within the entirety of a grid */
  new_room->position.x += rand() % (30 - new_room->width) + 1;
  new_room->position.y += rand() % (10 - new_room->height) + 1;

  new_room->p_doors = malloc(sizeof(Door*) * number_of_doors);
  for (int i = 0; i < number_of_doors; i++) {
    new_room->p_doors[i] = malloc(sizeof(Door));
    new_room->p_doors[i]->connected = 0;
  }

  // top door
  new_room->p_doors[0] = malloc(sizeof(Position));
  new_room->p_doors[0]->position.x =
      rand() % (new_room->width - 2) + new_room->position.x + 1;
  new_room->p_doors[0]->position.y = new_room->position.y;

  // left door
  new_room->p_doors[1] = malloc(sizeof(Position));
  new_room->p_doors[1]->position.y =
      rand() % (new_room->height - 2) + new_room->position.y + 1;
  new_room->p_doors[1]->position.x = new_room->position.x;

  // bottom door
  new_room->p_doors[2] = malloc(sizeof(Position));
  new_room->p_doors[2]->position.x =
      rand() % (new_room->width - 2) + new_room->position.x + 1;
  new_room->p_doors[2]->position.y =
      new_room->position.y + new_room->height - 1;

  // right door
  new_room->p_doors[3] = malloc(sizeof(Position));
  new_room->p_doors[3]->position.y =
      rand() % (new_room->height - 2) + new_room->position.y + 1;
  new_room->p_doors[3]->position.x = new_room->position.x + new_room->width - 1;
  return new_room;
}
