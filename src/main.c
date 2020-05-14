#include "roguelike.h"


int screen_setup(){
  initscr();
  //printw("Hello world");
  noecho();
  refresh();
  srand(time(NULL));
  return 1;
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
