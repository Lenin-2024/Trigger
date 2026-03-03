#include <stdio.h>
#include <stdlib.h>
#include "map.h"

map_t *get_map(char *file_name) {
    FILE *file = NULL;
    if ((file = fopen(file_name, "r")) == NULL) {
        fprintf(stderr, "[ ERROR ] file \'%s\' don't open\n", file_name);
        return NULL;
    }

    map_t *map = (map_t *)malloc(sizeof(map_t));
    if (map == NULL) {
        fprintf(stderr, "[ ERROR ] memory not allocate for map\n");
        return NULL;
    }

    fscanf(file, "%d %d", &map->rows, &map->cols);
    printf("[ INFO ] map size ( x = %d y = %d)\n", map->rows, map->cols);
    
    map->arr = (int **)malloc(map->rows * sizeof(int *));
    if (map->arr == NULL) {
        fprintf(stderr, "[ ERROR ] memory for rows not allocate\n");
        return NULL;
    }

    for (int i = 0; i < map->rows; i++) {
        map->arr[i] = (int *)malloc(map->cols * sizeof(int));
        if (map->arr[i] == NULL) {
            fprintf(stderr, "[ ERROR ] memory for cols not allocate\n");
            return NULL;
        }
    }

    int num = 0;
    for (int i = 0; i < map->rows; i++) {
        for (int j = 0; j < map->cols; j++) {
            fscanf(file, "%d ", &num);
            map->arr[i][j] = num;
        }
    }

    fclose(file);
    return map;
}