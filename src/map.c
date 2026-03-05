#include <stdio.h>
#include <stdlib.h>

#include "map.h"
#include "raylib.h"

map_t *get_map(char *file_name) {
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
            map->arr[i][j] = num;
        }
    }

    return map;
}

void draw_map(map_t *map) {
    for (int i = 0; i < map->rows; i++) {
        for (int j = 0; j < map->rows; j++) {
            if (map->arr[i][j] == 1) {
                DrawRectangle(i * 32, j * 32, 32, 32, BLUE);
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