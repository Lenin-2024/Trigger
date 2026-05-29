// map_editor.c
#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "config/cJSON.h"
#include "config/config.h"
#include "engine/texture_manager.h"

#define TILE_SIZE         32
#define PALETTE_WIDTH     200
#define GRID_LINE_COLOR   CLITERAL(Color){ 50, 50, 50, 100 }
#define ITEMS_PER_PAGE    17   // максимум объектов, отображаемых в палитре одновременно

// Прототипы
static void SaveLevelConfig(const level_config_t *config, const char *filename);
static void BuildObjectPalette(level_config_t *config);

static void BuildObjectPalette(level_config_t *config) {
    if (config->objects) {
        free(config->objects);
        config->objects = NULL;
    }

    int capacity = 2 + 10;
    int count = 0;
    object_config_t *new_objects = (object_config_t*)malloc(sizeof(object_config_t) * capacity);

    // 0: empty
    memset(&new_objects[count], 0, sizeof(object_config_t));
    new_objects[count].id = 0;
    strcpy(new_objects[count].name, "empty");
    strcpy(new_objects[count].texture, "");
    strcpy(new_objects[count].entity, "");
    count++;

    // 1: player
    memset(&new_objects[count], 0, sizeof(object_config_t));
    new_objects[count].id = 1;
    strcpy(new_objects[count].name, "player");
    strcpy(new_objects[count].texture, "resources/map/male_hero_template.png");
    strcpy(new_objects[count].entity, "player");
    count++;

    // Сканируем папку
    const char *mapDir = "../resources/map";
    if (DirectoryExists(mapDir)) {
        FilePathList files = LoadDirectoryFiles(mapDir);
        for (int i = 0; i < files.count; i++) {
            const char *path = files.paths[i];
            const char *ext = strrchr(path, '.');
            if (ext && strcmp(ext, ".png") == 0) {
                const char *fname = strrchr(path, '/');
                if (!fname) fname = strrchr(path, '\\');
                if (fname) fname++; else fname = path;

                // Пропускаем игрока
                if (strcmp(fname, "male_hero_template.png") == 0)
                    continue;

                if (count >= capacity) {
                    capacity *= 2;
                    new_objects = (object_config_t*)realloc(new_objects, sizeof(object_config_t) * capacity);
                }

                char name[256];
                strncpy(name, fname, sizeof(name)-1);
                name[sizeof(name)-1] = '\0';
                char *dot = strrchr(name, '.');
                if (dot) *dot = '\0';

                memset(&new_objects[count], 0, sizeof(object_config_t));
                new_objects[count].id = count;
                strcpy(new_objects[count].name, name);
                snprintf(new_objects[count].texture, sizeof(new_objects[count].texture), "%s/%s", mapDir, fname);
                strcpy(new_objects[count].entity, "");
                count++;
            }
        }
        UnloadDirectoryFiles(files);
    } else {
        printf("[WARNING] Directory %s not found.\n", mapDir);
    }

    config->objects = new_objects;
    config->count_objects = count;
}

// ------------------------------------------------------------
// Сохранение
// ------------------------------------------------------------
static void SaveLevelConfig(const level_config_t *config, const char *filename) {
    cJSON *root = cJSON_CreateObject();

    cJSON *levelInfo = cJSON_CreateObject();
    cJSON_AddStringToObject(levelInfo, "name", config->name);
    cJSON_AddStringToObject(levelInfo, "background", config->texture_background);
    cJSON_AddStringToObject(levelInfo, "next_level", config->next_level);
    cJSON_AddItemToObject(root, "level_info", levelInfo);

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
// Новая карта
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
    config->layout->rows = 500;
    config->layout->cols = 500;
    config->layout->data = malloc(sizeof(int*) * config->layout->rows);
    for (int i = 0; i < config->layout->rows; i++) {
        config->layout->data[i] = malloc(sizeof(int) * config->layout->cols);
        for (int j = 0; j < config->layout->cols; j++) {
            config->layout->data[i][j] = 0;
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

    if (FileExists(mapPath)) {
        config = load_level_config(mapPath);
        if (!config) {
            fprintf(stderr, "[ERROR] Failed to load map.\n");
            return 1;
        }
        BuildObjectPalette(config);
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
        int wallId = 0;
        for (int i = 0; i < config->count_objects; i++) {
            if (strcmp(config->objects[i].name, "wall") == 0) {
                wallId = config->objects[i].id;
                break;
            }
        }
        if (wallId == 0 && config->count_objects > 2) wallId = 2;
        for (int i = 0; i < config->layout->rows; i++) {
            for (int j = 0; j < config->layout->cols; j++) {
                if (i == 0 || i == 9 || j == 0 || j == 9)
                    config->layout->data[i][j] = wallId;
                else
                    config->layout->data[i][j] = 0;
            }
        }
        SaveLevelConfig(config, mapPath);
        printf("[INFO] New default map created: %s\n", mapPath);
    }
    const int screenWidth = 1280;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Map Editor");
    SetTargetFPS(60);

    texture_manager_t textureManager;
    init_texture_manager(&textureManager, config);

    int gridAreaWidth = screenWidth - PALETTE_WIDTH;
    float cellSize = fminf((float)gridAreaWidth / 20,
                           (float)screenHeight / 20);
    if (cellSize < 4) cellSize = 4;

    int currentTile = config->objects[0].id;   // по умолчанию пустота
    bool showGrid = true;

    // Параметры страничной палитры
    int palettePage = 0;
    int totalPages = (config->count_objects + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (mouse.x >= gridAreaWidth) {
                // Определяем индекс объекта с учётом текущей страницы
                int rowInPalette = (int)((mouse.y - 40) / 40);
                if (rowInPalette >= 0 && rowInPalette < ITEMS_PER_PAGE) {
                    int objIndex = palettePage * ITEMS_PER_PAGE + rowInPalette;
                    if (objIndex >= 0 && objIndex < config->count_objects) {
                        currentTile = config->objects[objIndex].id;
                    }
                }
            } else {
                int col = (int)(mouse.x / cellSize);
                int row = (int)(mouse.y / cellSize);
                if (row >= 0 && row < config->layout->rows &&
                    col >= 0 && col < config->layout->cols) {
                    config->layout->data[row][col] = currentTile;
                }
            }
        }

        // Перелистывание страниц палитры стрелками влево/вправо
        if (IsKeyPressed(KEY_A)) {
            palettePage = (palettePage - 1 + totalPages) % totalPages;
        }
        if (IsKeyPressed(KEY_D)) {
            palettePage = (palettePage + 1) % totalPages;
        }

        if (IsKeyPressed(KEY_S) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))) {
            SaveLevelConfig(config, mapPath);
        }
        if (IsKeyPressed(KEY_G)) showGrid = !showGrid;

        BeginDrawing();
        ClearBackground(DARKGRAY);

        // Отрисовка карты
        for (int row = 0; row < config->layout->rows; row++) {
            for (int col = 0; col < config->layout->cols; col++) {
                int id = config->layout->data[row][col];
                Rectangle dest = { col * cellSize, row * cellSize, cellSize, cellSize };

                if (id >= 0 && id < MAX_TEXTURES && textureManager.loaded[id]) {
                    DrawTexturePro(textureManager.texture[id],
                                   (Rectangle){0, 0, TILE_SIZE, TILE_SIZE},
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
        DrawText(TextFormat("PALETTE %d/%d", palettePage+1, totalPages), gridAreaWidth + 10, 10, 16, RAYWHITE);

        // Отображаем только объекты текущей страницы
        int startIdx = palettePage * ITEMS_PER_PAGE;
        int endIdx = startIdx + ITEMS_PER_PAGE;
        if (endIdx > config->count_objects) endIdx = config->count_objects;

        for (int i = startIdx; i < endIdx; i++) {
            int displayRow = i - startIdx;   // 0..16
            int y = 40 + displayRow * 40;
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

        DrawText("Ctrl+S: Save   G: Toggle grid   A/D: Palette page", 10, screenHeight - 25, 16, LIGHTGRAY);
        EndDrawing();
    }

    free_texture_manager(&textureManager);
    free_level_config(config);
    CloseWindow();
    return 0;
}