/*
 * Multiplayer Snakes game - Client
 * Luke Collins
 */

#include <ncurses.h>
#include <netdb.h>
#include <time.h>

#include "skotos.h"

#define LOCALHOST "127.0.0.1"

#define REFRESH 0.125
#define ONGOING -34
#define INTERRUPTED -30

#define PLAYER_CHAR '@'
#define SPIDER_CHAR 'X'
#define GOBLIN_CHAR 'G'
#define TROLL_CHAR 'T'

typedef struct {
  char name[25];
  char passwd[25];
  char action_choice[2];
} user_data;

user_data user_data_input() {
  user_data new_user;
  //   system("clear");
  initscr();
  echo();
  mvprintw((HEIGHT - 20) / 2 + 10, (WIDTH - 58) / 2, "Enter the user name :");
  wgetnstr(stdscr, new_user.name, sizeof(new_user.name));
  mvprintw((HEIGHT - 20) / 2 + 12, (WIDTH - 58) / 2, "Enter the password :");
  wgetnstr(stdscr, new_user.passwd, sizeof(new_user.passwd));
  noecho();
  return new_user;
}

WINDOW* win;
char key = STAT_KEY;
int game_result = ONGOING;

// Output error message and exit cleanly
void error(const char* msg) {
  perror(msg);
  exit(0);
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

void* write_to_server(void* arg) {
  int sockfd = *(int*)arg;
  struct timespec ts;
  ts.tv_sec = REFRESH;
  ts.tv_nsec = ((int)(REFRESH * 1000) % 1000) * 1000000;
  while (game_result == ONGOING) {
    nanosleep(&ts, NULL);
    int n = write(sockfd, &key, 1);
    if (n < 0)
      error("ERROR writing to socket.");
  }
  return 0;
}

void* update_screen(void* arg) {
  int sockfd = *(int*)arg;
  int bytes_read;
  int game_map[HEIGHT][WIDTH];
  int map_size = HEIGHT * WIDTH * sizeof(game_map[0][0]);
  char map_buffer[map_size];
  int i, j, n;

  while (game_result == ONGOING) {
    // Receive updated map from server
    bytes_read = 0;
    bzero(map_buffer, map_size);
    while (bytes_read < map_size) {
      n = read(sockfd, map_buffer + bytes_read, map_size - bytes_read);
      if (n <= 0)
        goto end;
      bytes_read += n;
    }
    memcpy(game_map, map_buffer, map_size);

    clear();
    // box(win, '+', '+');
    refresh();
    wrefresh(win);

    // paint map
    for (i = 0; i < HEIGHT; i++) {
      for (j = 0; j < WIDTH; j++) {
        int current = game_map[i][j];
        int player_colour = abs(current) % 7;

        if ((current > GOBLIN) && (current < GOBLIN + 100)) {
          attron(COLOR_PAIR(9));
          mvprintw(i, j, "G");
          attroff(COLOR_PAIR(9));

        } else if ((current > SPIDER) && (current < SPIDER + 100)) {
          attron(COLOR_PAIR(6));
          mvprintw(i, j, "X");
          attroff(COLOR_PAIR(6));
        } else if (current == NEXT_LEVEL) {
          attron(COLOR_PAIR(7));
          mvprintw(i, j, ">");
          attroff(COLOR_PAIR(7));
        } else if (current == PREVIOUS_LEVEL) {
          attron(COLOR_PAIR(7));
          mvprintw(i, j, "<");
          attroff(COLOR_PAIR(7));
        } else if ((current > 0) && (current != FRUIT)) {
          mvprintw(i, j, " ");
        } else if ((current < 0) && (current > -7)) {
          attron(COLOR_PAIR(player_colour));
          mvprintw(i, j, "@");
          attroff(COLOR_PAIR(player_colour));
        }

        else if (current == BORDER) {
          attron(COLOR_PAIR(8));
          mvprintw(i, j, " ");
          attroff(COLOR_PAIR(8));
        }

        else if (current == FRUIT) {
          attron(COLOR_PAIR(10));
          mvprintw(i, j, "O");
          attroff(COLOR_PAIR(10));
        }
      }
    }
    // mvprintw(24, 0, "Testing");
    refresh();
  }

end:
  game_result = game_map[0][0];
  return 0;
}

void first_screen() {
  system("clear");
  mvprintw((HEIGHT - 20) / 2, (WIDTH - 58) / 2,
           "  _____ _   _______ _____ _____ _____ ");
  mvprintw((HEIGHT - 20) / 2 + 1, (WIDTH - 58) / 2,
           " /  ___| | / /  _  |_   _|  _  /  ___|");
  mvprintw((HEIGHT - 20) / 2 + 2, (WIDTH - 58) / 2,
           " \\ `--.| |/ /| | | | | | | | | \\ `--. ");
  mvprintw((HEIGHT - 20) / 2 + 3, (WIDTH - 58) / 2,
           "  `--. \\    \\| | | | | | | | | |`--. \"");
  mvprintw((HEIGHT - 20) / 2 + 4, (WIDTH - 58) / 2,
           " /\\__/ / |\\  \\ \\_/ / | | \\ \\_/ /\\__/ /");
  mvprintw((HEIGHT - 20) / 2 + 5, (WIDTH - 58) / 2,
           " \\____/\\_| \\_/\\___/  \\_/  \\___/\\____/ v1.0  /\\");
  mvprintw((HEIGHT - 20) / 2 + 6, (WIDTH - 58) / 2,
           "___________________________________________/ /");
  mvprintw((HEIGHT - 20) / 2 + 7, (WIDTH - 58) / 2,
           "\\___________________________________________/");
  mvprintw((HEIGHT - 20) / 2 + 10, (WIDTH - 58) / 2, " Instructions:");
  mvprintw((HEIGHT - 20) / 2 + 12, (WIDTH - 58) / 2,
           " - Use the keys w, a, s, d to move.");
  mvprintw((HEIGHT - 20) / 2 + 13, (WIDTH - 58) / 2,
           " - Eat fruit, slay monsters.");
  mvprintw((HEIGHT - 20) / 2 + 15, (WIDTH - 58) / 2, " Commands:");
  mvprintw((HEIGHT - 20) / 2 + 17, (WIDTH - 58) / 2, "[l] to login");
  mvprintw((HEIGHT - 20) / 2 + 18, (WIDTH - 58) / 2, "[c] to create new user");
  mvprintw((HEIGHT - 20) / 2 + 20, (WIDTH - 58) / 2,
           "Press any key to start . . .");
  // getch();
}

int main(int argc, char* argv[]) {
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent* server;
  char key_buffer;

  // Create Ncurses Window, with input, no echo and hidden cursor
  initscr();
  cbreak();
  noecho();
  start_color();
  use_default_colors();
  curs_set(0);

  // Set window to new ncurses window
  win = newwin(HEIGHT, WIDTH, 0, 0);

  // WHITE, BLACK, BLUE, RED, GREEN, YELLOW, MAGENTA, CYAN
  // Player colours
  init_pair(0, COLOR_GREEN, COLOR_BLACK);
  init_pair(1, COLOR_BLUE, COLOR_BLACK);
  init_pair(2, COLOR_RED, COLOR_BLACK);
  init_pair(3, COLOR_YELLOW, COLOR_BLACK);
  init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(5, COLOR_CYAN, COLOR_BLACK);
  init_pair(6, COLOR_WHITE, COLOR_BLUE);
  // BUILDING
  init_pair(7, COLOR_WHITE, COLOR_BLACK);
  init_pair(8, COLOR_BLACK, COLOR_WHITE);
  // monster
  init_pair(9, COLOR_WHITE, COLOR_RED);
  // FRUITS
  init_pair(10, COLOR_GREEN, COLOR_BLACK);

  first_screen();
  char choice = getch();
  choice = toupper(choice);
  user_data user_to_login;
  if (choice == LOGIN_KEY) {
    system("clear");
    mvprintw((HEIGHT - 20) / 2 + 5, (WIDTH - 58) / 2, "user login");
    user_to_login = user_data_input();
    // user_to_login.action_choice = '1';
    strcpy(user_to_login.action_choice, "1");
  } else if (choice == CREATE_USER_KEY) {
    system("clear");
    mvprintw((HEIGHT - 20) / 2 + 5, (WIDTH - 58) / 2, "creating new user");
    user_to_login = user_data_input();
    // user_to_login.action_choice = '2';
    strcpy(user_to_login.action_choice, "2");
  } else {
    system("clear");
    mvprintw((HEIGHT - 20) / 2 + 15, (WIDTH - 58) / 2,
             " invalid command, exiting...");
    getch();
    echo();
    curs_set(1);
    endwin();
    return 0;
  }

  // Getting socket descriptor
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  // if no server name use default localhost
  if (argc < 2)
    server = gethostbyname(LOCALHOST);
  else
    server = gethostbyname(argv[1]);
  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host.\n");
    exit(0);
  }

  // Sets first n bytes of the area to zero
  bzero((char*)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr,
        server->h_length);

  // Converting unsigned short integer from host byte order to network byte
  // order.
  serv_addr.sin_port = htons(PORT);

  // Attempt connection with server
  if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR connecting");
  // getch();

  char ldata[1024];
  char login = 'A';
  int valread;
  strcpy(ldata, user_to_login.action_choice);
  strcat(ldata, " ");
  strcat(ldata, user_to_login.name);
  strcat(ldata, " ");
  strcat(ldata, user_to_login.passwd);
  // send login ("1 username password")string to server
  send(sockfd, ldata, strlen(ldata), 0);
  valread = read(sockfd, &login, 1);
  if (valread < 0)
    error("ERROR reading from socket.");
  // mvprintw((HEIGHT - 20) / 2 + 5, (WIDTH - 58) / 2, "after conn %d",
  // valread); getch(); Start writing inputs to the server every REFRESH seconds
  // and updating the screen
  if (login == '1') {
    make_thread(update_screen, &sockfd);
    make_thread(write_to_server, &sockfd);
  } else {
    echo();
    curs_set(1);
    endwin();

    // Close connection
    close(sockfd);
    return 0;
  }

  while (game_result == ONGOING) {
    // Get player input with time out
    bzero(&key_buffer, 1);
    timeout(REFRESH * 1000);
    key_buffer = getch();
    key_buffer = toupper(key_buffer);
    if (key_buffer == QUIT_KEY) {
      game_result = INTERRUPTED;
      break;
    } else if ((key_buffer == UP_KEY) || (key_buffer == DOWN_KEY) ||
               (key_buffer == LEFT_KEY) || (key_buffer == RIGHT_KEY) ||
               (key_buffer == STAT_KEY))
      key = key_buffer;
    else {
      key = STAT_KEY;
    }
  }

  // Show the user who won
  WINDOW* announcement = newwin(7, 35, (HEIGHT - 7) / 2, (WIDTH - 35) / 2);
  box(announcement, 0, 0);

  if (game_result == WINNER) {
    mvwaddstr(announcement, 2, (35 - 21) / 2, "Game Over - You WIN!");
    mvwaddstr(announcement, 4, (35 - 21) / 2, "Press any key to quit.");
    wbkgd(announcement, COLOR_PAIR(2));
  } else {
    sleep(1);
    mvwaddstr(announcement, 2, (35 - 21) / 2, "Game Over - you lose!");
    if (game_result > 0)
      mvwprintw(announcement, 3, (35 - 13) / 2, "Player %d won.", game_result);
    mvwaddstr(announcement, 4, (35 - 21) / 2, "Press any key to quit.");
    wbkgd(announcement, COLOR_PAIR(1));
    // sleep(5);
  }

  mvwin(announcement, (HEIGHT - 7) / 2, (WIDTH - 35) / 2);
  wnoutrefresh(announcement);
  wrefresh(announcement);
  sleep(2);
  wgetch(announcement);
  delwin(announcement);
  wclear(win);

  echo();
  curs_set(1);
  endwin();

  // Close connection
  close(sockfd);
  return 0;
}
