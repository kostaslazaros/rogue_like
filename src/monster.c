#include "roguelike.h"

int add_monsters(Level* p_level) {
  int x;
  p_level->p_monsters = malloc(sizeof(Monster*) * 6);
  p_level->monster_number = 0;
  for (x = 0; x < p_level->room_number; x++) {
    if ((rand() % 2) == 0) {
      p_level->p_monsters[p_level->monster_number] =
          select_monsters(p_level->level_number);
      set_start_pos(p_level->p_monsters[p_level->monster_number],
                    p_level->p_rooms[x]);
      p_level->monster_number++;
    }
  }
}

Monster* select_monsters(int level_number) {
  int p_monster;
  switch (level_number) {
    case 1:
    case 2:
    case 3:
      p_monster = (rand() % 2) + 1;
      break;
    case 4:
    case 5:
      p_monster = (rand() % 2) + 2;
      break;
    case 6:
      p_monster = 3;
      break;
  }
  switch (p_monster) {
    case 1:  // spider
      return create_monster('X', 2, 1, 1, 1, 1);
    case 2:  // goblin
      return create_monster('G', 5, 3, 1, 1, 2);
    case 3:  // troll
      return create_monster('T', 15, 5, 1, 1, 1);
  }
}

Monster* create_monster(char symbol,
                        int health,
                        int attack,
                        int speed,
                        int defense,
                        int find_path) {
  Monster* p_new_monster;
  p_new_monster = malloc(sizeof(Monster));
  p_new_monster->symbol = symbol;
  p_new_monster->health = health;
  p_new_monster->attack = attack;
  p_new_monster->speed = speed;
  p_new_monster->defense = defense;
  p_new_monster->find_path = find_path;
  p_new_monster->alive = 1;
  sprintf(p_new_monster->string, "%c", symbol);
  p_new_monster->p_position = malloc(sizeof(Position));
  return p_new_monster;
}

int kill_monster(Monster* p_monster) {
  mvprintw(p_monster->p_position->y, p_monster->p_position->x, ".");
  p_monster->alive = 0;
  return 1;
}

int set_start_pos(Monster* p_monster, Room* p_room) {
  p_monster->p_position->x =
      (rand() % (p_room->width - 2)) + p_room->position.x + 1;
  p_monster->p_position->y =
      (rand() % (p_room->height - 2)) + p_room->position.y + 1;
  mvprintw(p_monster->p_position->y, p_monster->p_position->x,
           p_monster->string);
}

int monster_move(Level* p_level) {
  int x;
  for (x = 0; x < p_level->monster_number; x++) {
    if (p_level->p_monsters[x]->alive == 0)
      continue;
    mvprintw(p_level->p_monsters[x]->p_position->y,
             p_level->p_monsters[x]->p_position->x, ".");
    if (p_level->p_monsters[x]->find_path == 1) {
      pathfind_random(p_level->p_monsters[x]->p_position);
    } else {
      pathfind_seek(p_level->p_monsters[x]->p_position,
                    p_level->p_player->p_position);
    }
    mvprintw(p_level->p_monsters[x]->p_position->y,
             p_level->p_monsters[x]->p_position->x,
             p_level->p_monsters[x]->string);
  }
}

int pathfind_random(Position* p_position) {
  int random;

  random = rand() % 5;

  switch (random) {
    /* step up */
    case 0:
      if (mvinch(p_position->y - 1, p_position->x) == '.') {
        p_position->y = p_position->y - 1;
      }
      break;
    /* step down */
    case 1:
      if (mvinch(p_position->y + 1, p_position->x) == '.') {
        p_position->y = p_position->y + 1;
      }
      break;
    /* step left */
    case 2:
      if (mvinch(p_position->y, p_position->x - 1) == '.') {
        p_position->x = p_position->x - 1;
      }
      break;
    /**/
    case 3:
      if (mvinch(p_position->y, p_position->x + 1) == '.') {
        p_position->x = p_position->x + 1;
      }
      break;
    case 4:
      break;
  }
  return 1;
}

int pathfind_seek(Position* p_start, Position* p_destination) {
  /* step left*/
  if ((abs((p_start->x - 1) - p_destination->x) <
       abs(p_start->x - p_destination->x)) &&
      (mvinch(p_start->y, p_start->x - 1) == '.')) {
    p_start->x = p_start->x - 1;

    /* step right*/
  } else if ((abs((p_start->x + 1) - p_destination->x) <
              abs(p_start->x - p_destination->x)) &&
             (mvinch(p_start->y, p_start->x + 1) == '.')) {
    p_start->x = p_start->x + 1;

    /* step down */
  } else if ((abs((p_start->y + 1) - p_destination->y) <
              abs(p_start->y - p_destination->y)) &&
             (mvinch(p_start->y + 1, p_start->x) == '.')) {
    p_start->y = p_start->y + 1;

    // step up
  } else if ((abs((p_start->y - 1) - p_destination->y) <
              abs(p_start->y - p_destination->y)) &&
             (mvinch(p_start->y - 1, p_start->x) == '.')) {
    p_start->y = p_start->y - 1;
  } else {
    /* do nothing at all */
  }
  return 1;
}

Monster* get_monster_at(Position* p_position, Monster** p_monsters) {
  int x;
  for (x = 0; x < 6; x++) {
    if ((p_position->y == p_monsters[x]->p_position->y) &&
        (p_position->x == p_monsters[x]->p_position->x))
      return p_monsters[x];
  }
  return NULL;
}