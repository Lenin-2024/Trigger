#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "raylib.h"
#include "map.h"

enum state {
    IDLE = 1,
    GO = 2,
    RUN = 3,
};

extern Texture2D player_texture;

struct player {
    Vector2 velocity;
    Vector2 pos;
    int current_frame;
    int tile_size;
    int flip;
    enum state state;
};
typedef struct player player_t;

void init_player(player_t *player, Vector2 pos);
void update_player(player_t *player, map_t *map);
void draw_player(player_t player);
void check_collision_pl(map_t *map, player_t *player, int fir);

#endif //__PLAYER_H__