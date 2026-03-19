#include <stdio.h>
#include <stdlib.h>

#include "map.h"
#include "game.h"
#include "raylib.h"

map_t *get_map(char *file_name, game_state_t* game) {
    FILE *file = NULL;
    if ((file = fopen(file_name, "r")) == NULL) {
        fprintf(stderr, "[ ERROR ] file \'%s\' don't open\n", file_name);
        return NULL;
    }

    map_t *map = (map_t *)malloc(sizeof(map_t));
    if (map == NULL) {
        fprintf(stderr, "[ ERROR ] memory not allocate for map\n");
        fclose(file);
        return NULL;
    }

    fscanf(file, "%d %d", &map->rows, &map->cols);
    if ((map->rows <= 0) || (map->rows <= 0)) {
        free(map);
        fclose(file);
        return NULL;
    }
    printf("[ INFO ] map size ( x = %d y = %d)\n", map->rows, map->cols);
    
    map->arr = (int **)malloc(map->rows * sizeof(int *));
    if (map->arr == NULL) {
        fprintf(stderr, "[ ERROR ] memory for rows not allocate\n");
        free(map);
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < map->rows; i++) {
        map->arr[i] = (int *)malloc(map->cols * sizeof(int));
        if (map->arr[i] == NULL) {
            fprintf(stderr, "[ ERROR ] memory for cols not allocate\n");
            for (int j = 0; j < i; j++) {
                free(map->arr[j]);
            }
            free(map->arr);
            free(map);
            fclose(file);
            return NULL;
        }
    }

    int num = 0;
    for (int i = 0; i < map->rows; i++) {
        for (int j = 0; j < map->cols; j++) {
            fscanf(file, "%d", &num);
            if (num == 2) {
                player_t *player = create_player((Vector2){j * 32, i * 32});
                if (player) {
                    create_entity(game->entity_manager, player, &player_vtable);
                }
                num = 0;
            }

            if (num == 3) {
                game->count_doors++;
                game->doors = (door_t *)realloc(game->doors, game->count_doors * sizeof(door_t));
                if (game->doors == NULL) {
                    fprintf(stderr, "[ ERROR ] Failed to reallocate memory for doors\n");
                    for (int k = 0; k < map->rows; k++) {
                        free(map->arr[k]);
                    }
                    free(map->arr);
                    free(map);
                    fclose(file);
                    return NULL;
                }

                init_door(game->doors, (Vector2){j * 32, i * 32}, game->count_doors - 1);
                num = 0;
            }

            map->arr[i][j] = num;
        }
    }

    return map;
}

void draw_map(map_t *map) {
    for (int i = 0; i < map->rows; i++) {
        for (int j = 0; j < map->cols; j++) {
            if (map->arr[i][j] == 1) {
                DrawRectangle(j * 32, i * 32, 32, 32, BLUE);
            }
        }
    }
}

void free_map(map_t *map) {
    if (map == NULL) {
        return;
    }

    for (int i = 0; i < map->rows; i++) {
        free(map->arr[i]);
    }
    free(map->arr);
    free(map);
}