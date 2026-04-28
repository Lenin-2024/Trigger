#ifndef __DOOR_H__
#define __DOOR_H__

#include "raylib.h"
#include "engine/engine.h"

struct engine_context;

struct door {
    int is_open;
    int id;
    int texture_id;
    char num[8];
    Vector2 pos;
    float max_height;
};
typedef struct door door_t;

door_t *create_door(Vector2 pos, int id, int texture_id);
void update_door(door_t *cdoor, struct engine_context *engine);
void draw_door(door_t *cdoor, struct engine_context *engine);
void free_door(door_t *cdoor);
door_t* find_door_by_id(struct engine_context *engine, int door_id);

extern const object_v_table_t door_vtable;

void door_entity_update(void *self, struct engine_context *engine);
void door_entity_draw(void *self, struct engine_context *engine);
void door_entity_destroy(void *self);

#endif /* __DOOR_H__ */