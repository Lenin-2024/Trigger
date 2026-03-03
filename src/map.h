#ifndef __MAP_H__
#define __MAP_H__

#include "player.h"

struct map {
    int **arr;
    int rows, cols;
};
typedef struct map map_t;

map_t *get_map(char *file_name);
void draw_map(player_t player, map_t *map);
void free_map(map_t *map);

#endif /* __MAP_H__ */