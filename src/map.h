#ifndef __MAP_H__
#define __MAP_H__

#include "config/config.h"
#include "engine/engine_context.h"

struct game_state;

level_config_t *get_map(engine_context_t *eng, const char *file_name);
void draw_map(engine_context_t *eng, level_config_t *map);
void free_map(engine_context_t *eng, level_config_t *map);

#endif /* __MAP_H__ */