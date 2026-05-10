// map_editor.c
#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "config/cJSON.h"
#include "config/config.h"
#include "engine/texture_manager.h"

#define TILE_SIZE         128
#define PALETTE_WIDTH     200
#define GRID_LINE_COLOR   CLITERAL(Color){ 50, 50, 50, 100 }

// Прототипы
static void SaveLevelConfig(const level_config_t *config, const char *filename);
static void BuildObjectPalette(level_config_t *config);
static void LoadTextures(texture_manager_t *tm, level_config_t* config);

// ------------------------------------------------------------
// Построение палитры объектов: 0-пустота, 1-стена(синяя),
// 2-дверь(фиолетовая), затем все .png из ../resources/map
// ------------------------------------------------------------
static void BuildObjectPalette(level_config_t *config) {
    // Удаляем старые объекты, если были
    if (config->objects) {
        free(config->objects);
        config->objects = NULL;
    }

    int capacity = 3;
    int count = 0;
    object_config_t *new_objects = (object_config_t*)malloc(sizeof(object_config_t) * capacity);

    // 0: empty (пустота)
    memset(&new_objects[count], 0, sizeof(object_config_t));
    new_objects[count].id = 0;
    strcpy(new_objects[count].name, "empty");
    strcpy(new_objects[count].texture, "");
    strcpy(new_objects[count].entity, "");
    count++;

    // 1: wall_stub (синяя заливка)
    memset(&new_objects[count], 0, sizeof(object_config_t));
    new_objects[count].id = 1;
    strcpy(new_objects[count].name, "wall");
    strcpy(new_objects[count].texture, "");   // текстура задаётся программно
    strcpy(new_objects[count].entity, "");
    count++;

    // 2: door_stub (фиолетовая заливка)
    memset(&new_objects[count], 0, sizeof(object_config_t));
    new_objects[count].id = 2;
    strcpy(new_objects[count].name, "door");
    strcpy(new_objects[count].texture, "");
    strcpy(new_objects[count].entity, "door");
    count++;

    // Сканируем папку с текстурами
    const char *mapDir = "../resources/map";
        FilePathList files = LoadDirectoryFiles(mapDir);
        for (int i = 0; i < files.count; i++) {
            const char *path = files.paths[i];
            // Только .png
            const char *ext = strrchr(path, '.');
            if (ext && strcmp(ext, ".png") == 0) {
                // Увеличиваем массив при необходимости
                if (count >= capacity) {
                    capacity *= 2;
                    new_objects = (object_config_t*)realloc(new_objects, sizeof(object_config_t) * capacity);
                }
                // Имя файла без расширения
                const char *fname = strrchr(path, '/');
                if (!fname) fname = strrchr(path, '\\');
                if (fname) fname++; else fname = path;
                char name[256];
                strncpy(name, fname, sizeof(name)-1);
                name[sizeof(name)-1] = '\0';
                char *dot = strrchr(name, '.');
                if (dot) *dot = '\0';

                memset(&new_objects[count], 0, sizeof(object_config_t));
                new_objects[count].id = count;          // ID начинаются с 3
                strcpy(new_objects[count].name, name);
                snprintf(new_objects[count].texture, sizeof(new_objects[count].texture), "%s/%s", mapDir, fname);
                strcpy(new_objects[count].entity, "");
                count++;
            }
        }
        UnloadDirectoryFiles(files);

    config->objects = new_objects;
    config->count_objects = count;
}

// ------------------------------------------------------------
// Генерация текстур-заглушек (синяя, фиолетовая) 32x32
// ------------------------------------------------------------
static void LoadTextures(texture_manager_t* tm, level_config_t* config) {
    if (tm->loaded[1] == 0) {
        Image img = GenImageColor(TILE_SIZE, TILE_SIZE, BLUE);
        tm->texture[1] = LoadTextureFromImage(img);
        UnloadImage(img);
        tm->loaded[1] = 1;
    }
    if (tm->loaded[2] == 0) {
        Image img = GenImageColor(TILE_SIZE, TILE_SIZE, PURPLE);
        tm->texture[2] = LoadTextureFromImage(img);
        UnloadImage(img);
        tm->loaded[2] = 1;
    }

}

// ------------------------------------------------------------
// Сохранение карты в JSON
// ------------------------------------------------------------
static void SaveLevelConfig(const level_config_t *config, const char *filename) {
    cJSON *root = cJSON_CreateObject();

    // level_info
    cJSON *levelInfo = cJSON_CreateObject();
    cJSON_AddStringToObject(levelInfo, "name", config->name);
    cJSON_AddStringToObject(levelInfo, "background", config->texture_background);
    cJSON_AddStringToObject(levelInfo, "next_level", config->next_level);
    cJSON_AddItemToObject(root, "level_info", levelInfo);

    // objects (актуальная палитра)
    cJSON *objects = cJSON_CreateArray();
    for (int i = 0; i < config->count_objects; i++) {
        cJSON *obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "id", config->objects[i].id);
        cJSON_AddStringToObject(obj, "name", config->objects[i].name);
        cJSON_AddStringToObject(obj, "texture", config->objects[i].texture);
        cJSON_AddStringToObject(obj, "entity", config->objects[i].entity);
        cJSON_AddItemToArray(objects, obj);
    }
    cJSON_AddItemToObject(root, "objects", objects);

    // map
    cJSON *map = cJSON_CreateObject();
    cJSON *size = cJSON_CreateObject();
    cJSON_AddNumberToObject(size, "rows", config->layout->rows);
    cJSON_AddNumberToObject(size, "cols", config->layout->cols);
    cJSON_AddItemToObject(map, "size", size);

    cJSON *data = cJSON_CreateArray();
    for (int r = 0; r < config->layout->rows; r++) {
        cJSON *row = cJSON_CreateArray();
        for (int c = 0; c < config->layout->cols; c++) {
            cJSON_AddItemToArray(row, cJSON_CreateNumber(config->layout->data[r][c]));
        }
        cJSON_AddItemToArray(data, row);
    }
    cJSON_AddItemToObject(map, "data", data);
    cJSON_AddItemToObject(root, "map", map);

    char *jsonStr = cJSON_Print(root);
    cJSON_Delete(root);

    FILE *file = fopen(filename, "w");
    if (file) {
        fputs(jsonStr, file);
        fclose(file);
        printf("[INFO] Map saved to %s\n", filename);
    } else {
        fprintf(stderr, "[ERROR] Could not write %s\n", filename);
    }
    free(jsonStr);
}

// ------------------------------------------------------------
// Создание новой карты (структура без объектов, карта 10x10)
// ------------------------------------------------------------
static level_config_t* CreateDefaultConfig(void) {
    level_config_t *config = malloc(sizeof(level_config_t));
    memset(config, 0, sizeof(level_config_t));

    strcpy(config->name, "Default");
    strcpy(config->next_level, "");
    strcpy(config->texture_background, "resources/backgrounds/default_bg.png");

    config->objects = NULL;
    config->count_objects = 0;

    config->layout = malloc(sizeof(map_layout_t));
    config->layout->rows = 10;
    config->layout->cols = 10;
    config->layout->data = malloc(sizeof(int*) * config->layout->rows);
    for (int i = 0; i < config->layout->rows; i++) {
        config->layout->data[i] = malloc(sizeof(int) * config->layout->cols);
        for (int j = 0; j < config->layout->cols; j++) {
            // Края — wall_stub (id=1), остальное — empty
            config->layout->data[i][j] = (i==0 || i==9 || j==0 || j==9) ? 1 : 0;
        }
    }
    return config;
}

// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <map_file.json>\n", argv[0]);
        return 1;
    }

    char mapPath[512];
    snprintf(mapPath, sizeof(mapPath), "../maps/%s", argv[1]);

    level_config_t *config = NULL;

    // 1. Загрузка существующей карты или создание новой
    if (FileExists(mapPath)) {
        config = load_level_config(mapPath);
        if (!config) {
            fprintf(stderr, "[ERROR] Failed to load map.\n");
            return 1;
        }
        // Заменяем старую палитру на новую (из файлов + заглушки)
        BuildObjectPalette(config);
        // Если в данных карты есть ID за пределами новой палитры, сбрасываем в 0
        for (int r = 0; r < config->layout->rows; r++) {
            for (int c = 0; c < config->layout->cols; c++) {
                if (config->layout->data[r][c] >= config->count_objects)
                    config->layout->data[r][c] = 0;
            }
        }
    } else {
        config = CreateDefaultConfig();
        if (!config) return 1;
        BuildObjectPalette(config);
        SaveLevelConfig(config, mapPath);
        printf("[INFO] New default map created: %s\n", mapPath);
    }

    // 2. Инициализация окна
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Map Editor – palette from files");
    SetTargetFPS(60);

    // 3. Текстуры
    texture_manager_t textureManager;
    init_texture_manager(&textureManager, config);
    LoadTextures(&textureManager, config);   // синий и фиолетовый квадраты

    // 4. Расчёт размеров сетки
    int gridAreaWidth = screenWidth - PALETTE_WIDTH;
    float cellSize = fminf((float)gridAreaWidth / config->layout->cols,
                           (float)screenHeight / config->layout->rows);
    if (cellSize < 4) cellSize = 4;

    int currentTile = 1;   // активный инструмент (wall_stub)
    bool showGrid = true;

    // 5. Главный цикл
    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            // Клик по палитре справа
            if (mouse.x >= gridAreaWidth) {
                int index = (int)((mouse.y - 40) / 40);
                if (index >= 0 && index < config->count_objects) {
                    currentTile = config->objects[index].id;
                }
            }
            // Клик по карте — рисуем выбранный тайл
            else {
                int col = (int)(mouse.x / cellSize);
                int row = (int)(mouse.y / cellSize);
                if (row >= 0 && row < config->layout->rows &&
                    col >= 0 && col < config->layout->cols) {
                    config->layout->data[row][col] = currentTile;
                }
            }
        }

        // Сохранение: Ctrl+S
        if (IsKeyPressed(KEY_S) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))) {
            SaveLevelConfig(config, mapPath);
        }
        // Сетка: G
        if (IsKeyPressed(KEY_G)) showGrid = !showGrid;

        // Рендеринг
        BeginDrawing();
        ClearBackground(DARKGRAY);

        // Карта
        for (int row = 0; row < config->layout->rows; row++) {
            for (int col = 0; col < config->layout->cols; col++) {
                int id = config->layout->data[row][col];
                Rectangle dest = { col * cellSize, row * cellSize, cellSize, cellSize };

                if (id >= 0 && id < MAX_TEXTURES && textureManager.loaded[id]) {
                    DrawTexturePro(textureManager.texture[id],
                                   (Rectangle){16, 16, TILE_SIZE /2, TILE_SIZE /2},
                                   dest, (Vector2){0,0}, 0.0f, WHITE);
                } else {
                    DrawRectangleRec(dest, (Color){40,40,40,255});
                }

                if (showGrid) DrawRectangleLinesEx(dest, 1, GRID_LINE_COLOR);
            }
        }

        // Подсветка клетки под курсором
        if (mouse.x < gridAreaWidth) {
            int col = (int)(mouse.x / cellSize);
            int row = (int)(mouse.y / cellSize);
            if (row >= 0 && row < config->layout->rows && col >= 0 && col < config->layout->cols) {
                Rectangle r = { col * cellSize, row * cellSize, cellSize, cellSize };
                DrawRectangleLinesEx(r, 2, YELLOW);
            }
        }

        // Палитра
        DrawRectangle(gridAreaWidth, 0, PALETTE_WIDTH, screenHeight, (Color){30,30,30,255});
        DrawText("PALETTE", gridAreaWidth + 10, 10, 20, RAYWHITE);
        for (int i = 0; i < config->count_objects; i++) {
            int y = 40 + i * 40;
            int id = config->objects[i].id;
            Color bg = (id == currentTile) ? SKYBLUE : DARKGRAY;
            DrawRectangle(gridAreaWidth + 5, y, PALETTE_WIDTH - 10, 35, bg);

            if (id >= 0 && id < MAX_TEXTURES && textureManager.loaded[id]) {
                DrawTexturePro(textureManager.texture[id],
                               (Rectangle){0, 0, TILE_SIZE, TILE_SIZE},
                               (Rectangle){ gridAreaWidth + 10, y + 2, 30, 30 },
                               (Vector2){0,0}, 0.0f, WHITE);
            }
            DrawText(config->objects[i].name, gridAreaWidth + 50, y + 8, 16, RAYWHITE);
        }

        DrawText("Ctrl+S: Save   G: Toggle grid", 10, screenHeight - 25, 16, LIGHTGRAY);
        EndDrawing();
    }

    // 6. Очистка
    free_texture_manager(&textureManager);
    free_level_config(config);
    CloseWindow();
    return 0;
}