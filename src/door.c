#include <stdio.h>
#include <stdlib.h>

#include "door.h"
#include "player.h"
#include "engine/engine_context.h"

const object_v_table_t door_vtable = {
    .update = door_entity_update,
    .draw = door_entity_draw,
    .destroy = door_entity_destroy
};

door_t *create_door(Vector2 pos, int id, int texture_id) {
    door_t *door = (door_t *)malloc(sizeof(door_t));
    if (!door) {
        return NULL;
    }

    door->pos = pos;
    door->is_open = 0;
    door->max_height = pos.y - 32;
    door->id = id;
    door->texture_id = texture_id;
    sprintf(door->num, "%d", id);

    return door;
}

void update_door(door_t *cdoor, engine_context_t *engine) {
    if (!cdoor || !engine || !engine->entity_manager) {
        return;
    }

    entity_manager_t *manager = engine->entity_manager;
    if (manager->player_idx < 0) {
        return;
    }

    player_t *player = (player_t*)manager->entities[manager->player_idx]->data;
    if (!player) {
        return;
    }

    if (cdoor->is_open && cdoor->pos.y > cdoor->max_height) {
        cdoor->pos.y -= 0.25f;
    }

    Rectangle player_rect = {
        player->pos.x, player->pos.y, 32, 64
    };

    Rectangle door_rect = {
        cdoor->pos.x, cdoor->pos.y,
        32, 32
    };

    if (CheckCollisionRecs(player_rect, door_rect)) {
        float dx_left = (player_rect.x + player_rect.width) - door_rect.x;
        float dx_right = (door_rect.x + door_rect.width) - player_rect.x;
        float dy_top = (player_rect.y + player_rect.height) - door_rect.y;
        float dy_bottom = (door_rect.y + door_rect.height) - player_rect.y;

        float min_x = (dx_left < dx_right) ? dx_left : dx_right;
        float min_y = (dy_top < dy_bottom) ? dy_top : dy_bottom;

        if (min_x < min_y) {
            if (dx_left < dx_right) {
                player->pos.x = door_rect.x - player_rect.width;
            } else {
                player->pos.x = door_rect.x + door_rect.width;
            }
        } else {
            if (dy_top < dy_bottom) {
                player->pos.y = door_rect.y - player_rect.height;
                player->on_ground = 1;
                player->state = IDLE;
                player->velocity.y = 0;
            } else {
                player->pos.y = door_rect.y + door_rect.height;
                player->velocity.y = 0;
            }
        }
    }
}

door_t* find_door_by_id(engine_context_t *engine, int door_id) {
    if (!engine || !engine->entity_manager) return NULL;
    entity_manager_t *manager = engine->entity_manager;
    for (int i = 0; i < manager->count; i++) {
        entity_t *e = manager->entities[i];
        if (e->vtable == &door_vtable) {
            door_t *d = (door_t*)e->data;
            if (d->id == door_id) return d;
        }
    }
    return NULL;
}

void draw_door(door_t *cdoor, engine_context_t *engine) {
    if (!cdoor || !engine) {
        return;
    }

    DrawTexture(engine->texture_manager.texture[cdoor->texture_id], cdoor->pos.x, cdoor->pos.y, WHITE);
    DrawText(cdoor->num, cdoor->pos.x + 8, cdoor->pos.y, 32, BLACK);
}

void free_door(door_t *cdoor) {
    free(cdoor);
}

void door_entity_update(void *self, engine_context_t *engine) {
    update_door((door_t*)self, engine);
}

void door_entity_draw(void *self, engine_context_t *engine) {
    draw_door((door_t*)self, engine);
}

void door_entity_destroy(void *self) {
    free_door((door_t*)self);
}