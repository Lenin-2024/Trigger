#include <stdio.h>
#include <stdlib.h>

#include "engien.h"

entity_manager_t *craete_entity_manager(int capacity) {
    entity_manager_t *entity_manager = (entity_manager_t *)malloc(sizeof(entity_manager_t));
    if (!entity_manager) {
        return NULL;
    }

    entity_manager->capacity = capacity;
    entity_manager->count = 0;
    entity_manager->entities = (entity_t **)malloc(entity_manager->capacity * sizeof(entity_t *));
    if (!entity_manager->entities) {
        free(entity_manager);
        return NULL;
    }

    return entity_manager;
}

entity_t *create_entity(entity_manager_t *manager, void *data, const object_v_table_t *vtable) {
    if (!manager || !vtable) {
        return NULL;
    }

    if (manager->count >= manager->capacity) {
        manager->capacity *= 2;
        entity_t **new_entities = (entity_t **)realloc(manager->entities, sizeof(entity_t*) * manager->capacity);
        if (!new_entities) {
            return NULL;
        }
        manager->entities = new_entities;
    }

    entity_t *entity = (entity_t *)malloc(sizeof(entity_t));
    if (entity == NULL) {
        return NULL;
    }

    entity->data = data;
    entity->active = 1;
    entity->vtable = vtable;
    
    manager->entities[manager->count++] = entity;
    return entity;
};

void destroy_entity(entity_manager_t *manager, entity_t *entity) {
    // TODO
}

void update_all_entities(entity_manager_t *manager) {
    if (!manager) {
        return;
    }

    for (int i = 0; i < manager->count; i++) {
        entity_t *entity = manager->entities[i];
        if (entity->vtable->update && entity->active) {
            entity->vtable->update(entity->data);
        }
    }
}

void draw_all_entities(entity_manager_t *manager) {
    if (!manager) return;
    
    for (int i = 0; i < manager->count; i++) {
        entity_t *entity = manager->entities[i];
        if (entity->active && entity->vtable->draw) {
            entity->vtable->draw(entity->data);
        }
    }
}