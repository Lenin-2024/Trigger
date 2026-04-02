#include <string.h>

#include "config.h"
#include "cJSON.h"

level_config_t *load_level_config(const char *filename) {
    FILE * file;
    if ((file = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "[ ERROR ] Cannot open config file %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = (char *)malloc(file_size + 1);
    if (!data) {
        fprintf(stderr, "[ ERROR ] Cannot allocate memory for data!\n");
        fclose(file);
        return NULL;
    }

    fread(data, 1, file_size, file);
    data[file_size] = 0;
    fclose(file);
    
    cJSON *json = cJSON_Parse(data);
    free(data);
    if (!json) {
        fprintf(stderr, "[ ERROR ] Cannot parse json\n");
        return NULL;
    }

    level_config_t *config = (level_config_t *)malloc(sizeof(level_config_t));
    if (!config) {
        fprintf(stderr, "[ ERROR ] Cannot allocate memory for config\n");
        cJSON_Delete(json);
        return NULL;
    }
    memset(config, 0, sizeof(level_config_t));

    cJSON *level_info = cJSON_GetObjectItem(json, "level_info");
    if (level_info) {
        cJSON *name = cJSON_GetObjectItem(level_info, "name");
        if (name) {
            strcpy(config->name, name->valuestring);
        }
        
        cJSON *background = cJSON_GetObjectItem(level_info, "background");
        if (background) {
            strcpy(config->texture_background, background->valuestring);
        }

        cJSON *next_level = cJSON_GetObjectItem(level_info, "next_level");
        if (next_level) {
            strcpy(config->next_level, next_level->valuestring);
        }
        
        printf("[ INFO ] level name = %s, nex level = %s\n", config->name, config->next_level);
    }

    cJSON *tiles = cJSON_GetObjectItem(json, "objects");
    if (cJSON_IsArray(tiles)) {
        size_t count_obj = cJSON_GetArraySize(tiles);
        config->count_objects = count_obj;
        config->objects = (object_config_t *)malloc(sizeof(object_config_t) * config->count_objects);
        if (!config->objects) {
            fprintf(stderr, "[ ERROR ] Cannot allocate memory for object\n");
            cJSON_Delete(json);
            free(config);
            return NULL;
        }

        for (int i = 0; i < count_obj; i++) {
            cJSON *obj = cJSON_GetArrayItem(tiles, i);
            if (obj) {
                cJSON *id = cJSON_GetObjectItem(obj, "id");
                cJSON *name = cJSON_GetObjectItem(obj, "name");
                cJSON *texture = cJSON_GetObjectItem(obj, "texture");
                cJSON *entity = cJSON_GetObjectItem(obj, "entity");
                
                if (id) {
                    config->objects[i].id = id->valueint;
                }

                if (name) {
                    strcpy(config->objects[i].name, name->valuestring);
                }

                if (texture) {
                    strcpy(config->objects[i].texture, texture->valuestring);
                }

                if (entity) {
                    strcpy(config->objects[i].entity, entity->valuestring);
                }
                
                printf("[ INFO ] object(%d, %s, %s, %s)\n", config->objects[i].id, config->objects[i].name, config->objects[i].texture, config->objects[i].entity);
            }
        }
    }

    config->layout = (map_layout_t *)malloc(sizeof(map_layout_t));
    if (!config->layout) {
        fprintf(stderr, "[ ERROR ] Cannot allocate memory for map\n");
        cJSON_Delete(json);
        free(config);
        return NULL;
    }

    cJSON *map_data = cJSON_GetObjectItem(json, "map");
    if (map_data) {
        cJSON *size = cJSON_GetObjectItem(map_data, "size");
        if (size) {
            config->layout->rows = cJSON_GetObjectItem(size, "rows")->valueint;
            config->layout->cols = cJSON_GetObjectItem(size, "cols")->valueint;
            printf("[ INFO ] size = (%d, %d)\n", config->layout->rows, config->layout->cols);
        }

        cJSON *array = cJSON_GetObjectItem(map_data, "data");
        if (cJSON_IsArray(array)) {
            /* Выжеление памяти под карту */
            config->layout->data = (int **)malloc(sizeof(int *) * config->layout->rows);
            if (!config->layout->data) {
                fprintf(stderr, "[ ERROR ] Cannot allocate memory for data map\n");
                cJSON_Delete(json);
                free(config->layout->data);
                free(config->layout);
                free(config->objects);
                free(config);
                return NULL;
            }

            for (int i = 0; i < config->layout->rows; i++) {
                config->layout->data[i] = (int *)malloc(sizeof(int) * config->layout->cols);
                if (!config->layout->data[i]) {
                    for (int k = i - 1; k >= 0; k--) {
                        free(config->layout->data[k]);
                    }
                    free(config->layout);
                    free(config->objects);
                    free(config);
                    return NULL;
                }
            }

            /* Заполнение карты */
            for (int i = 0; i < config->layout->rows; i++) {
                cJSON *row = cJSON_GetArrayItem(array, i);
                if (cJSON_IsArray(row)) {
                    for (int j = 0; j < config->layout->cols; j++) {
                        printf("%d ", cJSON_GetArrayItem(row, j)->valueint);
                        config->layout->data[i][j] = cJSON_GetArrayItem(row, j)->valueint;
                    }
                }
                printf("\n");
            }
        }
    }

    cJSON_Delete(json);
    return config;
}