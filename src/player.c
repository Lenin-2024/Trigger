#include <stdio.h>
#include <stdlib.h>

#include "player.h"

void init_player(player_t *player, Vector2 pos) {
    player->pos = pos;
    player->tile_size = 16;
}

void update_player(player_t *player) {

}

void draw_player(player_t player) {
    DrawRectangle(player.pos.x, player.pos.y, player.tile_size, player.tile_size, RED);
}