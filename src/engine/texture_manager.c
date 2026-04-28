#include <string.h>
#include "texture_manager.h"

void init_texture_manager(texture_manager_t *texture_manager, level_config_t *config) {
    memset(texture_manager, 0, sizeof(texture_manager_t));
    texture_manager->max_id = 0;
    for (int i = 0; i < config->count_objects; i++) {
        int id = config->objects[i].id;
        if (id > texture_manager->max_id) {
            texture_manager->max_id = id;
        }

        if (strlen(config->objects[id].texture) == 0) {
            texture_manager->loaded[id] = 0;
            continue;
        }

        texture_manager->texture[id] = LoadTexture(config->objects[id].texture);
        if ((texture_manager->texture[id].height == 0) || (texture_manager->texture[id].width == 0)) {
            fprintf(stderr, "[ WARNING ] texture %s not load\n", config->objects[id].texture);
            texture_manager->loaded[id] = 0;
            Image image = GenImageColor(32, 32, MAGENTA);
            texture_manager->texture[id] = LoadTextureFromImage(image);
            UnloadImage(image);
        } else {
            fprintf(stderr, "[ INFO ] texture %s was load\n", config->objects[id].texture);
            texture_manager->loaded[id] = 1;
        }
    }
}

void free_texture_manager(texture_manager_t *texture_manager) {
    for (int i = 0; i <  texture_manager->max_id; i++) {
        if (texture_manager->loaded[i]) {
            UnloadTexture(texture_manager->texture[i]);
            texture_manager->loaded[i] = 0;
        }
    }
}
