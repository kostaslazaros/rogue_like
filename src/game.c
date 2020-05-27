#include "roguelike.h"

void render(Game* game) {
  clear();
  game_hub_print(game->p_levels[game->current_level - 1]);
  level_draw(game->p_levels[game->current_level - 1]);
}

int game_loop(Game* game) {
  int ch = '\0';
  Position* p_pos_new;
  Level* p_level;
  if (game->current_level == 0) {
    game->p_levels[game->current_level] = create_level(1);
    game->current_level++;
  }
  p_level = game->p_levels[game->current_level - 1];

  /*main game loop*/
  while (ch != 'q') {
    p_pos_new = handleinput(ch, p_level->p_player);
    pos_check(p_pos_new, p_level);
    monster_move(p_level);
    render(game);

    if (p_level->p_player->health <= 0) {
      game->current_level = 0;
      return -1;
    }
    ch = getch();
  }
}