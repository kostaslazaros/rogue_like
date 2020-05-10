#include <ncurses.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>


typedef struct Room{
  int pos_x;
  int pos_y;
  int height;
  int width;
  // Monster ** monsters;
  // Item ** items;
}Room;



// player structure with player's attributes
typedef struct Player{
  int pos_x;
  int pos_y;
  int health;
  // Room * room;
}Player;


int screen_setup();
Room ** map_setup();
Player * setup_player();
int handleinput(int input, Player * user);
int pos_check(int y_new, int x_new, Player * user);
int move_player(int y, int x, Player * user);

// Room functions
Room * create_room(int y, int x, int height, int width);
int room_draw(Room * room);

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


int screen_setup(){
  initscr();
  //printw("Hello world");
  noecho();
  refresh();
  return 1;
}


// create the map here
Room ** map_setup(){

  Room ** rooms;
  rooms = malloc(sizeof(Room) * 6);

  rooms[0] = create_room(1, 1, 6, 8);
  room_draw(rooms[0]);


  rooms[1] = create_room(16, 20, 6, 8);
  room_draw(rooms[1]);


  rooms[2] = create_room(9, 20, 6, 12);
  room_draw(rooms[2]);

  return rooms;
}


Room * create_room(int y, int x, int height, int width){
  Room * new_room;
  new_room = malloc(sizeof(Room));
  new_room->pos_x = x;
  new_room->pos_y = y;
  new_room->height = height;
  new_room->width = width;
  return new_room;
}


int room_draw(Room * room){
  int x;
  int y;
  // draw the floor and the ceiling of the room
  for(x = room->pos_x; x < room->pos_x + room->width; x++){
    mvprintw(room->pos_y, x, "-"); // ceiling of room
    mvprintw(room->pos_y + room->height - 1, x, "-");
  }

  // draw the walls
  for(y = room->pos_y + 1; y < room->pos_y + room->height - 1; y++){
    mvprintw(y, room->pos_x, "|");
    mvprintw(y, room->pos_x + room->width - 1, "|");
    for(x = room->pos_x + 1; x < room->pos_x + room->width - 1; x++){
      mvprintw(y, x, ".");
    }
  }

  return 1;
}


Player * setup_player(){
  Player * new_player;
  new_player = malloc(sizeof(Player));
  new_player->pos_x = 2; // get access to player's x position
  new_player->pos_y = 2; // get access to player's y position
  new_player->health = 20; // get access to player's health
  move_player(new_player->pos_y, new_player->pos_x, new_player);
  return new_player;
}


int handleinput(int input, Player * user){
  int y_new;
  int x_new;
  switch(input){

  // move up
    case 'w':
    case 'W':
        y_new = user->pos_y - 1;
        x_new = user->pos_x;
        break;

  // move down
    case 's':
    case 'S':
        y_new = user->pos_y + 1;
        x_new = user->pos_x;
        break;

  // move left
    case 'a':
    case 'A':
        y_new = user->pos_y;
        x_new = user->pos_x - 1;
        break;

  // move right
    case 'd':
    case 'D':
        y_new = user->pos_y;
        x_new = user->pos_x + 1;
        break;

    default:
        break;
  }
  pos_check(y_new, x_new, user);
}


//check what is at next position
int pos_check(int y_new, int x_new, Player * user){
  int space;
  switch(mvinch(y_new, x_new)){
    case '.':
        move_player(y_new, x_new, user);
        break;
    default:
        move(user->pos_y, user->pos_x);
        break;
  }
}


int move_player(int y, int x, Player * user){
  mvprintw(user->pos_y, user->pos_x, ".");
  user->pos_y = y;
  user->pos_x = x;
  mvprintw(user->pos_y, user->pos_x, "@");
  move(user->pos_y, user->pos_x);
}