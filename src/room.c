#include "roguelike.h"


int room_draw(Room * room){
  int x;
  int y;
  // draw the floor and the ceiling of the room
  for(x = room->position.x; x < room->position.x + room->width; x++){
    mvprintw(room->position.y, x, "-"); // ceiling of room
    mvprintw(room->position.y + room->height - 1, x, "-");
  }
  // draw the walls
  for(y = room->position.y + 1; y < room->position.y + room->height - 1; y++){
    mvprintw(y, room->position.x, "|");
    mvprintw(y, room->position.x + room->width - 1, "|");
    for(x = room->position.x + 1; x < room->position.x + room->width - 1; x++){
      mvprintw(y, x, ".");
    }
  }

  // draw doors here
  mvprintw(room->doors[0]->y, room->doors[0]->x, "+");
  mvprintw(room->doors[1]->y, room->doors[1]->x, "+");
  mvprintw(room->doors[2]->y, room->doors[2]->x, "+");
  mvprintw(room->doors[3]->y, room->doors[3]->x, "+");

  return 1;
}


/*The idea of the door connection function is taking a random step in a direction. Is that step going to
take us closer to the destination door we want? If so, is that step happening in blank space? If so,
put the # sympol to start creation of a hallway.*/
int door_connect(Position * door1, Position * door2){
  Position temp;
  Position previous;
  int count = 0;
  temp.x = door1->x;
  temp.y = door1->y;
  previous = temp;
  while(1){

    /* step left*/
    if((abs((temp.x - 1) - door2->x) < abs(temp.x - door2->x)) && (mvinch(temp.y, temp.x - 1) == ' ')){
      previous.x = temp.x;
      temp.x = temp.x - 1;

      /* step right*/
    }else if((abs((temp.x + 1) - door2->x) < abs(temp.x - door2->x)) && (mvinch(temp.y, temp.x + 1) == ' ')){
      previous.x = temp.x;
      temp.x = temp.x + 1;

    /* step down */
    }else if((abs((temp.y + 1) - door2->y) < abs(temp.y - door2->y)) && (mvinch(temp.y + 1, temp.x) == ' ')){
      previous.y = temp.y;
      temp.y = temp.y + 1;

    // step up
    }else if((abs((temp.y - 1) - door2->y) < abs(temp.y - door2->y)) && (mvinch(temp.y - 1, temp.x) == ' ')){
      previous.y = temp.y;
      temp.y = temp.y - 1;
    }else{
      if(count == 0){
        temp = previous;
        count++;
        continue;
      }
      else{
        return 0;
    }
  }
    mvprintw(temp.y, temp.x, "#");
    getch();
}

  return 1;
}


Room * create_room(int y, int x, int height, int width){
  Room * new_room;
  new_room = malloc(sizeof(Room));
  new_room->position.x = x;
  new_room->position.y = y;
  new_room->height = height;
  new_room->width = width;

  new_room->doors = malloc(sizeof(Position) * 4);

  // top door
  new_room->doors[0] = malloc(sizeof(Position));
  new_room->doors[0]->x = rand() % (width - 2) + new_room->position.x + 1;
  new_room->doors[0]->y = new_room->position.y;

  // left door
  new_room->doors[1] = malloc(sizeof(Position));
  new_room->doors[1]->y = rand() % (height - 2) + new_room->position.y + 1;
  new_room->doors[1]->x = new_room->position.x;

  // bottom door
  new_room->doors[2] = malloc(sizeof(Position));
  new_room->doors[2]->x = rand() % (width - 2) + new_room->position.x + 1;
  new_room->doors[2]->y = new_room->position.y + new_room->height - 1;

  // right door
  new_room->doors[3] = malloc(sizeof(Position));
  new_room->doors[3]->y = rand() % (height - 2) + new_room->position.y + 1;
  new_room->doors[3]->x = new_room->position.x + width - 1;
  return new_room;
}
