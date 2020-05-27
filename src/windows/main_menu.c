#include "main_menu.h"
#include <menu.h>
#include <stdlib.h>
#include "roguelike.h"

void close_menu(int number_of_items, MENU* menu, ITEM** items) {
  int i;
  unpost_menu(menu);  // take menu off screen
  free_menu(menu);
  for (i = 0; i < number_of_items; i++) {
    free_item(items[i]);
  }
}

int main_menu(int number_of_items, char* choices[]) {
  int i, c;
  int value;
  MENU* menu;  // ncurses menu
  ITEM** items = malloc(sizeof(**items) *
                        number_of_items);  // what goes inside the ncurses menu
  ITEM* current;

  for (i = 0; i < number_of_items; i++) {
    items[i] = new_item(choices[i],
                        "");  // create new item takes a name and a description;
  }
  items[i] = (ITEM*)NULL;
  menu = new_menu((ITEM**)items);
  post_menu(menu);
  refresh();

  while (true) {
    c = getch();
    switch (c) {
      case KEY_DOWN:
        menu_driver(menu, REQ_DOWN_ITEM);
        break;

      case KEY_UP:
        menu_driver(menu, REQ_UP_ITEM);
        break;
      case 10:  // enter key
        current = current_item(menu);
        value = item_index(current);
        close_menu(number_of_items, menu, items);
        return value;
    }
  }
}