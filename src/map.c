#include <stdio.h>
#include <stdlib.h>
#include "map.h"

int **get_map(char *file_name) {
    FILE *file = NULL;
    if ((file = fopen(file_name, "r")) == NULL) {
        fprintf(stderr, "[ ERROR ] file \'%s\' don't open\n", file_name);
        return NULL;
    }

    int rows = 0, cols = 0;
    fscanf(file, "%d %d", &rows, &cols);
    printf("[ INFO ] map size ( x = %d y = %d)\n", rows, cols);
    
    int **map = (int **)malloc(rows * sizeof(int *));
    if (map == NULL) {
        fprintf(stderr, "[ ERROR ] memory for rows not allocate\n");
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        map[i] = (int *)malloc(cols * sizeof(int));
        if (map[i] == NULL) {
            fprintf(stderr, "[ ERROR ] memory for cols not allocate\n");
            return NULL;
        }
    }

    int num = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fscanf(file, "%d ", &num);
            map[i][j] = num;
        }
    }

    fclose(file);
    return map;
}