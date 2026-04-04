#include <stdio.h>
#include <stdlib.h>

#include "door.h"
#include "player.h"
#include "engien/engien.h"

const object_v_table_t door_vtable = {
    .update = door_entity_update,
    .draw = door_entity_draw,
    .destroy = door_entity_destroy
};

door_entity_data_t *create_door(entity_manager_t *manager, Vector2 pos, int id, int num) {
    door_entity_data_t *door_entity = (door_entity_data_t *)malloc(sizeof(door_entity_data_t));
    if (!door_entity) {
        return NULL;
    }
    
    door_t *door = (door_t *)malloc(sizeof(door_t));
    if (!door) {
        free(door_entity);
        return NULL;
    }

    door->pos = pos;
    door->is_open = 0;
    door->max_height = pos.y - 32;
    door->id = id;
    door->fi_num = num;
    sprintf(door->num, "%d", num);

    door_entity->door = door;
    door_entity->manager = manager;

    return door_entity;
}

void update_door(door_t *door, entity_t *player_entity) {
    player_t *player = (player_t *)player_entity->data;
    if (door->is_open && (door->pos.y > door->max_height)) {
        door->pos.y -= 0.25f;
    }

    Rectangle player_rect = {
        player->pos.x, player->pos.y, 32, 64
    };

    Rectangle door_rect = {
        door->pos.x, door->pos.y,
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

door_t* find_door_by_id(entity_manager_t *manager, int door_id) {
    for (int i = 0; i < manager->count; i++) {
        entity_t *entity = manager->entities[i];
        // Проверяем, что это дверь (сравниваем vtable)
        if (entity->vtable == &door_vtable) {
            door_entity_data_t *door_data = (door_entity_data_t *)entity->data;
            if (door_data->door->id == door_id) {
                return door_data->door;
            }
        }
    }
    return NULL;
}

void draw_door(door_t *cdoor) {
    DrawTexture(g_texture_manager.texture[cdoor->id], cdoor->pos.x, cdoor->pos.y, WHITE);
    DrawText(cdoor->num, cdoor->pos.x + 8, cdoor->pos.y, 32, BLACK);
}

void free_door(door_t *cdoor) {
    if (cdoor) {
        free(cdoor);
    }
}

void door_entity_update(void *data) {
    door_entity_data_t *door_data = (door_entity_data_t *)data;
    update_door(door_data->door, door_data->manager->entities[door_data->manager->player_idx]);
}

void door_entity_draw(void *data) {
    door_entity_data_t *door_data = (door_entity_data_t *)data;
    draw_door(door_data->door);
}

void door_entity_destroy(void *data) {
    door_entity_data_t *door_data = (door_entity_data_t *)data;
    free_door(door_data->door);
}