#ifndef __DOOR_H__
#define __DOOR_H__

#include "raylib.h"

struct door {
    int is_open;
    Vector2 pos;
    float max_height;
};
typedef struct door door_t;

void init_door(door_t *cdoor, Vector2 pos);
void update_door(door_t *cdoor);
void draw_door(door_t *cdoor);
void free_door(door_t *cdoor);

#endif /* __DOOR_H__ */