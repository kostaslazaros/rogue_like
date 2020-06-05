/*
 * Multiplayer Snakes game - Server
 * Luke Collins
 */
#include <ctype.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 7070
#define MAX_PLAYERS 1024
#define LEVELS 10
#define HEIGHT 24
#define WIDTH 80
#define MAX_SNAKE_LENGTH HEIGHT* WIDTH
#define WINNER_LENGTH 15
#define FRUIT -1024
#define SPIDER 111
#define GOBLIN 200
#define BORDER -99
#define NEXT_LEVEL 24
#define PREVIOUS_LEVEL 23
#define WINNER -94
#define UP_KEY 'W'
#define DOWN_KEY 'S'
#define LEFT_KEY 'A'
#define RIGHT_KEY 'D'
#define STAT_KEY 'Q'

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
  direction d;
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
  p_player->head.d = STATIC;

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

int collision_detection(int next_y, int next_x, player* p_player) {
  printf("Value of health %d\n", p_player->health);
  int next_move_val = game_map[p_player->level][next_y][next_x];
  if (next_move_val == NEXT_LEVEL) {
    p_player->level += 1;
    return 2;
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
  int old_x, old_y, prev_state, old_level, col_det, new_x, new_y;
  old_x = p_player->head.x;
  old_y = p_player->head.y;
  old_level = p_player->level;
  prev_state = p_player->prev_state;

  if (d == STATIC)
    return;

  switch (d) {
    case UP: {
      new_x = old_x;
      new_y = old_y - 1;
      p_player->head.d = UP;
      break;
    }
    case DOWN: {
      new_x = old_x;
      new_y = old_y + 1;
      p_player->head.d = DOWN;
      break;
    }
    case LEFT: {
      new_x = old_x - 1;
      new_y = old_y;
      p_player->head.d = LEFT;
      break;
    }
    case RIGHT: {
      new_x = old_x + 1;
      new_y = old_y;
      p_player->head.d = RIGHT;
      break;
    }
    case STATIC: {
      new_x = old_x;
      new_y = old_y;
      p_player->head.d = STATIC;
      break;
    }
    default:
      break;
  }
  col_det = collision_detection(new_y, new_x, p_player);
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
void add_fruit() {
  int x, y;
  do {
    y = rand() % (HEIGHT - 6) + 3;
    x = rand() % (WIDTH - 6) + 3;
  } while (game_map[0][y][x] != 0);
  pthread_mutex_lock(&map_lock);
  game_map[0][y][x] = FRUIT;
  pthread_mutex_unlock(&map_lock);
}

void add_fruit_directly(int level, int pos_x, int pos_y) {
  pthread_mutex_lock(&map_lock);
  game_map[level][pos_y][pos_x] = FRUIT;
  pthread_mutex_unlock(&map_lock);
}

// Function for a player to eat a fruit in front of it
void eat_fruit(player* p_player, direction d) {
  switch (d) {
    case UP: {
      p_player->head.y = p_player->head.y - 1;
      p_player->head.d = UP;
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
      p_player->head.d = DOWN;
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
      p_player->head.d = LEFT;
      break;
    }
    case RIGHT: {
      p_player->head.x = p_player->head.x + 1;
      p_player->head.d = RIGHT;
      break;
    }
    default:
      break;
  }
  pthread_mutex_lock(&map_lock);
  game_map[p_player->level][p_player->head.y][p_player->head.x] =
      -(p_player->player_no);
  //   game_map[p_player->body_segment[0].y][p_player->body_segment[0].x] =
  //   p_player->player_no;
  pthread_mutex_unlock(&map_lock);
  p_player->length++;
  add_fruit();
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

// Thread gameplay function
void* gameplay(void* arg) {
  // Determine player number from file descriptor argument
  int fd = *(int*)arg;
  int player_no = fd - 3;
  printf("Player number %d!\n", player_no);

  // Find three consecutive zeros in map for starting player position
  int head_y, head_x;
  srand(time(NULL));
  do {
    head_y = rand() % (HEIGHT - 6) + 3;
    head_x = rand() % (WIDTH - 6) + 3;
  } while (
      !(((game_map[0][head_y][head_x] == game_map[0][head_y + 1][head_x]) ==
         game_map[0][head_y + 2][head_x]) == 0));

  // Create player structure
  player* p_player = make_player(player_no, 0, head_y, head_x);

  // Variables for user input
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
  int i, l;

  // Handle Ctrl+C
  signal(SIGINT, ctrl_c_handler);

  // Fill gamestate matrix with zeros
  memset(game_map, 0, map_size);

  // Set game borders
  for (l = 0; l < LEVELS; l++)
    for (i = 0; i < HEIGHT; i++)
      game_map[l][i][0] = game_map[l][i][WIDTH - 1] = BORDER;
  for (l = 0; l < LEVELS; l++)
    for (i = 0; i < WIDTH; i++)
      game_map[l][0][i] = game_map[l][HEIGHT - 1][i] = BORDER;

  // Randomly add five fruit
  srand(time(NULL));
  //   for (i = 0; i < 3; i++)
  //     add_fruit();
  add_fruit_directly(0, 4, 5);
  add_fruit_directly(0, 4, 10);
  add_fruit_directly(0, 10, 10);
  add_fruit_directly(0, 20, 20);
  game_map[0][10][2] = BORDER;
  game_map[0][10][3] = BORDER;
  game_map[0][10][4] = BORDER;
  game_map[0][10][5] = BORDER;
  game_map[0][12][2] = BORDER;
  game_map[0][12][3] = BORDER;
  game_map[0][12][4] = BORDER;
  game_map[0][12][5] = BORDER;
  game_map[0][12][12] = 298;
  game_map[0][12][13] = 298;
  game_map[0][12][14] = 298;
  game_map[0][18][18] = NEXT_LEVEL;
  game_map[1][20][20] = PREVIOUS_LEVEL;
  game_map[1][8][8] = 215;
  game_map[1][20][25] = 215;
  game_map[1][10][2] = BORDER;
  game_map[1][10][3] = BORDER;
  game_map[1][22][3] = NEXT_LEVEL;
  game_map[2][15][15] = 298;
  game_map[2][22][3] = PREVIOUS_LEVEL;

  // Create server socket
  socket_fds[0] = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fds[0] < 0)
    error("ERROR opening socket");

  // Set socket address to zero and set attributes
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

  // Marking socket as a socket that will be used to accept incoming connection
  // requests
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

    make_thread(&gameplay, &socket_fds[i]);
  }

  // Closing the server socket
  close(socket_fds[0]);
  return 0;
}