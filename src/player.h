#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "raylib.h"

struct player {
    Vector2 pos;
    int tile_size;
};
typedef struct player player_t;

void init_player(player_t *player, Vector2 pos);
void update_player(player_t *player);
void draw_player(player_t player);

#endif //__PLAYER_H__