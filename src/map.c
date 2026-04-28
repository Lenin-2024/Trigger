#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "player.h"
#include "door.h"
#include "config/config.h"
#include "engine/engine_context.h"

#include "raylib.h"

level_config_t *get_map(engine_context_t *engine, const char *file_name) {
    level_config_t *map = load_level_config(file_name);
    if (!map) {
        return NULL;
    }

    int door_id = 0;
    for (int i = 0; i < map->layout->rows; i++) {
        for (int j = 0; j < map->layout->cols; j++) {
            int tile_id = map->layout->data[i][j];
            if (tile_id == 0) {
                continue;
            }

            for (int k = 0; k < map->count_objects; k++) {
                if (map->objects[k].id == tile_id) {
                    if (strcmp(map->objects[k].name, "player") == 0) {
                        player_t *player = create_player((Vector2){j * 32, i * 32}, tile_id);
                        if (player) {
                            engine->entity_manager->player_idx = engine->entity_manager->count;
                            create_entity(engine->entity_manager, player, &player_vtable);
                        }
                        map->layout->data[i][j] = 0;
                    } else if (strcmp(map->objects[k].name, "door") == 0) {
                        door_t *door = create_door((Vector2){j * 32, i * 32}, door_id++, tile_id);
                        create_entity(engine->entity_manager, door, &door_vtable);
                        map->layout->data[i][j] = 0;
                    }
                    break;
                }
            }
        }
    }

    init_texture_manager(&engine->texture_manager, map);
    return map;
}

void draw_map(engine_context_t *eng, level_config_t *map) {
    (void)eng;
    for (int i = 0; i < map->layout->rows; i++) {
        for (int j = 0; j < map->layout->cols; j++) {
            if (map->layout->data[i][j] == 1) {
                DrawRectangle(j * 32, i * 32, 32, 32, BLUE);
            }
        }
    }
}

void free_map(engine_context_t *eng, level_config_t *map) {
    free_texture_manager(&eng->texture_manager);
    free_level_config(map);
}