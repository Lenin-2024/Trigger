#ifndef __MAP_H__
#define __MAP_H__

struct game_state;

struct map {
    int **arr;
    int rows, cols;
};
typedef struct map map_t;

map_t *get_map(char *file_name, struct game_state* game);
void draw_map(map_t *map);
void free_map(map_t *map);

#endif /* __MAP_H__ */