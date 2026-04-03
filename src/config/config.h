#include <stdio.h>
#include <stdlib.h>

struct object_config {
    int id;
    char name[256];
    char texture[256];
    char entity[256];
};
typedef struct object_config object_config_t;

struct map_layout {
    int rows;
    int cols;
    int **data;
};
typedef struct map_layout map_layout_t;

struct level_config {
    char name[256];
    char next_level[256];
    char texture_background[256];
    object_config_t *objects;
    int count_objects;
    map_layout_t *layout;
};
typedef struct level_config level_config_t;


level_config_t *load_level_config(const char *filename);
void free_level_config(level_config_t *config);