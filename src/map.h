#ifndef __MAP_H__
#define __MAP_H__

#include "config/config.h"
struct game_state;

level_config_t *get_map(char *file_name, struct game_state* game);
void draw_map(level_config_t *map);
void free_map(level_config_t *map);

#endif /* __MAP_H__ */