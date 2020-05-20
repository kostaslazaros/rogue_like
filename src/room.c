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
  mvprintw(p_room->p_doors[0]->y, p_room->p_doors[0]->x, "+");
  mvprintw(p_room->p_doors[1]->y, p_room->p_doors[1]->x, "+");
  mvprintw(p_room->p_doors[2]->y, p_room->p_doors[2]->x, "+");
  mvprintw(p_room->p_doors[3]->y, p_room->p_doors[3]->x, "+");

  return 1;
}

/*The idea of the door connection function is taking a random step in a
direction. Is that step going to take us closer to the destination door we want?
If so, is that step happening in blank space? If so, put the # sympol to start
creation of a hallway.*/
int door_connect(Position* p_door1, Position* p_door2) {
  Position temp;
  Position previous;
  int count = 0;
  temp.x = p_door1->x;
  temp.y = p_door1->y;
  previous = temp;
  while (1) {
    /* step left*/
    if ((abs((temp.x - 1) - p_door2->x) < abs(temp.x - p_door2->x)) &&
        (mvinch(temp.y, temp.x - 1) == ' ')) {
      previous.x = temp.x;
      temp.x = temp.x - 1;

      /* step right*/
    } else if ((abs((temp.x + 1) - p_door2->x) < abs(temp.x - p_door2->x)) &&
               (mvinch(temp.y, temp.x + 1) == ' ')) {
      previous.x = temp.x;
      temp.x = temp.x + 1;

      /* step down */
    } else if ((abs((temp.y + 1) - p_door2->y) < abs(temp.y - p_door2->y)) &&
               (mvinch(temp.y + 1, temp.x) == ' ')) {
      previous.y = temp.y;
      temp.y = temp.y + 1;

      // step up
    } else if ((abs((temp.y - 1) - p_door2->y) < abs(temp.y - p_door2->y)) &&
               (mvinch(temp.y - 1, temp.x) == ' ')) {
      previous.y = temp.y;
      temp.y = temp.y - 1;
    } else {
      if (count == 0) {
        temp = previous;
        count++;
        continue;
      } else {
        return 0;
      }
    }
    mvprintw(temp.y, temp.x, "#");
    // getch();
  }

  return 1;
}

Room* create_room(int grid) {
  Room* new_room;
  new_room = malloc(sizeof(Room));
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

  new_room->position.x += rand() % (29 - new_room->width + 1);
  new_room->position.y += rand() % (9 - new_room->height + 1);

  new_room->p_doors = malloc(sizeof(Position) * 4);

  // top door
  new_room->p_doors[0] = malloc(sizeof(Position));
  new_room->p_doors[0]->x =
      rand() % (new_room->width - 2) + new_room->position.x + 1;
  new_room->p_doors[0]->y = new_room->position.y;

  // left door
  new_room->p_doors[1] = malloc(sizeof(Position));
  new_room->p_doors[1]->y =
      rand() % (new_room->height - 2) + new_room->position.y + 1;
  new_room->p_doors[1]->x = new_room->position.x;

  // bottom door
  new_room->p_doors[2] = malloc(sizeof(Position));
  new_room->p_doors[2]->x =
      rand() % (new_room->width - 2) + new_room->position.x + 1;
  new_room->p_doors[2]->y = new_room->position.y + new_room->height - 1;

  // right door
  new_room->p_doors[3] = malloc(sizeof(Position));
  new_room->p_doors[3]->y =
      rand() % (new_room->height - 2) + new_room->position.y + 1;
  new_room->p_doors[3]->x = new_room->position.x + new_room->width - 1;
  return new_room;
}
