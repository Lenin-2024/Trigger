#ifndef __TEXTURE_MANAGER_H__
#define __TEXTURE_MANAGER_H__

#include "raylib.h"
#include "../config/config.h"

#define MAX_TEXTURES 256

struct texture_manager {
    Texture2D texture[MAX_TEXTURES];
    int loaded[MAX_TEXTURES];
    int max_id;
};
typedef struct texture_manager texture_manager_t;

void init_texture_manager(texture_manager_t *texture_manager, level_config_t *config);
void free_texture_manager(texture_manager_t *texture_manager);

#endif /* __TEXTURE_MANAGER_H__ */