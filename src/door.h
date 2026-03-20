#ifndef __DOOR_H__
#define __DOOR_H__

#include "raylib.h"
#include "engien/engien.h"

struct game_state;

struct door {
    int is_open;
    int id;
    char num[2];
    Vector2 pos;
    float max_height;
};
typedef struct door door_t;

struct door_entity_data {
    door_t *door;
    entity_manager_t *manager;
};
typedef struct door_entity_data door_entity_data_t;

door_entity_data_t *create_door(entity_manager_t *manager, Vector2 pos, int num);
void update_door(door_t *door, entity_t *player_entity);
void draw_door(door_t *cdoor);
void free_door(door_t *cdoor);
door_t* find_door_by_id(entity_manager_t *manager, int door_id);

extern const object_v_table_t door_vtable;

void door_entity_update(void *data);
void door_entity_draw(void *data);
void door_entity_destroy(void *data);

#endif /* __DOOR_H__ */