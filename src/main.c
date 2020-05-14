#include "roguelike.h"


int screen_setup(){
  initscr();
  //printw("Hello world");
  noecho();
  refresh();
  srand(time(NULL));
  return 1;
}


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


//check what is at next position
int pos_check(int y_new, int x_new, Player * user){
  int space;
  switch(mvinch(y_new, x_new)){
    case '.':
    case '#':
    case '+':
        move_player(y_new, x_new, user);
        break;
    default:
        move(user->position.y, user->position.x);
        break;
  }
}


int move_player(int y, int x, Player * user){
  mvprintw(user->position.y, user->position.x, ".");
  user->position.y = y;
  user->position.x = x;
  mvprintw(user->position.y, user->position.x, "@");
  move(user->position.y, user->position.x);
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


int handleinput(int input, Player * user){
  int y_new;
  int x_new;
  switch(input){

  // move up
    case 'w':
    case 'W':
        y_new = user->position.y - 1;
        x_new = user->position.x;
        break;

  // move down
    case 's':
    case 'S':
        y_new = user->position.y + 1;
        x_new = user->position.x;
        break;

  // move left
    case 'a':
    case 'A':
        y_new = user->position.y;
        x_new = user->position.x - 1;
        break;

  // move right
    case 'd':
    case 'D':
        y_new = user->position.y;
        x_new = user->position.x + 1;
        break;

    default:
        break;
  }
  pos_check(y_new, x_new, user);
}


Player * setup_player(){
  Player * new_player;
  new_player = malloc(sizeof(Player));
  new_player->position.x = 2; // get access to player's x position
  new_player->position.y = 19; // get access to player's y position
  new_player->health = 20; // get access to player's health
  new_player->coins = 0;
  move_player(new_player->position.y, new_player->position.x, new_player);
  return new_player;
}


// create the map here
Room ** map_setup(){
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


int main(){
  Player * user;
  int ch;
  screen_setup(); // seting up the screen
  map_setup();    // map setup call
  user = setup_player(); //player setup call
  /*main game loop*/
  while((ch = getch()) != 'q'){
    handleinput(ch, user);
  }
  endwin(); // close it all down
  return 0;
}
