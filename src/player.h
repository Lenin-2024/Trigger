#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "raylib.h"
#include "map.h"
#include "engine/engine.h"

struct engine_context;

enum state {
    IDLE = 1,
    GO = 2,
    RUN = 3,
    JUMP = 4
};

struct player {
    int id;
    Vector2 velocity;
    Vector2 pos;
    int current_frame;
    int tile_size;
    int flip;
    int on_ground;
    enum state state;
};
typedef struct player player_t;

player_t *create_player(Vector2 pos, int id);
void update_player(player_t *player, struct engine_context *engine);
void draw_player(player_t player, struct engine_context *engine);
void check_collision_pl(level_config_t *map, player_t *player, int dir);

void player_entity_update(void *self, engine_context_t *engine);
void player_entity_draw(void *self, engine_context_t *engine);
void player_entity_destroy(void *self);

extern const object_v_table_t player_vtable;

#endif //__PLAYER_H__