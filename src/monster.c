#include "roguelike.h"


int add_monsters(Level * level){
    int x;
    level->monsters = malloc(sizeof(Monster *) * 6);
    level->monster_number = 0;
    for(x = 0; x < level->room_number; x++){
        if((rand() % 2) == 0){
            level->monsters[level->monster_number] = select_monsters(level->level_number);
            set_start_pos( level->monsters[level->monster_number], level->rooms[x]);
            level->monster_number++;
        }
    }
}


Monster * select_monsters(int level_number){
    int monster;
    switch(level_number){
        case 1:
        case 2:
        case 3:
            monster = (rand() % 2) + 1;
            break;
        case 4:
        case 5:
            monster = (rand() % 2) + 2;
            break;
        case 6:
            monster = 3;
            break;
    }
    switch(monster){
        case 1: // spider
            return create_monster('X', 2, 1, 1, 1, 1);
        case 2: // goblin
            return create_monster('G', 5, 3, 1, 1, 2);
        case 3: // troll
            return create_monster('T', 15, 5, 1, 1, 1);
    }
}


Monster * create_monster(char symbol, int health, int attack, int speed, int defense, int find_path){
    Monster * new_monster;
    new_monster = malloc(sizeof(Monster));
    new_monster->symbol = symbol;
    new_monster->health = health;
    new_monster->attack = attack;
    new_monster->speed = speed;
    new_monster->defense = defense;
    new_monster->find_path = find_path;
    return new_monster;
}


int set_start_pos(Monster * monster, Room * room){
    char buffer[8];
    monster->position.x = (rand() % (room->width - 2)) + room->position.x + 1;
    monster->position.y = (rand() % (room->height - 2)) + room->position.y + 1;
    sprintf(buffer, "%c", monster->symbol);
    mvprintw(monster->position.y, monster->position.x, buffer);
}


/*
1 Spiders
    symbol: X
    level: 1-3
    health: 2
    attack: 1
    speed: 1
    defense: 1
    find_path: 1 (random)


2 Goblins:
    symbol: G
    level: 1-5
    health: 5
    attack: 3
    speed: 1
    defense: 1
    find_path: 2 (seeking)


3 Trolls:
    symbol: T
    level: 4-6
    health: 15
    attack: 5
    speed: 1
    defense: 1
    find_path: 1 (random)
*/