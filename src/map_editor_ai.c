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
#define ITEMS_PER_PAGE    17
#define MAX_OBJECTS       256
#define EDITOR_MAX_SIZE   500

static void SaveLevelConfig(const level_config_t *config, const char *filename);
static void BuildObjectPalette(level_config_t *config);
static void MergeObjectPalette(level_config_t *config);
static void ExpandMapToEditorSize(level_config_t *config, int maxRows, int maxCols);

// ------------------------------------------------------------
// Построение палитры для НОВОЙ карты (0:empty, 1:player, 2+ из папки)
// ------------------------------------------------------------
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
    strcpy(new_objects[count].texture, "../resources/map/male_hero_template.png");
    strcpy(new_objects[count].entity, "player");
    count++;

    // 2: platform
    memset(&new_objects[count], 0, sizeof(object_config_t));
    new_objects[count].id = 1;
    strcpy(new_objects[count].name, "platform");
    strcpy(new_objects[count].texture, "../resources/map/IndustrialTile_81.png");
    strcpy(new_objects[count].entity, "door");
    count++;

    // 3: level end
    memset(&new_objects[count], 0, sizeof(object_config_t));
    new_objects[count].id = 1;
    strcpy(new_objects[count].name, "end");
    strcpy(new_objects[count].texture, "../resources/map/IndustrialTile_45.png");
    strcpy(new_objects[count].entity, "");
    count++;

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

                // Пропускаем игрока, он уже добавлен
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
// Слияние объектов загруженной карты с папкой ресурсов
// ------------------------------------------------------------
static void MergeObjectPalette(level_config_t *config) {
    object_config_t *old_objects = config->objects;
    int old_count = config->count_objects;

    // Начальная ёмкость с запасом
    int temp_capacity = old_count + 32;
    object_config_t *temp = malloc(sizeof(object_config_t) * temp_capacity);
    if (!temp) {
        fprintf(stderr, "[ERROR] MergeObjectPalette: malloc failed\n");
        return;
    }
    int temp_count = 0;

    bool has_empty = false;
    bool has_player = false;
    int max_id = 0;

    // 1. Копируем и корректируем загруженные объекты
    for (int i = 0; i < old_count; i++) {
        object_config_t *src = &old_objects[i];
        if (src->id == 0) {
            memset(&temp[temp_count], 0, sizeof(object_config_t));
            temp[temp_count].id = 0;
            strcpy(temp[temp_count].name, "empty");
            temp[temp_count].texture[0] = '\0';
            temp[temp_count].entity[0] = '\0';
            has_empty = true;
        } else if (src->id == 1) {
            memset(&temp[temp_count], 0, sizeof(object_config_t));
            temp[temp_count].id = 1;
            strcpy(temp[temp_count].name, "player");
            strcpy(temp[temp_count].texture, "../resources/map/male_hero_template.png");
            strcpy(temp[temp_count].entity, "player");
            has_player = true;
        } else {
            // Копируем объект, исправляя путь для загрузки
            object_config_t obj = *src;
            if (strncmp(obj.texture, "resources/", 10) == 0) {
                char corrected[512];
                snprintf(corrected, sizeof(corrected), "../%s", obj.texture);
                strcpy(obj.texture, corrected);
            }
            if (obj.id > max_id) max_id = obj.id;
            temp[temp_count] = obj;
        }
        temp_count++;
    }

    // Добавляем empty/player, если их не было
    if (!has_empty) {
        memset(&temp[temp_count], 0, sizeof(object_config_t));
        temp[temp_count].id = 0;
        strcpy(temp[temp_count].name, "empty");
        temp[temp_count].texture[0] = '\0';
        temp[temp_count].entity[0] = '\0';
        temp_count++;
    }
    if (!has_player) {
        memset(&temp[temp_count], 0, sizeof(object_config_t));
        temp[temp_count].id = 1;
        strcpy(temp[temp_count].name, "player");
        strcpy(temp[temp_count].texture, "../resources/map/male_hero_template.png");
        strcpy(temp[temp_count].entity, "player");
        temp_count++;
    }

    // 2. Добавляем .png из ../resources/map, которых ещё нет
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

                // Игнорируем игрока
                if (strcmp(fname, "male_hero_template.png") == 0)
                    continue;

                // Формируем полный путь для сравнения
                char fullPath[512];
                snprintf(fullPath, sizeof(fullPath), "%s/%s", mapDir, fname);

                // Проверяем, есть ли уже такой объект
                bool exists = false;
                for (int j = 0; j < temp_count; j++) {
                    if (strcmp(temp[j].texture, fullPath) == 0) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    // Расширяем массив, если нужно
                    if (temp_count >= temp_capacity) {
                        temp_capacity *= 2;
                        object_config_t *new_temp = realloc(temp, sizeof(object_config_t) * temp_capacity);
                        if (!new_temp) {
                            fprintf(stderr, "[ERROR] MergeObjectPalette: realloc failed\n");
                            UnloadDirectoryFiles(files);
                            free(temp);
                            return;
                        }
                        temp = new_temp;
                    }

                    char name[256];
                    strncpy(name, fname, sizeof(name)-1);
                    name[sizeof(name)-1] = '\0';
                    char *dot = strrchr(name, '.');
                    if (dot) *dot = '\0';

                    memset(&temp[temp_count], 0, sizeof(object_config_t));
                    temp[temp_count].id = ++max_id;
                    strcpy(temp[temp_count].name, name);
                    strcpy(temp[temp_count].texture, fullPath);
                    temp[temp_count].entity[0] = '\0';
                    temp_count++;
                }
            }
        }
        UnloadDirectoryFiles(files);
    }

    // Заменяем старый массив объектов
    free(old_objects);
    config->objects = realloc(temp, sizeof(object_config_t) * temp_count);
    config->count_objects = temp_count;
}
// ------------------------------------------------------------
// Сохранение с удалением "../" из путей и обрезкой карты
// ------------------------------------------------------------
static void SaveLevelConfig(const level_config_t *config, const char *filename) {
    int rows = config->layout->rows;
    int cols = config->layout->cols;

    int min_r = rows, max_r = -1, min_c = cols, max_c = -1;
    bool found = false;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (config->layout->data[r][c] != 0) {
                if (r < min_r) min_r = r;
                if (r > max_r) max_r = r;
                if (c < min_c) min_c = c;
                if (c > max_c) max_c = c;
                found = true;
            }
        }
    }
    if (!found) {
        min_r = max_r = 0;
        min_c = max_c = 0;
    }

    int new_rows = max_r - min_r + 1;
    int new_cols = max_c - min_c + 1;

    int **new_data = malloc(sizeof(int*) * new_rows);
    for (int i = 0; i < new_rows; i++) {
        new_data[i] = malloc(sizeof(int) * new_cols);
        for (int j = 0; j < new_cols; j++) {
            new_data[i][j] = config->layout->data[min_r + i][min_c + j];
        }
    }

    bool used_ids[MAX_OBJECTS] = {0};
    for (int i = 0; i < new_rows; i++) {
        for (int j = 0; j < new_cols; j++) {
            int id = new_data[i][j];
            if (id >= 0 && id < MAX_OBJECTS) used_ids[id] = true;
        }
    }

    int old_ids_sorted[MAX_OBJECTS];
    int used_count = 0;
    for (int id = 0; id < MAX_OBJECTS; id++) {
        if (used_ids[id]) old_ids_sorted[used_count++] = id;
    }

    int id_map[MAX_OBJECTS];
    memset(id_map, -1, sizeof(id_map));
    for (int i = 0; i < used_count; i++) {
        id_map[old_ids_sorted[i]] = i;
    }

    for (int i = 0; i < new_rows; i++) {
        for (int j = 0; j < new_cols; j++) {
            new_data[i][j] = id_map[new_data[i][j]];
        }
    }

    object_config_t *new_objects = malloc(sizeof(object_config_t) * used_count);
    for (int i = 0; i < used_count; i++) {
        int old_id = old_ids_sorted[i];
        object_config_t *src = NULL;
        for (int k = 0; k < config->count_objects; k++) {
            if (config->objects[k].id == old_id) {
                src = &config->objects[k];
                break;
            }
        }
        memset(&new_objects[i], 0, sizeof(object_config_t));
        new_objects[i].id = i;
        if (src) {
            strcpy(new_objects[i].name, src->name);
            // Убираем "../" из путей текстур для всех объектов
            char cleanPath[256];
            if (strncmp(src->texture, "../", 3) == 0) {
                strcpy(cleanPath, src->texture + 3); // обрезаем первые 3 символа
            } else {
                strcpy(cleanPath, src->texture);
            }
            strcpy(new_objects[i].texture, cleanPath);
            strcpy(new_objects[i].entity, src->entity);
        } else {
            strcpy(new_objects[i].name, "unknown");
            strcpy(new_objects[i].texture, "");
            strcpy(new_objects[i].entity, "");
        }
    }

    cJSON *root = cJSON_CreateObject();
    cJSON *levelInfo = cJSON_CreateObject();
    cJSON_AddStringToObject(levelInfo, "name", config->name);
    cJSON_AddStringToObject(levelInfo, "background", config->texture_background);
    cJSON_AddStringToObject(levelInfo, "next_level", config->next_level);
    cJSON_AddItemToObject(root, "level_info", levelInfo);

    cJSON *objects_json = cJSON_CreateArray();
    for (int i = 0; i < used_count; i++) {
        cJSON *obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "id", new_objects[i].id);
        cJSON_AddStringToObject(obj, "name", new_objects[i].name);
        cJSON_AddStringToObject(obj, "texture", new_objects[i].texture);
        cJSON_AddStringToObject(obj, "entity", new_objects[i].entity);
        cJSON_AddItemToArray(objects_json, obj);
    }
    cJSON_AddItemToObject(root, "objects", objects_json);

    cJSON *map = cJSON_CreateObject();
    cJSON *size = cJSON_CreateObject();
    cJSON_AddNumberToObject(size, "rows", new_rows);
    cJSON_AddNumberToObject(size, "cols", new_cols);
    cJSON_AddItemToObject(map, "size", size);

    cJSON *data = cJSON_CreateArray();
    for (int r = 0; r < new_rows; r++) {
        cJSON *row = cJSON_CreateArray();
        for (int c = 0; c < new_cols; c++) {
            cJSON_AddItemToArray(row, cJSON_CreateNumber(new_data[r][c]));
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
        printf("[INFO] Map saved to %s (%dx%d, %d objects)\n", filename, new_rows, new_cols, used_count);
    } else {
        fprintf(stderr, "[ERROR] Could not write %s\n", filename);
    }
    free(jsonStr);

    for (int i = 0; i < new_rows; i++) free(new_data[i]);
    free(new_data);
    free(new_objects);
}

// ------------------------------------------------------------
static void ExpandMapToEditorSize(level_config_t *config, int maxRows, int maxCols) {
    if (config->layout->rows == maxRows && config->layout->cols == maxCols) return;

    int old_rows = config->layout->rows;
    int old_cols = config->layout->cols;

    int **new_data = malloc(sizeof(int*) * maxRows);
    for (int i = 0; i < maxRows; i++) {
        new_data[i] = malloc(sizeof(int) * maxCols);
        memset(new_data[i], 0, sizeof(int) * maxCols);
    }

    int copy_rows = old_rows < maxRows ? old_rows : maxRows;
    int copy_cols = old_cols < maxCols ? old_cols : maxCols;
    for (int r = 0; r < copy_rows; r++) {
        for (int c = 0; c < copy_cols; c++) {
            new_data[r][c] = config->layout->data[r][c];
        }
    }

    for (int i = 0; i < old_rows; i++) free(config->layout->data[i]);
    free(config->layout->data);

    config->layout->data = new_data;
    config->layout->rows = maxRows;
    config->layout->cols = maxCols;
}

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
    config->layout->rows = EDITOR_MAX_SIZE;
    config->layout->cols = EDITOR_MAX_SIZE;
    config->layout->data = malloc(sizeof(int*) * EDITOR_MAX_SIZE);
    for (int i = 0; i < EDITOR_MAX_SIZE; i++) {
        config->layout->data[i] = malloc(sizeof(int) * EDITOR_MAX_SIZE);
        memset(config->layout->data[i], 0, sizeof(int) * EDITOR_MAX_SIZE);
    }
    return config;
}

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
        // Расширяем до 500x500
        ExpandMapToEditorSize(config, EDITOR_MAX_SIZE, EDITOR_MAX_SIZE);
        // Сливаем объекты из файла с папкой ресурсов
        MergeObjectPalette(config);
        // Сброс ID, отсутствующих в новой палитре
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

    int currentTile = config->objects[0].id;
    bool showGrid = true;

    int palettePage = 0;
    int totalPages = (config->count_objects + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;

    Vector2 cameraOffset = {0, 0};
    float moveSpeed = 200.0f;

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_RIGHT)) cameraOffset.x += moveSpeed * GetFrameTime();
        if (IsKeyDown(KEY_LEFT))  cameraOffset.x -= moveSpeed * GetFrameTime();
        if (IsKeyDown(KEY_DOWN))  cameraOffset.y += moveSpeed * GetFrameTime();
        if (IsKeyDown(KEY_UP))    cameraOffset.y -= moveSpeed * GetFrameTime();

        float maxOffX = (config->layout->cols * cellSize) - gridAreaWidth;
        float maxOffY = (config->layout->rows * cellSize) - screenHeight;
        if (maxOffX < 0) maxOffX = 0;
        if (maxOffY < 0) maxOffY = 0;
        if (cameraOffset.x < 0) cameraOffset.x = 0;
        if (cameraOffset.y < 0) cameraOffset.y = 0;
        if (cameraOffset.x > maxOffX) cameraOffset.x = maxOffX;
        if (cameraOffset.y > maxOffY) cameraOffset.y = maxOffY;

        Vector2 mouse = GetMousePosition();
        Vector2 mapMouse = { mouse.x + cameraOffset.x, mouse.y + cameraOffset.y };

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (mouse.x >= gridAreaWidth) {
                int rowInPalette = (int)((mouse.y - 40) / 40);
                if (rowInPalette >= 0 && rowInPalette < ITEMS_PER_PAGE) {
                    int objIndex = palettePage * ITEMS_PER_PAGE + rowInPalette;
                    if (objIndex >= 0 && objIndex < config->count_objects) {
                        currentTile = config->objects[objIndex].id;
                    }
                }
            } else {
                int col = (int)(mapMouse.x / cellSize);
                int row = (int)(mapMouse.y / cellSize);
                if (row >= 0 && row < config->layout->rows &&
                    col >= 0 && col < config->layout->cols) {
                    config->layout->data[row][col] = currentTile;
                }
            }
        }

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

        int startCol = (int)(cameraOffset.x / cellSize);
        int endCol = startCol + (int)(gridAreaWidth / cellSize) + 2;
        int startRow = (int)(cameraOffset.y / cellSize);
        int endRow = startRow + (int)(screenHeight / cellSize) + 2;
        if (startCol < 0) startCol = 0;
        if (startRow < 0) startRow = 0;
        if (endCol > config->layout->cols) endCol = config->layout->cols;
        if (endRow > config->layout->rows) endRow = config->layout->rows;

        for (int row = startRow; row < endRow; row++) {
            for (int col = startCol; col < endCol; col++) {
                int id = config->layout->data[row][col];
                Rectangle dest = { col * cellSize - cameraOffset.x,
                                   row * cellSize - cameraOffset.y,
                                   cellSize, cellSize };

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

        if (mouse.x < gridAreaWidth) {
            int col = (int)(mapMouse.x / cellSize);
            int row = (int)(mapMouse.y / cellSize);
            if (row >= 0 && row < config->layout->rows && col >= 0 && col < config->layout->cols) {
                Rectangle r = { col * cellSize - cameraOffset.x,
                                row * cellSize - cameraOffset.y,
                                cellSize, cellSize };
                DrawRectangleLinesEx(r, 2, YELLOW);
            }
        }

        DrawRectangle(gridAreaWidth, 0, PALETTE_WIDTH, screenHeight, (Color){30,30,30,255});
        DrawText(TextFormat("PALETTE %d/%d", palettePage+1, totalPages), gridAreaWidth + 10, 10, 16, RAYWHITE);

        int startIdx = palettePage * ITEMS_PER_PAGE;
        int endIdx = startIdx + ITEMS_PER_PAGE;
        if (endIdx > config->count_objects) endIdx = config->count_objects;

        for (int i = startIdx; i < endIdx; i++) {
            int displayRow = i - startIdx;
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

        DrawText("Ctrl+S: Save   G: Grid   Arrows: Move   A/D: Palette page", 10, screenHeight - 25, 16, LIGHTGRAY);
        EndDrawing();
    }

    free_texture_manager(&textureManager);
    free_level_config(config);
    CloseWindow();
    return 0;
}