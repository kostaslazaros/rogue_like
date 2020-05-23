#include "roguelike.h"
#include "utils.h"

void add_position_xy(int** frontier, int front_counter, int y, int x) {
  frontier[front_counter][0] = y;
  frontier[front_counter][1] = x;
}

int add_neighbors(int** frontier,
                  int front_counter,
                  int*** came_from,
                  int y,
                  int x) {
  // north position
  if (y > 0 && came_from[y - 1][x][0] < 0) {
    add_position_xy(frontier, front_counter, y - 1, x);
    front_counter++;
    came_from[y - 1][x][0] = y;
    came_from[y - 1][x][1] = x;
  }
  // south position
  if (y < (ROWS - 1) && came_from[y + 1][x][0] < 0) {
    add_position_xy(frontier, front_counter, y + 1, x);
    front_counter++;
    came_from[y + 1][x][0] = y;
    came_from[y + 1][x][1] = x;
  }
  // east position
  if (x < (COLUMNS - 1) && came_from[y][x + 1][0] < 0) {
    add_position_xy(frontier, front_counter, y, x + 1);
    front_counter++;
    came_from[y][x + 1][0] = y;
    came_from[y][x + 1][1] = x;
  }
  // west position
  if (x > 0 && came_from[y][x - 1][0] < 0) {
    add_position_xy(frontier, front_counter, y, x - 1);
    front_counter++;
    came_from[y][x - 1][0] = y;
    came_from[y][x - 1][1] = x;
  }
  return front_counter;
}

void find_path(Position* start, Position* end) {
  int i, j;
  int x, y;
  int front_index = 0;
  int front_counter = 0;
  int** frontier = malloc(sizeof(int*) * ROWS * COLUMNS);
  int*** came_from = malloc(sizeof(int**) * ROWS);

  /* came from starts with two dimentions of a point in order to see where this
  point started from. That means that it gets access to another dimension.
   */

  for (i = 0; i < ROWS * COLUMNS; i++) {
    frontier[i] = malloc(sizeof(int) * 2);
  }

  for (i = 0; i < ROWS; i++) {
    came_from[i] = malloc(sizeof(int*) * COLUMNS);
    for (j = 0; j < COLUMNS; j++) {
      came_from[i][j] = malloc(sizeof(int) * 2);
      came_from[i][j][0] = -1;
      came_from[i][j][1] = -1;
    }
  }
  came_from[start->y][start->x][0] = -9;
  came_from[start->y][start->x][1] = 9;

  add_position_xy(frontier, front_counter, start->y, start->x);
  front_counter++;

  while (front_index < front_counter) {
    y = frontier[front_index][0];
    x = frontier[front_index][1];
    front_index++;
    if (y == end->y && x == end->x) {
      break;
    }
    front_counter = add_neighbors(frontier, front_counter, came_from, y, x);
  }
  y == end->y;
  x == end->x;

  while (y != start->y || x != start->x) {
    y = came_from[y][x][0];
    x = came_from[y][x][1];
    mvprintw(y, x, "+");
    getch();
  }
}
