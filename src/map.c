#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "config/config.h"
#include "engien/texture_manager.h"
#include "game.h"
#include "raylib.h"

level_config_t *get_map(char *file_name, game_state_t* game) {
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
                            game->entity_manager->player_idx = game->entity_manager->count;
                            create_entity(game->entity_manager, player, &player_vtable);
                        }
                        map->layout->data[i][j] = 0;
                    } else if (strcmp(map->objects[k].name, "door") == 0) {
                        door_entity_data_t *door = create_door(game->entity_manager, 
                            (Vector2){j * 32, i * 32}, door_id++, tile_id);
                        create_entity(game->entity_manager, door, &door_vtable);
                        map->layout->data[i][j] = 0;
                    }
                    break;
                }
            }
        }
    }

    init_texture_manager(map);
    return map;
}

void draw_map(level_config_t *map) {
    for (int i = 0; i < map->layout->rows; i++) {
        for (int j = 0; j < map->layout->cols; j++) {
            if (map->layout->data[i][j] == 1) {
                DrawRectangle(j * 32, i * 32, 32, 32, BLUE);
            }
        }
    }
}

void free_map(level_config_t *map) {
    free_texture_manager();
    free_level_config(map);
}