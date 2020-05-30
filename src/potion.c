#include "roguelike.h"

int add_potions(Level* p_level) {
  int x;
  p_level->p_potions = malloc(sizeof(Potion*) * 4);
  p_level->potion_number = 0;
  for (x = 0; x < p_level->room_number; x++) {
    if ((rand() % 2) == 0) {
      p_level->p_potions[p_level->potion_number] =
          create_potion('H', rand() % 5, 1);
      set_potion_pos(p_level->p_potions[p_level->potion_number],
                     p_level->p_rooms[x]);
      p_level->potion_number++;
    }
  }
}

Potion* create_potion(char symbol, int health_value, int exist_value) {
  Potion* p_new_potion;
  p_new_potion = malloc(sizeof(Monster));
  p_new_potion->symbol = symbol;
  p_new_potion->health_value = health_value;
  p_new_potion->exist_value = exist_value;
  sprintf(p_new_potion->string, "%c", symbol);
  p_new_potion->p_position = malloc(sizeof(Position));
  return p_new_potion;
}

int set_potion_pos(Potion* p_potion, Room* p_room) {
  p_potion->p_position->x =
      (rand() % (p_room->width - 3)) + p_room->position.x + 2;
  p_potion->p_position->y =
      (rand() % (p_room->height - 3)) + p_room->position.y + 2;
}

int potion_disappear(Potion* p_potion) {
  p_potion->exist_value = 0;
  return 1;
}

void potion_draw(Potion* p_potion) {
  if (p_potion->exist_value != 0) {
    mvprintw(p_potion->p_position->y, p_potion->p_position->x,
             p_potion->string);
  }
}

Potion* get_potion_at(Position* p_position, Potion** p_potions) {
  int x;
  for (x = 0; x < MAX_MONSTERS; x++) {
    if ((p_position->y == p_potions[x]->p_position->y) &&
        (p_position->x == p_potions[x]->p_position->x))
      return p_potions[x];
  }
  return NULL;
}

int drink_potion(Player* p_player, Potion* p_potion, int order) {
  /* Player attacks monster */
  if (order == PLAYER_FIRST && p_player->health < 20) {
    p_player->health += p_potion->health_value;
    potion_disappear(p_potion);
  }
  return 1;
}
