#ifndef __DOOR_H__
#define __DOOR_H__

#include "raylib.h"

struct game_state;

struct door {
    int is_open;
    char num[2];
    Vector2 pos;
    float max_height;
};
typedef struct door door_t;

void init_door(door_t *cdoor, Vector2 pos, int num);
void update_door(struct game_state *game, int num);
void draw_door(door_t *cdoor);
void free_door(door_t *cdoor);

#endif /* __DOOR_H__ */