/*
 * Multiplayer rogue-like game
 */

#include <signal.h>
#include "skotos.h"
#include "users.h"

#define MAX_PLAYERS 1024
#define WINNER_LENGTH 15

// Game map
int game_map[LEVELS][HEIGHT][WIDTH];
int map_size = LEVELS * HEIGHT * WIDTH * sizeof(game_map[0][0][0]);
int client_map_size = HEIGHT * WIDTH * sizeof(game_map[0][0][0]);
pthread_mutex_t map_lock = PTHREAD_MUTEX_INITIALIZER;
int someone_won = 0;

// Direction key types
typedef enum {
  UP = UP_KEY,
  DOWN = DOWN_KEY,
  LEFT = LEFT_KEY,
  RIGHT = RIGHT_KEY,
  STATIC = STAT_KEY
} direction;

// Coordinate structure, these are the building blocks for snakes
typedef struct {
  int x, y;
} coordinate;

// Snake structure, each part is made up of coordinate
typedef struct {
  int player_no, length, prev_state, health, attack, level;
  coordinate head;
} player;

// Function to create a player
player* make_player(int player_no, int level, int head_y, int head_x) {
  // Place the player on the map (matrix)
  pthread_mutex_lock(&map_lock);
  game_map[level][head_y][head_x] = -player_no;
  pthread_mutex_unlock(&map_lock);

  // Create player struct, set coordinates facing up
  player* p_player = malloc(sizeof(player));

  p_player->player_no = player_no;
  p_player->level = level;
  p_player->length = 3;
  p_player->prev_state = 0;
  p_player->health = 20;
  p_player->attack = 3;

  p_player->head.y = head_y;
  p_player->head.x = head_x;

  return p_player;
}

// Function to kill player and free memory
void kill_player(player* p_player) {
  // Set all player coordinates to zero on map
  pthread_mutex_lock(&map_lock);
  game_map[p_player->level][p_player->head.y][p_player->head.x] = 0;
  pthread_mutex_unlock(&map_lock);

  // Free memory
  free(p_player);
  p_player = NULL;
}

// Function for a player to eat a fruit in front of it
void eat_fruit(player* p_player, direction d) {
  switch (d) {
    case UP: {
      p_player->head.y = p_player->head.y - 1;
      if (game_map[p_player->level][p_player->head.y][p_player->head.x + 1] ==
          FRUIT) {
        pthread_mutex_lock(&map_lock);
        game_map[p_player->level][p_player->head.y][p_player->head.x + 1] = 0;
        pthread_mutex_unlock(&map_lock);
      }
      break;
    }
    case DOWN: {
      p_player->head.y = p_player->head.y + 1;
      if (game_map[p_player->level][p_player->head.y][p_player->head.x + 1] ==
          FRUIT) {
        pthread_mutex_lock(&map_lock);
        game_map[p_player->level][p_player->head.y][p_player->head.x + 1] = 0;
        pthread_mutex_unlock(&map_lock);
      }
      break;
    }
    case LEFT: {
      p_player->head.x = p_player->head.x - 1;
      break;
    }
    case RIGHT: {
      p_player->head.x = p_player->head.x + 1;
      break;
    }
    default:
      break;
  }

  pthread_mutex_lock(&map_lock);
  game_map[p_player->level][p_player->head.y][p_player->head.x] = 0;
  pthread_mutex_unlock(&map_lock);
  p_player->health++;
}

int collision_detection(int next_y,
                        int next_x,
                        player* p_player,
                        direction dir) {
  // printf("Value of health %d\n", p_player->health);
  int next_move_val = game_map[p_player->level][next_y][next_x];
  if (next_move_val == NEXT_LEVEL) {
    p_player->level += 1;
    return 2;
  }
  if (next_move_val == FRUIT && p_player->health < 20) {
    eat_fruit(p_player, dir);
    if (p_player->health > 20) {
      p_player->health = 20;
    }
  }
  if (next_move_val == PREVIOUS_LEVEL) {
    p_player->level -= 1;
    return 2;
  }
  if (next_move_val == BORDER)
    return 1;
  else if (next_move_val < 0 && next_move_val > -7)
    return 1;
  else if ((next_move_val > GOBLIN) && (next_move_val < GOBLIN + 100)) {
    int health = next_move_val % 10;
    int rest_attr = next_move_val - health;
    int new_health = (next_move_val % 10) - p_player->attack;
    if (new_health < 0)
      new_health = 0;
    int monster_attack = (rest_attr / 10) % 10;
    p_player->health -= monster_attack;
    pthread_mutex_lock(&map_lock);
    if (new_health == 0) {
      game_map[p_player->level][next_y][next_x] = 0;
    } else {
      game_map[p_player->level][next_y][next_x] = rest_attr + new_health;
    }
    pthread_mutex_unlock(&map_lock);

    if (new_health > 0) {
      return 1;
    }
  }
  return 0;
}

// Function to move player
void move_player(player* p_player, direction d) {
  int new_x, new_y;

  int old_x = p_player->head.x;
  int old_y = p_player->head.y;
  int old_level = p_player->level;
  int prev_state = p_player->prev_state;

  if (d == STATIC)
    return;

  switch (d) {
    case UP: {
      new_x = old_x;
      new_y = old_y - 1;
      break;
    }
    case DOWN: {
      new_x = old_x;
      new_y = old_y + 1;
      break;
    }
    case LEFT: {
      new_x = old_x - 1;
      new_y = old_y;
      break;
    }
    case RIGHT: {
      new_x = old_x + 1;
      new_y = old_y;
      break;
    }
    case STATIC: {
      new_x = old_x;
      new_y = old_y;
      break;
    }
    default:
      break;
  }

  int col_det = collision_detection(new_y, new_x, p_player, d);
  if (col_det == 1) {
    return;
  } else if (col_det == 2) {
    pthread_mutex_lock(&map_lock);
    game_map[old_level][old_y][old_x] = 0;
    pthread_mutex_unlock(&map_lock);
  }
  p_player->prev_state = game_map[p_player->level][new_y][new_x];
  p_player->head.x = new_x;
  p_player->head.y = new_y;

  pthread_mutex_lock(&map_lock);
  game_map[p_player->level][p_player->head.y][p_player->head.x] =
      -(p_player->player_no);
  game_map[p_player->level][old_y][old_x] = prev_state;
  pthread_mutex_unlock(&map_lock);
}

// Function to randomly add a fruit to the game map
void add_fruit(int lvl) {
  int x, y;
  do {
    y = rand() % (HEIGHT - 6) + 3;
    x = rand() % (WIDTH - 6) + 3;
  } while (game_map[lvl][y][x] != 0);
  pthread_mutex_lock(&map_lock);
  game_map[lvl][y][x] = FRUIT;
  pthread_mutex_unlock(&map_lock);
}

void add_randomly_fruits(int fruit_number) {
  srand(time(NULL));
  for (int lvl = 0; lvl < LEVELS; lvl++) {
    for (int i = 0; i < fruit_number; i++)
      add_fruit(lvl);
  }
}

void add_monster(int lvl, int monster_num) {
  int x, y;
  do {
    y = rand() % (HEIGHT - 6) + 3;
    x = rand() % (WIDTH - 6) + 3;
  } while (game_map[lvl][y][x] != 0);
  pthread_mutex_lock(&map_lock);
  game_map[lvl][y][x] = monster_num;
  pthread_mutex_unlock(&map_lock);
}

void add_randomly_monsters(int number_of_monsters, int monster_num) {
  srand(time(NULL));
  for (int lvl = 0; lvl < LEVELS; lvl++) {
    for (int i = 0; i < number_of_monsters; i++)
      add_monster(lvl, monster_num);
  }
}

void add_fruit_directly(int level, int pos_x, int pos_y) {
  pthread_mutex_lock(&map_lock);
  game_map[level][pos_y][pos_x] = FRUIT;
  pthread_mutex_unlock(&map_lock);
}

// Stevens, chapter 12, page 428: Create detatched thread
int make_thread(void* (*fn)(void*), void* arg) {
  int err;
  pthread_t tid;
  pthread_attr_t attr;

  err = pthread_attr_init(&attr);
  if (err != 0)
    return err;
  err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  if (err == 0)
    err = pthread_create(&tid, &attr, fn, arg);
  pthread_attr_destroy(&attr);
  return err;
}

// Output error message and exit cleanly
void error(const char* msg) {
  perror(msg);
  fflush(stdout);
  exit(1);
}

// Handle ctrl+c signal
void ctrl_c_handler() {
  printf("\nSignal control\n");
  exit(0);
}

// game border saved in game_map
void set_game_borders() {
  int i, l;
  for (l = 0; l < LEVELS; l++)
    for (i = 0; i < HEIGHT; i++)
      game_map[l][i][0] = game_map[l][i][WIDTH - 1] = BORDER;
  for (l = 0; l < LEVELS; l++)
    for (i = 0; i < WIDTH; i++)
      game_map[l][0][i] = game_map[l][HEIGHT - 1][i] = BORDER;
}

void set_level_walls(int level, int x1, int y1, int x2, int y2) {
  int i, l;
  for (l = x1; l <= x2; l++)
    for (i = y1; i <= y2; i++)
      game_map[level][l][i] = BORDER;
}

// game logic is here
void* gameplay(void* arg) {
  int fd = *(int*)arg;
  int player_no = fd - 3;

  printf("Connected player: %d!\n", player_no);

  int head_y, head_x;
  srand(time(NULL));
  do {
    head_y = rand() % (HEIGHT - 6) + 3;
    head_x = rand() % (WIDTH - 6) + 3;
  } while (!(game_map[0][head_y][head_x] == 0));

  player* p_player = make_player(player_no, 0, head_y, head_x);

  // User input
  char key = STATIC;
  char key_buffer = STATIC;
  char map_buffer[map_size];
  int bytes_sent, n;
  int success = 1;

  while (success) {
    // Check if someone won
    if (someone_won)
      success = 0;

    // Check if you are the winner
    if (p_player->length >= 15) {
      someone_won = player_no;
      pthread_mutex_lock(&map_lock);
      game_map[0][0][0] = WINNER;
      pthread_mutex_unlock(&map_lock);
    } else if (game_map[0][0][0] != BORDER) {
      pthread_mutex_lock(&map_lock);
      game_map[0][0][0] = someone_won;
      pthread_mutex_unlock(&map_lock);
    } else if (p_player->health <= 0) {
      success = 0;
    }

    // Copy map to buffer, and send to client
    memcpy(map_buffer, game_map[p_player->level], client_map_size);
    bytes_sent = 0;
    while (bytes_sent < client_map_size) {
      bytes_sent += write(fd, game_map[p_player->level], client_map_size);
      if (bytes_sent < 0)
        error("ERROR writing to socket");
    }

    // Player key input
    bzero(&key_buffer, 1);
    n = read(fd, &key_buffer, 1);
    if (n <= 0)
      break;

    // If user key is a direction, then apply it
    key_buffer = toupper(key_buffer);
    if ((key_buffer == UP) || (key_buffer == DOWN) || (key_buffer == LEFT) ||
        (key_buffer == RIGHT) || (key_buffer == STATIC))
      key = key_buffer;

    switch (key) {
      case UP: {
        move_player(p_player, UP);
        break;
      }

      case DOWN: {
        move_player(p_player, DOWN);
        break;
      }

      case LEFT: {
        move_player(p_player, LEFT);
        break;
      }

      case RIGHT: {
        move_player(p_player, RIGHT);
        break;
      }

      case STATIC: {
        // printf("Testing static");
        break;
      }

      default:
        break;
    }
  }

  if (p_player->length == WINNER_LENGTH) {
    fprintf(stderr, "Player %d won!\n", player_no);
    kill_player(p_player);
    close(fd);
    return 0;
  }

  else {
    fprintf(stderr, "Player %d lost.\n", player_no);
    kill_player(p_player);
    close(fd);
    return 0;
  }

  if (p_player->health <= 0) {
    fprintf(stderr, "Player %d died.\n", player_no);
    kill_player(p_player);
    close(fd);
    return 0;
  }
}

// Main function
int main() {
  int socket_fds[MAX_PLAYERS];
  struct sockaddr_in socket_addr[MAX_PLAYERS];
  int i;
  user_list active_users = new_userlist_from_file();

  for (int gh = 0; gh < active_users.size; gh++)
    printf("%s \t %s \t %d \t %d \n", active_users.udata[gh].name,
           active_users.udata[gh].passwd, active_users.udata[gh].won_games,
           active_users.udata[gh].lost_games);

  // Handle Ctrl+C
  signal(SIGINT, ctrl_c_handler);

  // Fill gamestate matrix with zeros
  memset(game_map, 0, map_size);

  set_game_borders();

  /* Level 0 */
  // set_level_walls(0, 10, 2, 10, 2);
  // set_level_walls(0, 6, 0, 6, 5);
  // set_level_walls(0, 3, 6, 6, 6);
  // set_level_walls(0, 10, 40, 10, 70);
  // set_level_walls(0, 15, 30, 24, 30);
  // set_level_walls(0, 3, 7, 3, 31);
  set_level_walls(0, 0, 0, 5, 79);
  set_level_walls(0, 6, 0, 24, 21);
  set_level_walls(0, 16, 22, 24, 34);
  set_level_walls(0, 6, 35, 8, 41);
  set_level_walls(0, 11, 35, 24, 41);
  set_level_walls(0, 21, 42, 24, 79);
  set_level_walls(0, 6, 65, 20, 79);
  set_level_walls(0, 13, 51, 13, 64);
  set_level_walls(0, 14, 51, 18, 51);

  /* Level 1 */
  game_map[1][1][2] = BORDER;
  game_map[1][2][2] = BORDER;
  game_map[1][3][2] = BORDER;
  game_map[1][4][2] = BORDER;
  game_map[1][5][2] = BORDER;
  game_map[1][6][2] = BORDER;
  game_map[1][7][2] = BORDER;
  game_map[1][8][2] = BORDER;
  game_map[1][9][2] = BORDER;
  game_map[1][10][2] = BORDER;
  game_map[1][1][3] = BORDER;
  game_map[1][1][4] = BORDER;
  game_map[1][1][5] = BORDER;
  game_map[1][1][6] = BORDER;
  game_map[1][1][7] = BORDER;
  game_map[1][1][8] = BORDER;
  game_map[1][1][9] = BORDER;
  game_map[1][1][10] = BORDER;
  game_map[1][1][11] = BORDER;
  game_map[1][1][12] = BORDER;
  game_map[1][1][13] = BORDER;
  game_map[1][2][13] = BORDER;
  game_map[1][23][13] = BORDER;
  game_map[1][2][13] = BORDER;
  game_map[1][3][13] = BORDER;
  game_map[1][4][13] = BORDER;
  game_map[1][5][13] = BORDER;
  game_map[1][6][13] = BORDER;
  game_map[1][7][13] = BORDER;
  game_map[1][8][13] = BORDER;
  game_map[1][9][13] = BORDER;
  game_map[1][10][13] = BORDER;
  game_map[1][11][13] = BORDER;
  game_map[1][12][13] = BORDER;
  game_map[1][13][13] = BORDER;
  game_map[1][12][5] = BORDER;
  game_map[1][11][5] = BORDER;
  game_map[1][10][5] = BORDER;
  game_map[1][3][2] = BORDER;
  game_map[1][3][6] = BORDER;
  game_map[1][3][7] = BORDER;
  game_map[1][3][8] = BORDER;
  game_map[1][3][9] = BORDER;
  game_map[1][9][6] = BORDER;
  game_map[1][9][7] = BORDER;
  game_map[1][9][8] = BORDER;
  game_map[1][9][9] = BORDER;
  game_map[1][11][9] = BORDER;
  game_map[1][11][8] = BORDER;
  game_map[1][11][7] = BORDER;
  game_map[1][11][6] = BORDER;
  game_map[1][12][6] = BORDER;
  game_map[1][13][6] = BORDER;
  game_map[1][14][6] = BORDER;
  game_map[1][15][6] = BORDER;
  game_map[1][16][6] = BORDER;
  game_map[1][17][6] = BORDER;
  game_map[1][18][6] = BORDER;
  game_map[1][19][6] = BORDER;
  game_map[1][20][6] = BORDER;
  game_map[1][21][6] = BORDER;
  game_map[1][22][6] = BORDER;
  game_map[1][23][6] = BORDER;
  game_map[1][24][6] = BORDER;
  game_map[1][11][13] = BORDER;
  game_map[1][11][16] = BORDER;
  game_map[1][11][17] = BORDER;
  game_map[1][10][17] = BORDER;
  game_map[1][9][17] = BORDER;
  game_map[1][8][17] = BORDER;
  game_map[1][8][16] = BORDER;
  game_map[1][7][16] = BORDER;
  game_map[1][6][16] = BORDER;
  game_map[1][5][16] = BORDER;
  game_map[1][4][16] = BORDER;
  game_map[1][3][16] = BORDER;
  game_map[1][2][16] = BORDER;
  game_map[1][1][16] = BORDER;
  game_map[1][8][19] = BORDER;
  game_map[1][9][19] = BORDER;
  game_map[1][10][19] = BORDER;
  game_map[1][11][19] = BORDER;
  game_map[1][11][20] = BORDER;
  game_map[1][11][21] = BORDER;
  game_map[1][11][22] = BORDER;
  game_map[1][15][22] = BORDER;
  game_map[1][16][22] = BORDER;
  game_map[1][17][22] = BORDER;
  game_map[1][18][22] = BORDER;
  game_map[1][19][22] = BORDER;
  game_map[1][20][22] = BORDER;
  game_map[1][21][22] = BORDER;
  game_map[1][22][22] = BORDER;
  game_map[1][23][22] = BORDER;
  game_map[1][24][22] = BORDER;
  game_map[1][12][22] = BORDER;
  game_map[1][15][22] = BORDER;
  game_map[1][16][22] = BORDER;
  game_map[1][17][22] = BORDER;
  game_map[1][19][22] = BORDER;
  game_map[1][20][22] = BORDER;
  game_map[1][21][22] = BORDER;
  game_map[1][22][22] = BORDER;
  game_map[1][23][22] = BORDER;
  game_map[1][24][22] = BORDER;
  game_map[1][12][23] = BORDER;
  game_map[1][12][25] = BORDER;
  game_map[1][12][26] = BORDER;
  game_map[1][12][27] = BORDER;
  game_map[1][12][28] = BORDER;
  game_map[1][11][28] = BORDER;
  game_map[1][16][28] = BORDER;
  game_map[1][17][28] = BORDER;
  game_map[1][19][28] = BORDER;
  game_map[1][20][28] = BORDER;
  game_map[1][21][28] = BORDER;
  game_map[1][22][28] = BORDER;
  game_map[1][23][28] = BORDER;
  game_map[1][24][28] = BORDER;
  game_map[1][15][23] = BORDER;
  game_map[1][15][24] = BORDER;
  game_map[1][15][25] = BORDER;
  game_map[1][15][26] = BORDER;
  game_map[1][15][27] = BORDER;
  game_map[1][15][28] = BORDER;
  game_map[1][11][29] = BORDER;
  game_map[1][10][29] = BORDER;
  game_map[1][9][29] = BORDER;
  game_map[1][8][29] = BORDER;
  game_map[1][7][29] = BORDER;
  game_map[1][7][28] = BORDER;
  game_map[1][7][27] = BORDER;
  game_map[1][7][26] = BORDER;
  game_map[1][6][26] = BORDER;
  game_map[1][5][26] = BORDER;
  game_map[1][4][26] = BORDER;
  game_map[1][3][26] = BORDER;
  game_map[1][2][26] = BORDER;
  game_map[1][1][26] = BORDER;
  game_map[1][11][32] = BORDER;
  game_map[1][11][33] = BORDER;
  game_map[1][11][34] = BORDER;
  game_map[1][11][35] = BORDER;
  game_map[1][11][36] = BORDER;
  game_map[1][11][37] = BORDER;
  game_map[1][11][38] = BORDER;
  game_map[1][12][38] = BORDER;
  game_map[1][16][38] = BORDER;
  game_map[1][17][38] = BORDER;
  game_map[1][18][38] = BORDER;
  game_map[1][19][38] = BORDER;
  game_map[1][20][38] = BORDER;
  game_map[1][21][38] = BORDER;
  game_map[1][22][38] = BORDER;
  game_map[1][23][38] = BORDER;
  game_map[1][24][38] = BORDER;
  game_map[1][12][38] = BORDER;
  game_map[1][12][39] = BORDER;
  game_map[1][12][40] = BORDER;
  game_map[1][12][41] = BORDER;
  game_map[1][12][42] = BORDER;
  game_map[1][12][43] = BORDER;
  game_map[1][12][44] = BORDER;
  game_map[1][15][39] = BORDER;
  game_map[1][15][40] = BORDER;
  game_map[1][15][41] = BORDER;
  game_map[1][15][42] = BORDER;
  game_map[1][15][43] = BORDER;
  game_map[1][15][44] = BORDER;
  game_map[1][10][44] = BORDER;
  game_map[1][11][44] = BORDER;
  game_map[1][16][44] = BORDER;
  game_map[1][17][44] = BORDER;
  game_map[1][18][44] = BORDER;
  game_map[1][20][44] = BORDER;
  game_map[1][21][44] = BORDER;
  game_map[1][22][44] = BORDER;
  game_map[1][23][44] = BORDER;
  game_map[1][24][44] = BORDER;
  game_map[1][10][50] = BORDER;
  game_map[1][9][50] = BORDER;
  game_map[1][8][50] = BORDER;
  game_map[1][7][50] = BORDER;
  game_map[1][10][51] = BORDER;
  game_map[1][10][52] = BORDER;
  game_map[1][10][53] = BORDER;
  game_map[1][10][54] = BORDER;
  game_map[1][10][55] = BORDER;
  game_map[1][10][56] = BORDER;
  game_map[1][11][56] = BORDER;
  game_map[1][13][56] = BORDER;
  game_map[1][14][56] = BORDER;
  game_map[1][15][56] = BORDER;
  game_map[1][15][57] = BORDER;
  game_map[1][15][58] = BORDER;
  game_map[1][15][59] = BORDER;
  game_map[1][15][60] = BORDER;
  game_map[1][15][61] = BORDER;
  game_map[1][15][62] = BORDER;
  game_map[1][20][56] = BORDER;
  game_map[1][20][57] = BORDER;
  game_map[1][20][58] = BORDER;
  game_map[1][20][59] = BORDER;
  game_map[1][20][60] = BORDER;
  game_map[1][20][61] = BORDER;
  game_map[1][20][62] = BORDER;
  game_map[1][21][62] = BORDER;
  game_map[1][22][62] = BORDER;
  game_map[1][23][62] = BORDER;
  game_map[1][24][62] = BORDER;
  game_map[1][14][62] = BORDER;
  game_map[1][13][62] = BORDER;
  game_map[1][13][63] = BORDER;
  game_map[1][13][64] = BORDER;
  game_map[1][13][65] = BORDER;
  game_map[1][13][66] = BORDER;
  game_map[1][13][67] = BORDER;
  game_map[1][13][68] = BORDER;
  game_map[1][13][69] = BORDER;
  game_map[1][13][70] = BORDER;
  game_map[1][13][71] = BORDER;
  game_map[1][13][72] = BORDER;
  game_map[1][13][73] = BORDER;
  game_map[1][12][73] = BORDER;
  game_map[1][11][73] = BORDER;
  game_map[1][10][73] = BORDER;
  game_map[1][9][73] = BORDER;
  game_map[1][9][72] = BORDER;
  game_map[1][9][71] = BORDER;
  game_map[1][8][71] = BORDER;
  game_map[1][7][71] = BORDER;
  game_map[1][7][70] = BORDER;
  game_map[1][7][69] = BORDER;
  game_map[1][7][68] = BORDER;
  game_map[1][8][68] = BORDER;
  game_map[1][9][68] = BORDER;
  game_map[1][9][67] = BORDER;
  game_map[1][9][66] = BORDER;
  game_map[1][9][65] = BORDER;
  game_map[1][9][64] = BORDER;
  game_map[1][9][63] = BORDER;
  game_map[1][9][62] = BORDER;
  game_map[1][9][61] = BORDER;
  game_map[1][8][61] = BORDER;
  game_map[1][7][61] = BORDER;
  game_map[1][6][61] = BORDER;
  game_map[1][5][61] = BORDER;
  game_map[1][4][61] = BORDER;
  game_map[1][3][61] = BORDER;
  game_map[1][2][61] = BORDER;
  game_map[1][1][61] = BORDER;
  game_map[1][1][71] = BORDER;
  game_map[1][2][71] = BORDER;
  game_map[1][3][71] = BORDER;
  game_map[1][3][70] = BORDER;
  game_map[1][3][69] = BORDER;
  game_map[1][3][68] = BORDER;
  game_map[1][1][12] = BORDER;
  game_map[1][22][56] = BORDER;
  game_map[1][23][56] = BORDER;
  game_map[1][24][56] = BORDER;
  game_map[1][7][51] = BORDER;
  game_map[1][7][52] = BORDER;
  game_map[1][7][53] = BORDER;
  game_map[1][7][54] = BORDER;
  game_map[1][7][55] = BORDER;
  game_map[1][7][56] = BORDER;
  game_map[1][7][57] = BORDER;
  game_map[1][7][58] = BORDER;
  game_map[1][6][58] = BORDER;
  game_map[1][5][58] = BORDER;
  game_map[1][4][58] = BORDER;
  game_map[1][3][58] = BORDER;
  game_map[1][2][58] = BORDER;
  game_map[1][1][58] = BORDER;
  game_map[1][10][45] = BORDER;
  game_map[1][10][46] = BORDER;
  game_map[1][9][47] = BORDER;
  game_map[1][8][47] = BORDER;
  game_map[1][7][47] = BORDER;
  game_map[1][7][46] = BORDER;
  game_map[1][6][46] = BORDER;
  game_map[1][5][46] = BORDER;
  game_map[1][4][46] = BORDER;
  game_map[1][3][46] = BORDER;
  game_map[1][2][46] = BORDER;
  game_map[1][1][46] = BORDER;
  game_map[1][9][32] = BORDER;
  game_map[1][8][32] = BORDER;
  game_map[1][7][32] = BORDER;
  game_map[1][7][33] = BORDER;
  game_map[1][7][34] = BORDER;
  game_map[1][7][35] = BORDER;
  game_map[1][7][36] = BORDER;
  game_map[1][7][37] = BORDER;
  game_map[1][7][38] = BORDER;
  game_map[1][7][39] = BORDER;
  game_map[1][7][40] = BORDER;
  game_map[1][6][40] = BORDER;
  game_map[1][5][40] = BORDER;
  game_map[1][4][40] = BORDER;
  game_map[1][3][40] = BORDER;
  game_map[1][2][40] = BORDER;
  game_map[1][1][40] = BORDER;
  game_map[1][8][20] = BORDER;
  game_map[1][8][21] = BORDER;
  game_map[1][8][22] = BORDER;
  game_map[1][8][23] = BORDER;
  game_map[1][7][23] = BORDER;
  game_map[1][6][23] = BORDER;
  game_map[1][5][23] = BORDER;
  game_map[1][4][23] = BORDER;
  game_map[1][3][23] = BORDER;
  game_map[1][2][23] = BORDER;
  game_map[1][1][23] = BORDER;

  game_map[2][1][3] = BORDER;
  game_map[2][2][3] = BORDER;
  game_map[2][3][3] = BORDER;
  game_map[2][6][3] = BORDER;
  game_map[2][6][4] = BORDER;
  game_map[2][6][5] = BORDER;
  game_map[2][6][6] = BORDER;
  game_map[2][6][7] = BORDER;
  game_map[2][6][8] = BORDER;
  game_map[2][6][9] = BORDER;
  game_map[2][6][10] = BORDER;
  game_map[2][6][11] = BORDER;
  game_map[2][6][12] = BORDER;
  game_map[2][6][13] = BORDER;
  game_map[2][6][14] = BORDER;
  game_map[2][6][15] = BORDER;
  game_map[2][6][16] = BORDER;
  game_map[2][7][16] = BORDER;
  game_map[2][8][16] = BORDER;
  game_map[2][8][17] = BORDER;
  game_map[2][8][18] = BORDER;
  game_map[2][8][19] = BORDER;
  game_map[2][8][20] = BORDER;
  game_map[2][8][21] = BORDER;
  game_map[2][8][22] = BORDER;
  game_map[2][8][23] = BORDER;
  game_map[2][7][23] = BORDER;
  game_map[2][6][23] = BORDER;
  game_map[2][5][23] = BORDER;
  game_map[2][4][23] = BORDER;
  game_map[2][4][24] = BORDER;
  game_map[2][4][25] = BORDER;
  game_map[2][4][26] = BORDER;
  game_map[2][4][27] = BORDER;
  game_map[2][5][27] = BORDER;
  game_map[2][6][27] = BORDER;
  game_map[2][7][27] = BORDER;
  game_map[2][8][27] = BORDER;
  game_map[2][8][28] = BORDER;
  game_map[2][8][29] = BORDER;
  game_map[2][8][30] = BORDER;
  game_map[2][8][31] = BORDER;
  game_map[2][8][32] = BORDER;
  game_map[2][8][33] = BORDER;
  game_map[2][8][34] = BORDER;
  game_map[2][7][34] = BORDER;
  game_map[2][6][34] = BORDER;
  game_map[2][5][34] = BORDER;
  game_map[2][8][35] = BORDER;
  game_map[2][8][36] = BORDER;
  game_map[2][8][37] = BORDER;
  game_map[2][8][38] = BORDER;
  game_map[2][8][39] = BORDER;
  game_map[2][7][39] = BORDER;
  game_map[2][5][39] = BORDER;
  game_map[2][6][39] = BORDER;
  game_map[2][5][39] = BORDER;
  game_map[2][5][40] = BORDER;
  game_map[2][5][41] = BORDER;
  game_map[2][5][42] = BORDER;
  game_map[2][5][43] = BORDER;
  game_map[2][5][44] = BORDER;
  game_map[2][5][45] = BORDER;
  game_map[2][5][46] = BORDER;
  game_map[2][5][47] = BORDER;
  game_map[2][2][39] = BORDER;
  game_map[2][5][40] = BORDER;
  game_map[2][2][41] = BORDER;
  game_map[2][2][42] = BORDER;
  game_map[2][2][43] = BORDER;
  game_map[2][2][44] = BORDER;
  game_map[2][2][45] = BORDER;
  game_map[2][2][46] = BORDER;
  game_map[2][2][47] = BORDER;
  game_map[2][1][47] = BORDER;
  game_map[2][6][47] = BORDER;
  game_map[2][7][47] = BORDER;
  game_map[2][7][48] = BORDER;
  game_map[2][7][49] = BORDER;
  game_map[2][7][50] = BORDER;
  game_map[2][7][51] = BORDER;
  game_map[2][7][52] = BORDER;
  game_map[2][7][53] = BORDER;
  game_map[2][7][54] = BORDER;
  game_map[2][7][55] = BORDER;
  game_map[2][7][56] = BORDER;
  game_map[2][7][57] = BORDER;
  game_map[2][7][58] = BORDER;
  game_map[2][7][59] = BORDER;
  game_map[2][7][60] = BORDER;
  game_map[2][7][61] = BORDER;
  game_map[2][7][62] = BORDER;
  game_map[2][8][62] = BORDER;
  game_map[2][9][62] = BORDER;
  game_map[2][9][63] = BORDER;
  game_map[2][9][64] = BORDER;
  game_map[2][9][65] = BORDER;
  game_map[2][9][66] = BORDER;
  game_map[2][9][67] = BORDER;
  game_map[2][9][68] = BORDER;
  game_map[2][9][69] = BORDER;
  game_map[2][4][70] = BORDER;
  game_map[2][4][71] = BORDER;
  game_map[2][4][72] = BORDER;
  game_map[2][7][70] = BORDER;
  game_map[2][7][71] = BORDER;
  game_map[2][7][72] = BORDER;
  game_map[2][8][72] = BORDER;
  game_map[2][9][72] = BORDER;
  game_map[2][10][73] = BORDER;
  game_map[2][11][73] = BORDER;
  game_map[2][12][73] = BORDER;
  game_map[2][13][73] = BORDER;
  game_map[2][13][78] = BORDER;
  game_map[2][13][79] = BORDER;
  game_map[2][8][69] = BORDER;
  game_map[2][7][69] = BORDER;
  game_map[2][13][72] = BORDER;
  game_map[2][13][71] = BORDER;
  game_map[2][13][70] = BORDER;
  game_map[2][13][69] = BORDER;
  game_map[2][13][68] = BORDER;
  game_map[2][13][67] = BORDER;
  game_map[2][13][66] = BORDER;
  game_map[2][13][65] = BORDER;
  game_map[2][13][64] = BORDER;
  game_map[2][13][63] = BORDER;
  game_map[2][13][62] = BORDER;
  game_map[2][14][62] = BORDER;
  game_map[2][15][62] = BORDER;
  game_map[2][15][61] = BORDER;
  game_map[2][15][60] = BORDER;
  game_map[2][15][59] = BORDER;
  game_map[2][15][58] = BORDER;
  game_map[2][14][58] = BORDER;
  game_map[2][12][58] = BORDER;
  game_map[2][11][58] = BORDER;
  game_map[2][10][58] = BORDER;
  game_map[2][9][58] = BORDER;
  game_map[2][9][57] = BORDER;
  game_map[2][9][56] = BORDER;
  game_map[2][9][55] = BORDER;
  game_map[2][9][54] = BORDER;
  game_map[2][9][56] = BORDER;
  game_map[2][9][55] = BORDER;
  game_map[2][9][54] = BORDER;
  game_map[2][9][53] = BORDER;
  game_map[2][9][52] = BORDER;
  game_map[2][9][51] = BORDER;
  game_map[2][9][50] = BORDER;
  game_map[2][9][49] = BORDER;
  game_map[2][9][48] = BORDER;
  game_map[2][9][47] = BORDER;
  game_map[2][9][46] = BORDER;
  game_map[2][9][45] = BORDER;
  game_map[2][9][44] = BORDER;
  game_map[2][9][43] = BORDER;
  game_map[2][9][42] = BORDER;
  game_map[2][9][40] = BORDER;
  game_map[2][9][39] = BORDER;
  game_map[2][9][38] = BORDER;
  game_map[2][9][49] = BORDER;
  game_map[2][9][37] = BORDER;
  game_map[2][9][36] = BORDER;
  game_map[2][9][35] = BORDER;
  game_map[2][9][35] = BORDER;
  game_map[2][11][35] = BORDER;
  game_map[2][12][35] = BORDER;
  game_map[2][13][35] = BORDER;
  game_map[2][14][35] = BORDER;
  game_map[2][15][35] = BORDER;
  game_map[2][16][35] = BORDER;
  game_map[2][17][35] = BORDER;
  game_map[2][17][34] = BORDER;
  game_map[2][17][33] = BORDER;
  game_map[2][17][32] = BORDER;
  game_map[2][17][31] = BORDER;
  game_map[2][17][30] = BORDER;
  game_map[2][17][29] = BORDER;
  game_map[2][17][28] = BORDER;
  game_map[2][17][27] = BORDER;
  game_map[2][17][26] = BORDER;
  game_map[2][17][25] = BORDER;
  game_map[2][16][25] = BORDER;
  game_map[2][15][25] = BORDER;
  game_map[2][14][25] = BORDER;
  game_map[2][13][25] = BORDER;
  game_map[2][11][25] = BORDER;
  game_map[2][11][24] = BORDER;
  game_map[2][11][23] = BORDER;
  game_map[2][11][22] = BORDER;
  game_map[2][11][21] = BORDER;
  game_map[2][11][20] = BORDER;
  game_map[2][11][19] = BORDER;
  game_map[2][11][18] = BORDER;
  game_map[2][11][17] = BORDER;
  game_map[2][11][16] = BORDER;
  game_map[2][12][16] = BORDER;
  game_map[2][13][16] = BORDER;
  game_map[2][14][16] = BORDER;
  game_map[2][15][16] = BORDER;
  game_map[2][16][16] = BORDER;
  game_map[2][17][16] = BORDER;
  game_map[2][18][16] = BORDER;
  game_map[2][19][16] = BORDER;
  game_map[2][20][16] = BORDER;
  game_map[2][21][16] = BORDER;
  game_map[2][21][15] = BORDER;
  game_map[2][21][14] = BORDER;
  game_map[2][21][13] = BORDER;
  game_map[2][20][13] = BORDER;
  game_map[2][18][13] = BORDER;
  game_map[2][17][13] = BORDER;
  game_map[2][16][13] = BORDER;
  game_map[2][15][13] = BORDER;
  game_map[2][14][13] = BORDER;
  game_map[2][14][12] = BORDER;
  game_map[2][14][11] = BORDER;
  game_map[2][14][10] = BORDER;
  game_map[2][14][9] = BORDER;
  game_map[2][14][8] = BORDER;
  game_map[2][14][7] = BORDER;
  game_map[2][14][6] = BORDER;
  game_map[2][13][6] = BORDER;
  game_map[2][12][6] = BORDER;
  game_map[2][11][6] = BORDER;
  game_map[2][10][6] = BORDER;
  game_map[2][9][6] = BORDER;
  game_map[2][8][6] = BORDER;
  game_map[2][7][6] = BORDER;
  game_map[2][21][25] = BORDER;
  game_map[2][22][25] = BORDER;
  game_map[2][23][25] = BORDER;
  game_map[2][24][25] = BORDER;
  game_map[2][10][35] = BORDER;
  game_map[2][10][34] = BORDER;
  game_map[2][10][33] = BORDER;
  game_map[2][10][32] = BORDER;
  game_map[2][10][31] = BORDER;
  game_map[2][10][30] = BORDER;
  game_map[2][10][29] = BORDER;
  game_map[2][10][28] = BORDER;
  game_map[2][10][27] = BORDER;
  game_map[2][10][26] = BORDER;
  game_map[2][10][25] = BORDER;
  game_map[2][20][35] = BORDER;
  game_map[2][21][35] = BORDER;
  game_map[2][22][35] = BORDER;
  game_map[2][23][35] = BORDER;
  game_map[2][24][35] = BORDER;
  game_map[2][18][61] = BORDER;
  game_map[2][18][60] = BORDER;
  game_map[2][18][59] = BORDER;
  game_map[2][18][58] = BORDER;
  game_map[2][19][58] = BORDER;
  game_map[2][20][58] = BORDER;
  game_map[2][21][58] = BORDER;
  game_map[2][23][58] = BORDER;
  game_map[2][24][58] = BORDER;
  game_map[2][24][58] = BORDER;
  game_map[2][18][62] = BORDER;
  game_map[2][19][62] = BORDER;
  game_map[2][20][62] = BORDER;
  game_map[2][21][62] = BORDER;
  game_map[2][22][62] = BORDER;
  game_map[2][23][62] = BORDER;
  game_map[2][24][62] = BORDER;
  game_map[2][2][69] = BORDER;
  game_map[2][1][69] = BORDER;
  game_map[2][6][72] = BORDER;
  game_map[2][8][40] = BORDER;
  game_map[2][9][41] = BORDER;
  game_map[2][2][72] = BORDER;
  game_map[2][1][72] = BORDER;
  game_map[2][2][13] = BORDER;
  game_map[2][1][13] = BORDER;
  game_map[2][2][14] = BORDER;
  game_map[2][2][15] = BORDER;
  game_map[2][2][16] = BORDER;
  game_map[2][1][16] = BORDER;
  game_map[2][9][1] = BORDER;
  game_map[2][10][2] = BORDER;
  game_map[2][11][1] = BORDER;
  game_map[2][12][1] = BORDER;
  game_map[2][13][1] = BORDER;
  game_map[2][14][1] = BORDER;
  game_map[2][15][1] = BORDER;
  game_map[2][16][1] = BORDER;
  game_map[2][17][1] = BORDER;
  game_map[2][18][1] = BORDER;
  game_map[2][19][1] = BORDER;
  game_map[2][20][1] = BORDER;
  game_map[2][21][1] = BORDER;
  game_map[2][22][1] = BORDER;
  game_map[2][23][1] = BORDER;
  game_map[2][24][1] = BORDER;

  game_map[3][2][1] = BORDER;
  game_map[3][2][2] = BORDER;
  game_map[3][2][3] = BORDER;
  game_map[3][2][4] = BORDER;
  game_map[3][2][5] = BORDER;
  game_map[3][2][6] = BORDER;
  game_map[3][2][7] = BORDER;
  game_map[3][2][8] = BORDER;
  game_map[3][2][9] = BORDER;
  game_map[3][2][10] = BORDER;
  game_map[3][2][12] = BORDER;
  game_map[3][2][13] = BORDER;
  game_map[3][2][14] = BORDER;
  game_map[3][1][14] = BORDER;
  game_map[3][6][1] = BORDER;
  game_map[3][6][2] = BORDER;
  game_map[3][6][3] = BORDER;
  game_map[3][6][4] = BORDER;
  game_map[3][6][5] = BORDER;
  game_map[3][6][6] = BORDER;
  game_map[3][6][7] = BORDER;
  game_map[3][6][8] = BORDER;
  game_map[3][6][9] = BORDER;
  game_map[3][6][10] = BORDER;
  game_map[3][7][10] = BORDER;
  game_map[3][8][10] = BORDER;
  game_map[3][9][10] = BORDER;
  game_map[3][10][10] = BORDER;
  game_map[3][12][10] = BORDER;
  game_map[3][13][10] = BORDER;
  game_map[3][14][10] = BORDER;
  game_map[3][15][10] = BORDER;
  game_map[3][16][10] = BORDER;
  game_map[3][17][10] = BORDER;
  game_map[3][18][10] = BORDER;
  game_map[3][19][10] = BORDER;
  game_map[3][20][10] = BORDER;
  game_map[3][21][10] = BORDER;
  game_map[3][22][10] = BORDER;
  game_map[3][23][10] = BORDER;
  game_map[3][24][10] = BORDER;
  game_map[3][6][14] = BORDER;
  game_map[3][7][14] = BORDER;
  game_map[3][8][14] = BORDER;
  game_map[3][9][14] = BORDER;
  game_map[3][10][14] = BORDER;
  game_map[3][10][15] = BORDER;
  game_map[3][10][16] = BORDER;
  game_map[3][10][17] = BORDER;
  game_map[3][10][18] = BORDER;
  game_map[3][10][19] = BORDER;
  game_map[3][10][20] = BORDER;
  game_map[3][10][21] = BORDER;
  game_map[3][10][22] = BORDER;
  game_map[3][10][23] = BORDER;
  game_map[3][10][25] = BORDER;
  game_map[3][9][25] = BORDER;
  game_map[3][8][25] = BORDER;
  game_map[3][7][25] = BORDER;
  game_map[3][7][26] = BORDER;
  game_map[3][7][27] = BORDER;
  game_map[3][7][28] = BORDER;
  game_map[3][7][29] = BORDER;
  game_map[3][7][29] = BORDER;
  game_map[3][7][30] = BORDER;
  game_map[3][7][31] = BORDER;
  game_map[3][7][32] = BORDER;
  game_map[3][7][33] = BORDER;
  game_map[3][7][34] = BORDER;
  game_map[3][7][35] = BORDER;
  game_map[3][7][36] = BORDER;
  game_map[3][7][37] = BORDER;
  game_map[3][7][38] = BORDER;
  game_map[3][6][38] = BORDER;
  game_map[3][5][38] = BORDER;
  game_map[3][4][38] = BORDER;
  game_map[3][4][39] = BORDER;
  game_map[3][4][40] = BORDER;
  game_map[3][4][41] = BORDER;
  game_map[3][4][42] = BORDER;
  game_map[3][4][43] = BORDER;
  game_map[3][4][44] = BORDER;
  game_map[3][5][44] = BORDER;
  game_map[3][6][44] = BORDER;
  game_map[3][7][44] = BORDER;
  game_map[3][8][44] = BORDER;
  game_map[3][9][44] = BORDER;
  game_map[3][10][44] = BORDER;
  game_map[3][10][45] = BORDER;
  game_map[3][10][46] = BORDER;
  game_map[3][10][47] = BORDER;
  game_map[3][10][48] = BORDER;
  game_map[3][10][49] = BORDER;
  game_map[3][10][50] = BORDER;
  game_map[3][10][51] = BORDER;
  game_map[3][10][52] = BORDER;
  game_map[3][10][53] = BORDER;
  game_map[3][10][54] = BORDER;
  game_map[3][10][55] = BORDER;
  game_map[3][10][56] = BORDER;
  game_map[3][10][57] = BORDER;
  game_map[3][10][58] = BORDER;
  game_map[3][10][59] = BORDER;
  game_map[3][10][60] = BORDER;
  game_map[3][10][61] = BORDER;
  game_map[3][10][62] = BORDER;
  game_map[3][10][62] = BORDER;
  game_map[3][10][62] = BORDER;
  game_map[3][10][62] = BORDER;
  game_map[3][10][62] = BORDER;
  game_map[3][10][63] = BORDER;
  game_map[3][10][64] = BORDER;
  game_map[3][10][65] = BORDER;
  game_map[3][10][66] = BORDER;
  game_map[3][10][67] = BORDER;
  game_map[3][10][68] = BORDER;
  game_map[3][1][68] = BORDER;
  game_map[3][2][68] = BORDER;
  game_map[3][3][68] = BORDER;
  game_map[3][3][69] = BORDER;
  game_map[3][3][70] = BORDER;
  game_map[3][2][70] = BORDER;
  game_map[3][1][70] = BORDER;
  game_map[3][5][69] = BORDER;
  game_map[3][5][70] = BORDER;
  game_map[3][6][70] = BORDER;
  game_map[3][7][70] = BORDER;
  game_map[3][8][70] = BORDER;
  game_map[3][8][71] = BORDER;
  game_map[3][8][72] = BORDER;
  game_map[3][8][73] = BORDER;
  game_map[3][8][74] = BORDER;
  game_map[3][8][75] = BORDER;
  game_map[3][8][76] = BORDER;
  game_map[3][8][77] = BORDER;
  game_map[3][8][78] = BORDER;
  game_map[3][8][79] = BORDER;
  game_map[3][5][68] = BORDER;
  game_map[3][6][68] = BORDER;
  game_map[3][7][68] = BORDER;
  game_map[3][8][68] = BORDER;
  game_map[3][9][68] = BORDER;
  game_map[3][9][62] = BORDER;
  game_map[3][8][62] = BORDER;
  game_map[3][7][62] = BORDER;
  game_map[3][6][62] = BORDER;
  game_map[3][1][25] = BORDER;
  game_map[3][3][25] = BORDER;
  game_map[3][3][26] = BORDER;
  game_map[3][3][27] = BORDER;
  game_map[3][3][28] = BORDER;
  game_map[3][3][29] = BORDER;
  game_map[3][2][29] = BORDER;
  game_map[3][1][29] = BORDER;
  game_map[3][6][13] = BORDER;
  game_map[3][7][13] = BORDER;
  game_map[3][8][13] = BORDER;
  game_map[3][9][13] = BORDER;
  game_map[3][10][13] = BORDER;
  game_map[3][11][13] = BORDER;
  game_map[3][12][13] = BORDER;
  game_map[3][13][13] = BORDER;
  game_map[3][14][13] = BORDER;
  game_map[3][15][13] = BORDER;
  game_map[3][16][13] = BORDER;
  game_map[3][17][13] = BORDER;
  game_map[3][17][14] = BORDER;
  game_map[3][17][15] = BORDER;
  game_map[3][17][16] = BORDER;
  game_map[3][17][17] = BORDER;
  game_map[3][17][18] = BORDER;
  game_map[3][17][19] = BORDER;
  game_map[3][17][20] = BORDER;
  game_map[3][17][21] = BORDER;
  game_map[3][17][22] = BORDER;
  game_map[3][17][23] = BORDER;
  game_map[3][17][24] = BORDER;
  game_map[3][17][25] = BORDER;
  game_map[3][17][26] = BORDER;
  game_map[3][17][27] = BORDER;
  game_map[3][17][28] = BORDER;
  game_map[3][17][29] = BORDER;
  game_map[3][18][29] = BORDER;
  game_map[3][19][29] = BORDER;
  game_map[3][20][29] = BORDER;
  game_map[3][20][30] = BORDER;
  game_map[3][20][31] = BORDER;
  game_map[3][20][32] = BORDER;
  game_map[3][20][33] = BORDER;
  game_map[3][20][34] = BORDER;
  game_map[3][20][35] = BORDER;
  game_map[3][20][36] = BORDER;
  game_map[3][20][37] = BORDER;
  game_map[3][20][38] = BORDER;
  game_map[3][19][38] = BORDER;
  game_map[3][18][38] = BORDER;
  game_map[3][17][38] = BORDER;
  game_map[3][16][38] = BORDER;
  game_map[3][15][38] = BORDER;
  game_map[3][14][38] = BORDER;
  game_map[3][14][39] = BORDER;
  game_map[3][14][40] = BORDER;
  game_map[3][14][41] = BORDER;
  game_map[3][14][42] = BORDER;
  game_map[3][14][43] = BORDER;
  game_map[3][14][44] = BORDER;
  game_map[3][14][45] = BORDER;
  game_map[3][14][45] = BORDER;
  game_map[3][15][45] = BORDER;
  game_map[3][16][45] = BORDER;
  game_map[3][16][46] = BORDER;
  game_map[3][16][47] = BORDER;
  game_map[3][16][48] = BORDER;
  game_map[3][16][49] = BORDER;
  game_map[3][16][50] = BORDER;
  game_map[3][15][50] = BORDER;
  game_map[3][14][50] = BORDER;
  game_map[3][12][50] = BORDER;
  game_map[3][11][50] = BORDER;
  game_map[3][11][51] = BORDER;
  game_map[3][11][52] = BORDER;
  game_map[3][11][53] = BORDER;
  game_map[3][11][54] = BORDER;
  game_map[3][11][55] = BORDER;
  game_map[3][11][56] = BORDER;
  game_map[3][11][57] = BORDER;
  game_map[3][11][58] = BORDER;
  game_map[3][11][59] = BORDER;
  game_map[3][11][60] = BORDER;
  game_map[3][11][61] = BORDER;
  game_map[3][11][62] = BORDER;
  game_map[3][11][62] = BORDER;
  game_map[3][12][62] = BORDER;
  game_map[3][13][62] = BORDER;
  game_map[3][14][62] = BORDER;
  game_map[3][15][62] = BORDER;
  game_map[3][16][62] = BORDER;
  game_map[3][17][62] = BORDER;
  game_map[3][19][62] = BORDER;
  game_map[3][20][62] = BORDER;
  game_map[3][20][63] = BORDER;
  game_map[3][20][64] = BORDER;
  game_map[3][20][65] = BORDER;
  game_map[3][20][66] = BORDER;
  game_map[3][20][67] = BORDER;
  game_map[3][19][67] = BORDER;
  game_map[3][18][67] = BORDER;
  game_map[3][17][67] = BORDER;
  game_map[3][16][67] = BORDER;
  game_map[3][15][67] = BORDER;
  game_map[3][14][67] = BORDER;
  game_map[3][13][67] = BORDER;
  game_map[3][13][68] = BORDER;
  game_map[3][13][69] = BORDER;
  game_map[3][13][70] = BORDER;
  game_map[3][13][71] = BORDER;
  game_map[3][13][72] = BORDER;
  game_map[3][13][73] = BORDER;
  game_map[3][13][74] = BORDER;
  game_map[3][13][75] = BORDER;
  game_map[3][13][76] = BORDER;
  game_map[3][13][77] = BORDER;
  game_map[3][13][78] = BORDER;
  game_map[3][13][79] = BORDER;
  game_map[3][20][45] = BORDER;
  game_map[3][22][45] = BORDER;
  game_map[3][23][45] = BORDER;
  game_map[3][24][45] = BORDER;
  game_map[3][20][46] = BORDER;
  game_map[3][20][47] = BORDER;
  game_map[3][20][48] = BORDER;
  game_map[3][20][49] = BORDER;
  game_map[3][20][50] = BORDER;
  game_map[3][21][50] = BORDER;
  game_map[3][22][50] = BORDER;
  game_map[3][23][50] = BORDER;
  game_map[3][24][50] = BORDER;

  game_map[4][1][6] = BORDER;
  game_map[4][2][6] = BORDER;
  game_map[4][3][6] = BORDER;
  game_map[4][4][6] = BORDER;
  game_map[4][5][6] = BORDER;
  game_map[4][6][6] = BORDER;
  game_map[4][9][7] = BORDER;
  game_map[4][9][8] = BORDER;
  game_map[4][9][9] = BORDER;
  game_map[4][9][10] = BORDER;
  game_map[4][9][11] = BORDER;
  game_map[4][9][12] = BORDER;
  game_map[4][9][13] = BORDER;
  game_map[4][9][14] = BORDER;
  game_map[4][9][15] = BORDER;
  game_map[4][9][16] = BORDER;
  game_map[4][9][17] = BORDER;
  game_map[4][9][18] = BORDER;
  game_map[4][9][19] = BORDER;
  game_map[4][9][20] = BORDER;
  game_map[4][9][21] = BORDER;
  game_map[4][9][22] = BORDER;
  game_map[4][8][22] = BORDER;
  game_map[4][7][22] = BORDER;
  game_map[4][6][22] = BORDER;
  game_map[4][5][22] = BORDER;
  game_map[4][4][22] = BORDER;
  game_map[4][4][23] = BORDER;
  game_map[4][4][24] = BORDER;
  game_map[4][4][25] = BORDER;
  game_map[4][4][26] = BORDER;
  game_map[4][4][27] = BORDER;
  game_map[4][4][28] = BORDER;
  game_map[4][4][29] = BORDER;
  game_map[4][4][30] = BORDER;
  game_map[4][5][30] = BORDER;
  game_map[4][6][30] = BORDER;
  game_map[4][7][30] = BORDER;
  game_map[4][8][30] = BORDER;
  game_map[4][9][30] = BORDER;
  game_map[4][10][30] = BORDER;
  game_map[4][10][31] = BORDER;
  game_map[4][10][32] = BORDER;
  game_map[4][10][33] = BORDER;
  game_map[4][10][34] = BORDER;
  game_map[4][10][35] = BORDER;
  game_map[4][10][36] = BORDER;
  game_map[4][10][37] = BORDER;
  game_map[4][10][38] = BORDER;
  game_map[4][10][39] = BORDER;
  game_map[4][10][40] = BORDER;
  game_map[4][10][41] = BORDER;
  game_map[4][10][42] = BORDER;
  game_map[4][10][43] = BORDER;
  game_map[4][10][44] = BORDER;
  game_map[4][9][44] = BORDER;
  game_map[4][8][44] = BORDER;
  game_map[4][7][44] = BORDER;
  game_map[4][6][44] = BORDER;
  game_map[4][5][44] = BORDER;
  game_map[4][4][44] = BORDER;
  game_map[4][4][45] = BORDER;
  game_map[4][4][46] = BORDER;
  game_map[4][4][47] = BORDER;
  game_map[4][4][48] = BORDER;
  game_map[4][4][49] = BORDER;
  game_map[4][4][50] = BORDER;
  game_map[4][5][50] = BORDER;
  game_map[4][6][50] = BORDER;
  game_map[4][7][50] = BORDER;
  game_map[4][8][50] = BORDER;
  game_map[4][9][50] = BORDER;
  game_map[4][9][51] = BORDER;
  game_map[4][9][52] = BORDER;
  game_map[4][9][53] = BORDER;
  game_map[4][9][54] = BORDER;
  game_map[4][9][55] = BORDER;
  game_map[4][9][56] = BORDER;
  game_map[4][9][57] = BORDER;
  game_map[4][9][58] = BORDER;
  game_map[4][9][59] = BORDER;
  game_map[4][9][60] = BORDER;
  game_map[4][8][60] = BORDER;
  game_map[4][7][60] = BORDER;
  game_map[4][6][60] = BORDER;
  game_map[4][5][60] = BORDER;
  game_map[4][9][61] = BORDER;
  game_map[4][9][62] = BORDER;
  game_map[4][9][63] = BORDER;
  game_map[4][9][64] = BORDER;
  game_map[4][9][65] = BORDER;
  game_map[4][9][66] = BORDER;
  game_map[4][9][67] = BORDER;
  game_map[4][8][67] = BORDER;
  game_map[4][7][67] = BORDER;
  game_map[4][7][68] = BORDER;
  game_map[4][7][69] = BORDER;
  game_map[4][7][70] = BORDER;
  game_map[4][8][70] = BORDER;
  game_map[4][9][70] = BORDER;
  game_map[4][10][70] = BORDER;
  game_map[4][10][71] = BORDER;
  game_map[4][10][72] = BORDER;
  game_map[4][10][73] = BORDER;
  game_map[4][10][74] = BORDER;
  game_map[4][10][75] = BORDER;
  game_map[4][10][76] = BORDER;
  game_map[4][10][77] = BORDER;
  game_map[4][5][68] = BORDER;
  game_map[4][5][69] = BORDER;
  game_map[4][5][70] = BORDER;
  game_map[4][4][70] = BORDER;
  game_map[4][3][70] = BORDER;
  game_map[4][2][70] = BORDER;
  game_map[4][1][70] = BORDER;
  game_map[4][1][67] = BORDER;
  game_map[4][2][67] = BORDER;
  game_map[4][3][67] = BORDER;
  game_map[4][4][67] = BORDER;
  game_map[4][5][67] = BORDER;
  game_map[4][8][6] = BORDER;
  game_map[4][9][6] = BORDER;
  game_map[4][10][6] = BORDER;
  game_map[4][11][6] = BORDER;
  game_map[4][12][6] = BORDER;
  game_map[4][13][6] = BORDER;
  game_map[4][14][6] = BORDER;
  game_map[4][15][6] = BORDER;
  game_map[4][16][6] = BORDER;
  game_map[4][17][6] = BORDER;
  game_map[4][17][7] = BORDER;
  game_map[4][17][8] = BORDER;
  game_map[4][17][9] = BORDER;
  game_map[4][17][10] = BORDER;
  game_map[4][17][11] = BORDER;
  game_map[4][16][11] = BORDER;
  game_map[4][15][11] = BORDER;
  game_map[4][14][11] = BORDER;
  game_map[4][13][11] = BORDER;
  game_map[4][12][11] = BORDER;
  game_map[4][12][12] = BORDER;
  game_map[4][12][13] = BORDER;
  game_map[4][12][14] = BORDER;
  game_map[4][12][15] = BORDER;
  game_map[4][12][16] = BORDER;
  game_map[4][12][17] = BORDER;
  game_map[4][12][18] = BORDER;
  game_map[4][12][19] = BORDER;
  game_map[4][12][20] = BORDER;
  game_map[4][12][21] = BORDER;
  game_map[4][12][22] = BORDER;
  game_map[4][13][22] = BORDER;
  game_map[4][14][22] = BORDER;
  game_map[4][15][22] = BORDER;
  game_map[4][16][22] = BORDER;
  game_map[4][17][22] = BORDER;
  game_map[4][18][22] = BORDER;
  game_map[4][18][23] = BORDER;
  game_map[4][18][24] = BORDER;
  game_map[4][18][25] = BORDER;
  game_map[4][18][26] = BORDER;
  game_map[4][17][26] = BORDER;
  game_map[4][16][26] = BORDER;
  game_map[4][15][26] = BORDER;
  game_map[4][14][26] = BORDER;
  game_map[4][13][26] = BORDER;
  game_map[4][13][27] = BORDER;
  game_map[4][13][28] = BORDER;
  game_map[4][13][29] = BORDER;
  game_map[4][13][30] = BORDER;
  game_map[4][13][31] = BORDER;
  game_map[4][13][32] = BORDER;
  game_map[4][13][33] = BORDER;
  game_map[4][13][34] = BORDER;
  game_map[4][13][35] = BORDER;
  game_map[4][14][35] = BORDER;
  game_map[4][15][35] = BORDER;
  game_map[4][16][35] = BORDER;
  game_map[4][16][36] = BORDER;
  game_map[4][16][37] = BORDER;
  game_map[4][16][38] = BORDER;
  game_map[4][16][39] = BORDER;
  game_map[4][16][40] = BORDER;
  game_map[4][16][41] = BORDER;
  game_map[4][16][42] = BORDER;
  game_map[4][16][43] = BORDER;
  game_map[4][15][43] = BORDER;
  game_map[4][14][43] = BORDER;
  game_map[4][13][43] = BORDER;
  game_map[4][12][43] = BORDER;
  game_map[4][12][44] = BORDER;
  game_map[4][12][45] = BORDER;
  game_map[4][12][46] = BORDER;
  game_map[4][12][47] = BORDER;
  game_map[4][12][48] = BORDER;
  game_map[4][12][49] = BORDER;
  game_map[4][12][50] = BORDER;
  game_map[4][12][51] = BORDER;
  game_map[4][12][52] = BORDER;
  game_map[4][12][53] = BORDER;
  game_map[4][12][54] = BORDER;
  game_map[4][12][55] = BORDER;
  game_map[4][12][56] = BORDER;
  game_map[4][12][57] = BORDER;
  game_map[4][12][58] = BORDER;
  game_map[4][12][59] = BORDER;
  game_map[4][12][60] = BORDER;
  game_map[4][12][61] = BORDER;
  game_map[4][12][62] = BORDER;
  game_map[4][13][62] = BORDER;
  game_map[4][14][62] = BORDER;
  game_map[4][15][62] = BORDER;
  game_map[4][16][62] = BORDER;
  game_map[4][17][62] = BORDER;
  game_map[4][18][62] = BORDER;
  game_map[4][19][62] = BORDER;
  game_map[4][20][62] = BORDER;
  game_map[4][20][63] = BORDER;
  game_map[4][20][64] = BORDER;
  game_map[4][20][65] = BORDER;
  game_map[4][20][66] = BORDER;
  game_map[4][20][67] = BORDER;
  game_map[4][19][67] = BORDER;
  game_map[4][18][67] = BORDER;
  game_map[4][17][67] = BORDER;
  game_map[4][16][67] = BORDER;
  game_map[4][15][67] = BORDER;
  game_map[4][14][67] = BORDER;
  game_map[4][14][68] = BORDER;
  game_map[4][14][69] = BORDER;
  game_map[4][14][70] = BORDER;
  game_map[4][14][71] = BORDER;
  game_map[4][14][72] = BORDER;
  game_map[4][14][73] = BORDER;
  game_map[4][14][74] = BORDER;
  game_map[4][14][75] = BORDER;
  game_map[4][14][76] = BORDER;
  game_map[4][14][77] = BORDER;
  game_map[4][14][78] = BORDER;
  game_map[4][14][79] = BORDER;
  game_map[4][20][36] = BORDER;
  game_map[4][20][37] = BORDER;
  game_map[4][20][38] = BORDER;
  game_map[4][20][39] = BORDER;
  game_map[4][20][40] = BORDER;
  game_map[4][20][41] = BORDER;
  game_map[4][20][42] = BORDER;
  game_map[4][20][43] = BORDER;
  game_map[4][21][43] = BORDER;
  game_map[4][22][43] = BORDER;
  game_map[4][23][43] = BORDER;
  game_map[4][24][43] = BORDER;
  game_map[4][20][35] = BORDER;
  game_map[4][21][35] = BORDER;
  game_map[4][22][35] = BORDER;
  game_map[4][23][35] = BORDER;

  /* Level 6 */
  set_level_walls(5, 6, 6, 6, 19);
  set_level_walls(5, 6, 20, 10, 20);
  set_level_walls(5, 11, 20, 11, 29);
  set_level_walls(5, 4, 30, 11, 30);
  set_level_walls(5, 3, 30, 3, 38);
  set_level_walls(5, 3, 39, 7, 39);
  set_level_walls(5, 8, 39, 8, 49);
  set_level_walls(5, 4, 50, 8, 50);
  set_level_walls(5, 3, 50, 3, 55);
  set_level_walls(5, 3, 56, 6, 56);
  set_level_walls(5, 7, 56, 7, 78);
  set_level_walls(5, 6, 6, 8, 6);
  set_level_walls(5, 19, 6, 19, 10);
  set_level_walls(5, 16, 11, 19, 11);
  set_level_walls(5, 15, 11, 15, 21);
  set_level_walls(5, 15, 22, 17, 22);
  set_level_walls(5, 15, 23, 15, 25);
  set_level_walls(5, 12, 26, 15, 26);
  set_level_walls(5, 11, 26, 11, 38);
  set_level_walls(5, 11, 39, 15, 39);
  set_level_walls(5, 17, 39, 19, 39);
  set_level_walls(5, 20, 39, 20, 43);
  set_level_walls(5, 14, 44, 20, 44);
  set_level_walls(5, 13, 44, 13, 50);
  set_level_walls(5, 13, 51, 20, 51);
  set_level_walls(5, 21, 51, 21, 57);
  set_level_walls(5, 12, 58, 21, 58);
  set_level_walls(5, 11, 58, 11, 61);
  set_level_walls(5, 11, 62, 17, 62);
  set_level_walls(5, 18, 62, 18, 66);
  set_level_walls(5, 13, 67, 18, 67);
  set_level_walls(5, 12, 67, 12, 79);
  set_level_walls(5, 19, 23, 19, 25);
  set_level_walls(5, 19, 26, 24, 26);
  set_level_walls(5, 19, 22, 24, 22);

  /* Level 7 */
  set_level_walls(6, 2, 5, 2, 7);
  set_level_walls(6, 2, 8, 5, 8);
  set_level_walls(6, 6, 8, 6, 21);
  set_level_walls(6, 4, 22, 6, 22);
  set_level_walls(6, 3, 22, 3, 27);
  set_level_walls(6, 3, 28, 9, 28);
  set_level_walls(6, 10, 28, 10, 34);
  set_level_walls(6, 6, 35, 10, 35);
  set_level_walls(6, 5, 35, 5, 41);
  set_level_walls(6, 5, 42, 10, 42);
  set_level_walls(6, 11, 42, 11, 49);
  set_level_walls(6, 5, 50, 11, 50);
  set_level_walls(6, 4, 50, 4, 58);
  set_level_walls(6, 4, 59, 9, 59);
  set_level_walls(6, 10, 59, 10, 66);
  set_level_walls(6, 5, 67, 10, 67);
  set_level_walls(6, 4, 67, 4, 69);
  set_level_walls(6, 4, 70, 8, 70);
  set_level_walls(6, 9, 70, 9, 78);
  set_level_walls(6, 2, 4, 8, 4);
  set_level_walls(6, 10, 4, 16, 4);
  set_level_walls(6, 17, 4, 17, 11);
  set_level_walls(6, 11, 12, 17, 12);
  set_level_walls(6, 10, 12, 10, 21);
  set_level_walls(6, 10, 22, 18, 22);
  set_level_walls(6, 19, 22, 19, 28);
  set_level_walls(6, 15, 29, 19, 29);
  set_level_walls(6, 14, 29, 14, 41);
  set_level_walls(6, 14, 42, 19, 42);
  set_level_walls(6, 20, 42, 20, 46);
  set_level_walls(6, 16, 47, 20, 47);
  set_level_walls(6, 15, 47, 15, 57);
  set_level_walls(6, 15, 58, 19, 58);
  set_level_walls(6, 20, 58, 20, 66);
  set_level_walls(6, 15, 67, 20, 67);
  set_level_walls(6, 14, 67, 14, 71);
  set_level_walls(6, 14, 72, 18, 72);
  set_level_walls(6, 19, 72, 19, 74);
  set_level_walls(6, 16, 75, 19, 75);
  set_level_walls(6, 15, 75, 15, 78);
  set_level_walls(6, 22, 23, 22, 29);
  set_level_walls(6, 23, 22, 28, 22);

  /* Level 8 */
  set_level_walls(7, 29, 22, 29, 24);
  set_level_walls(7, 22, 22, 25, 22);
  set_level_walls(7, 4, 4, 18, 4);
  set_level_walls(7, 19, 4, 19, 9);
  set_level_walls(7, 14, 10, 19, 10);
  set_level_walls(7, 13, 10, 13, 21);
  set_level_walls(7, 14, 21, 18, 21);
  set_level_walls(7, 19, 21, 19, 27);
  set_level_walls(7, 12, 28, 14, 28);
  set_level_walls(7, 16, 28, 19, 28);
  set_level_walls(7, 11, 28, 11, 38);
  set_level_walls(7, 12, 38, 21, 38);
  set_level_walls(7, 21, 39, 21, 44);
  set_level_walls(7, 14, 44, 20, 44);
  set_level_walls(7, 14, 45, 14, 50);
  set_level_walls(7, 15, 50, 21, 50);
  set_level_walls(7, 21, 51, 21, 56);
  set_level_walls(7, 13, 56, 20, 56);
  set_level_walls(7, 13, 57, 13, 62);
  set_level_walls(7, 14, 62, 19, 62);
  set_level_walls(7, 19, 63, 19, 65);
  set_level_walls(7, 13, 65, 18, 65);
  set_level_walls(7, 13, 66, 13, 68);
  set_level_walls(7, 70, 28, 13, 78);
  set_level_walls(7, 3, 4, 3, 12);
  set_level_walls(7, 4, 12, 9, 12);
  set_level_walls(7, 10, 12, 10, 20);
  set_level_walls(7, 8, 20, 9, 20);
  set_level_walls(7, 8, 21, 8, 25);
  set_level_walls(7, 9, 25, 10, 25);
  set_level_walls(7, 11, 25, 11, 35);
  set_level_walls(7, 4, 35, 10, 35);
  set_level_walls(7, 4, 36, 4, 42);
  set_level_walls(7, 5, 42, 10, 42);
  set_level_walls(7, 10, 43, 10, 51);
  set_level_walls(7, 5, 51, 9, 51);
  set_level_walls(7, 5, 52, 5, 61);
  set_level_walls(7, 6, 61, 11, 61);
  set_level_walls(7, 11, 62, 11, 68);
  set_level_walls(7, 4, 68, 10, 68);
  set_level_walls(7, 4, 69, 4, 71);
  set_level_walls(7, 5, 71, 10, 71);
  set_level_walls(7, 10, 72, 10, 78);
  set_level_walls(7, 5, 24, 5, 25);
  set_level_walls(7, 1, 25, 4, 25);

  set_level_walls(8, 1, 1, 3, 47);
  set_level_walls(8, 4, 1, 23, 8);
  set_level_walls(8, 9, 9, 15, 11);
  set_level_walls(8, 4, 20, 7, 47);
  set_level_walls(8, 9, 13, 15, 47);
  set_level_walls(8, 16, 20, 23, 47);
  set_level_walls(8, 1, 48, 2, 56);
  set_level_walls(8, 21, 48, 23, 59);
  set_level_walls(8, 1, 57, 19, 58);
  set_level_walls(8, 2, 60, 23, 62);
  set_level_walls(8, 2, 63, 3, 76);
  set_level_walls(8, 4, 76, 4, 76);
  set_level_walls(8, 2, 78, 4, 78);
  set_level_walls(8, 7, 65, 8, 78);
  set_level_walls(8, 9, 65, 17, 66);
  set_level_walls(8, 18, 64, 21, 66);
  set_level_walls(8, 11, 71, 23, 72);
  set_level_walls(8, 14, 74, 16, 78);
  set_level_walls(8, 11, 73, 11, 76);

  /* Level entities */
  game_map[0][19][63] = NEXT_LEVEL;
  game_map[1][19][59] = PREVIOUS_LEVEL;
  game_map[1][22][3] = NEXT_LEVEL;
  game_map[2][22][3] = PREVIOUS_LEVEL;
  game_map[2][11][78] = NEXT_LEVEL;
  game_map[3][11][30] = PREVIOUS_LEVEL;
  game_map[3][11][78] = NEXT_LEVEL;
  game_map[4][22][2] = PREVIOUS_LEVEL;
  game_map[4][1][78] = NEXT_LEVEL;
  game_map[5][1][78] = NEXT_LEVEL;
  game_map[5][22][1] = PREVIOUS_LEVEL;
  game_map[6][22][1] = PREVIOUS_LEVEL;
  game_map[6][12][78] = NEXT_LEVEL;
  game_map[7][18][17] = NEXT_LEVEL;
  game_map[8][21][17] = PREVIOUS_LEVEL;
  // game_map[0][19][52] = 298;
  // game_map[0][20][52] = 215;
  // game_map[0][10][40] = 215;
  // game_map[1][8][8] = 215;
  // game_map[1][20][25] = 215;
  // game_map[2][15][15] = 298;

  // must run after custom items creation
  add_randomly_fruits(3);
  add_randomly_monsters(4, 215);

  // Create server socket
  socket_fds[0] = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fds[0] < 0)
    error("ERROR opening socket");

  // Zero socket address and set attributes
  bzero((char*)&socket_addr[0], sizeof(socket_addr[0]));
  socket_addr[0].sin_family = AF_INET;
  socket_addr[0].sin_addr.s_addr = INADDR_ANY;
  // Converting unsigned short integer from host byte order to network byte
  // order.
  socket_addr[0].sin_port = htons(PORT);

  // Assigning address specified by addr to the socket referred by the server
  // socket fd
  if (bind(socket_fds[0], (struct sockaddr*)&socket_addr[0],
           sizeof(socket_addr[0])) < 0)
    error("ERROR on binding");

  // Marking socket to be used to accept incoming connection requests
  listen(socket_fds[0], 5);
  socklen_t clilen = sizeof(socket_addr[0]);

  for (i = 1;; i++) {
    // Accepting an incoming connection request
    socket_fds[i] =
        accept(socket_fds[0], (struct sockaddr*)&socket_addr[i], &clilen);
    if (socket_fds[i] < 0)
      error("ERROR on accept");

    // Reset game if someone won
    if (someone_won) {
      printf("We have a winner!\n");
      someone_won = 0;
    }

    // listen for username and password
    int valread;
    char pass_buffer[1024] = {0};
    valread = read(socket_fds[i], pass_buffer, 1024);
    if (valread < 0)
      error("ERROR reading from socket.");

    user_login usr;
    // break string to a series of tokens
    strcpy(usr.action, strtok(pass_buffer, " "));
    strcpy(usr.name, strtok(NULL, " "));
    strcpy(usr.passwd, strtok(NULL, " "));

    if (strncmp(usr.action, "1", 1) == 0) {
      if (check_user_pass(usr.name, usr.passwd, &active_users)) {
        send(socket_fds[i], "1", strlen("1"), 0);
        make_thread(&gameplay, &socket_fds[i]);
        printf("Pass is ok\n");
      }

      else {
        printf("Not Found\n");
        send(socket_fds[i], "2", strlen("1"), 0);
      }
    } else if (strncmp(usr.action, "2", 1) == 0) {
      add_new_user(usr.name, usr.passwd, &active_users);
      save_to_file(&active_users);
      send(socket_fds[i], "1", strlen("1"), 0);
      make_thread(&gameplay, &socket_fds[i]);
    }
  }

  // Closing the server socket
  close(socket_fds[0]);
  printf("Closing server. Bye...\n");
  return 0;
}
