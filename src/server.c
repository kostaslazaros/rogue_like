/******************************** 
* A rogue like multiplayer game * 
*            Made by:           *
*       Nefeli Stefanatou       *
*       Konstantinos Lazaros    *
*********************************/ 

#include <signal.h>
#include "skotos.h"
#include "users.h"
#define MAP_FILE "game_map.dat"

#define MAX_PLAYERS 1024
#define WINNER_LENGTH 15

// Game map
int game_map[LEVELS][HEIGHT][WIDTH];
int map_size = LEVELS * HEIGHT * WIDTH * sizeof(game_map[0][0][0]);
int client_map_size = HEIGHT * WIDTH * sizeof(game_map[0][0][0]);
pthread_mutex_t map_lock = PTHREAD_MUTEX_INITIALIZER;
int someone_won = 0;
int number_of_players = 0;

// Direction key types
typedef enum {
  UP = UP_KEY,
  DOWN = DOWN_KEY,
  LEFT = LEFT_KEY,
  RIGHT = RIGHT_KEY,
  STATIC = STAT_KEY
} direction;


void load_level(char* filename, int level) {
  FILE* myFile;
  myFile = fopen(filename, "r");

  // read file into array
  int i, j;

  for (i = 0; i < HEIGHT; i++)
    for (j = 0; j < WIDTH; j++)
      fscanf(myFile, "%d", &game_map[level][i][j]);
  fclose(myFile);
}

int save_map() {
  FILE* outfile;
  outfile = fopen(MAP_FILE, "w");
  if (outfile == NULL) {
    fprintf(stderr, "\nError opening file\n");
    exit(1);
  }

  fwrite(&game_map, sizeof(game_map), 1, outfile);

  fclose(outfile);
  return 0;
}

void load_map() {
  FILE* infile;
  infile = fopen(MAP_FILE, "r");
  if (infile == NULL) {
    fprintf(stderr, "\nError opening file\n");
    exit(1);
  }
  fread(&game_map, sizeof(game_map), 1, infile);
  fclose(infile);
}
// Function to create a player
player* make_player(int player_no) {
  int head_y, head_x;
  srand(time(NULL));
  do {
    head_y = rand() % (HEIGHT - 6) + 3;
    head_x = rand() % (WIDTH - 6) + 3;
  } while (!(game_map[0][head_y][head_x] == 0));
  // Place the player on the map (matrix)

  pthread_mutex_lock(&map_lock);
  game_map[0][head_y][head_x] = -player_no;
  pthread_mutex_unlock(&map_lock);

  // Create player struct, set coordinates facing up
  player* p_player = malloc(sizeof(player));

  p_player->player_no = player_no;
  p_player->level = 0;
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
  save_map();
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
  } else if ((next_move_val > SPIDER) && (next_move_val < SPIDER + 100)) {
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

// game logic is here
void* gameplay(void* arg) {
  int fd = *(int*)arg;
  int player_no = fd - 3;


  printf("Connected player: %d!\n", player_no);
  player* p_player = make_player(player_no);

  // User input
  char key = STATIC;
  char key_buffer = STATIC;
  // char map_buffer[map_size];
  display_data current_map_user;
  int bytes_sent, n;
  int success = 1;

  while (success) {
    // Check if someone won
    if (someone_won)
      success = 0;

    // Check if you are the winner
    if (p_player->level == 9) {
      someone_won = player_no;
      pthread_mutex_lock(&map_lock);
      for (int lvl = 0; lvl < LEVELS; lvl++ )
        game_map[lvl][0][0] = WINNER;
      pthread_mutex_unlock(&map_lock);
    } else if (game_map[0][0][0] != BORDER) {
      pthread_mutex_lock(&map_lock);
      for (int lvl = 0; lvl < LEVELS; lvl++ )
        game_map[lvl][0][0] = someone_won;
      pthread_mutex_unlock(&map_lock);
    } else if (p_player->health <= 0) {
      success = 0;
    }

    memcpy(current_map_user.level_map, game_map[p_player->level],
           client_map_size);
    current_map_user.current_player.level = p_player->level;
    current_map_user.current_player.health = p_player->health;
    current_map_user.current_player.player_no = p_player->player_no;
    bytes_sent = 0;
    while (bytes_sent < sizeof(current_map_user)) {
      bytes_sent += write(fd, &current_map_user, sizeof(current_map_user));
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
        break;
      }

      default:
        break;
    }
  }

  if (p_player->level == 9) {
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

  load_level("level0.txt", 0);
  load_level("level1.txt", 1);
  load_level("level2.txt", 2);
  load_level("level3.txt", 3);
  load_level("level4.txt", 4);
  load_level("level5.txt", 5);
  load_level("level6.txt", 6);
  load_level("level7.txt", 7);
  load_level("level8.txt", 8);
  load_level("level9.txt", 9);

  // must run after custom items creation
  add_randomly_fruits(6);
  add_randomly_monsters(4, 215);
  add_randomly_monsters(2, 298);
  add_randomly_monsters(2, 115);
  // load_map();
  save_map();

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
      // make_thread(&gameplay, &socket_fds[i]);
    }
    make_thread(&gameplay, &socket_fds[i]);
  }
  
  // Closing the server socket
  close(socket_fds[0]);
  printf("Closing server. Bye...\n");
  return 0;
}
