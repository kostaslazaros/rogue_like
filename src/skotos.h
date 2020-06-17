#ifndef SKOTOS_H
#define SKOTOS_H

#include <ctype.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 7070
#define LEVELS 10
#define HEIGHT 24
#define WIDTH 80

// key definitions
#define UP_KEY 'W'
#define DOWN_KEY 'S'
#define LEFT_KEY 'A'
#define RIGHT_KEY 'D'
#define STAT_KEY 'Z'
#define QUIT_KEY 'Q'
#define LOGIN_KEY 'L'
#define CREATE_USER_KEY 'C'

#define FRUIT -1024
#define SPIDER 100
#define GOBLIN 200
#define BORDER -99
#define NEXT_LEVEL 24
#define PREVIOUS_LEVEL 23
#define WINNER -94

typedef struct {
  int x, y;
} coordinate;

typedef struct {
  int player_no, length, prev_state, health, attack, level;
  coordinate head;
} player;

typedef struct {
  int level_map[LEVELS][HEIGHT][WIDTH];
  player current_player;
} display_data;

#endif