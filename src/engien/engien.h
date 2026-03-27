#ifndef __ENGIEN_H__
#define __ENGIEN_H__

#include "raylib.h"

struct object_v_table {
    void (*update)(void *self);
    void (*draw)(void *self);
    void (*destroy)(void *self);
};
typedef struct object_v_table object_v_table_t;

struct entity {
    void *data;
    const object_v_table_t *vtable;
    int active;
};
typedef struct entity entity_t;

typedef struct {
    entity_t **entities;
    int count;
    int capacity;
    int player_idx;
} entity_manager_t;

entity_manager_t *craete_entity_manager(int capacity);
entity_t *create_entity(entity_manager_t *manager, void *data, const object_v_table_t *vtable);
void destroy_entity(entity_manager_t *manager, entity_t *entity);
void update_all_entities(entity_manager_t *manager);
void draw_all_entities(entity_manager_t *manager);

#endif /* __ENGIEN_H__ */