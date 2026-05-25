#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "raylib.h"
#include "menu.h"
#include "engine/engine_context.h"
#include "player.h"

#define DEBUG_MENU_MODE 0

Texture2D menu_box_texture;
int menu_box_width = 0;
int menu_box_height = 0;
int menu_box_start_x = 0;
int menu_box_start_y = 0;

Texture2D menu_start_button_texture;
int menu_start_button_width = 0;
int menu_start_button_height = 0;
int menu_start_button_start_x = 0;
int menu_start_button_start_y = 0;

Texture2D menu_exit_game_button_texture;
int menu_exit_game_button_width = 0;
int menu_exit_game_button_height = 0;
int menu_exit_game_button_start_x = 0;
int menu_exit_game_button_start_y = 0;

Texture2D menu_resume_button_texture;
int menu_resume_button_width = 0;
int menu_resume_button_height = 0;
int menu_resume_button_start_x = 0;
int menu_resume_button_start_y = 0;

static char **level_files = NULL;        
static int level_count = 0;
static int level_select_index = 0;       
static bool level_select_active = false; // true, когда показываем список
static char selected_level_path[256];

static void scan_level_files(void) {
    // Очистка предыдущего списка
    if (level_files) {
        for (int i = 0; i < level_count; i++) free(level_files[i]);
        free(level_files);
        level_files = NULL;
    }
    level_count = 0;
    level_select_index = 0;

    const char *map_dir = "maps";


    FilePathList files = LoadDirectoryFiles(map_dir);
    int capacity = 0;
    for (int i = 0; i < files.count; i++) {
        const char *path = files.paths[i];
        if (IsFileExtension(path, ".json")) {
            if (level_count >= capacity) {
                capacity = (capacity == 0) ? 16 : capacity * 2;
                level_files = realloc(level_files, capacity * sizeof(char*));
            }
            const char *fname = GetFileName(path);

            char *dot = strrchr(fname, '.');
            size_t len = dot ? (size_t)(dot - fname) : strlen(fname);
            char *name_no_ext = malloc(len + 1);
            memcpy(name_no_ext, fname, len);
            name_no_ext[len] = '\0';
            level_files[level_count++] = name_no_ext;
        }
    }
    UnloadDirectoryFiles(files);
}

void menu_init(int width, int height) {
    menu_box_texture = LoadTexture("resources/menu/BoxesBanners/Box_Orange_Rounded.png");
    if ((menu_box_texture.width == 0) || (menu_box_texture.height == 0)) {
        fprintf(stderr, "Не удалось загрузить текстуру %s!\n", "resources/menu/BoxesBanners/Box_Orange_Rounded.png");
        exit(1);
    }
    menu_box_width = menu_box_texture.width * 0.35;
    menu_box_height = menu_box_texture.height * 0.35;
    menu_box_start_x = (width / 2) - ((menu_box_width / 2));
    menu_box_start_y = (height / 2) - ((menu_box_height / 2));

    menu_start_button_texture = LoadTexture("resources/menu/ButtonsText/ButtonText_Large_Orange_Round.png");
    if ((menu_start_button_texture.width == 0) || (menu_start_button_texture.height == 0)) {
        fprintf(stderr, "Не удалось загрузить текстуру %s!\n", "resources/menu/ButtonsText/ButtonText_Large_Orange_Round.png");
        exit(1);
    }
    menu_start_button_width = menu_start_button_texture.width * 0.3;
    menu_start_button_height = menu_start_button_texture.height * 0.3;
    menu_start_button_start_x = (width / 2) - ((menu_start_button_width / 2));
    menu_start_button_start_y = (height / 2) - ((menu_start_button_height / 2));

    menu_exit_game_button_texture = LoadTexture("resources/menu/ButtonsText/PremadeButtons_ExitOrange.png");
    if ((menu_exit_game_button_texture.width == 0) || (menu_exit_game_button_texture.height == 0)) {
        fprintf(stderr, "Не удалось загрузить текстуру %s!\n", "resources/menu/ButtonsText/PremadeButtons_ExitOrange.png");
        exit(1);
    }
    menu_exit_game_button_width = menu_exit_game_button_texture.width * 0.3;
    menu_exit_game_button_height = menu_exit_game_button_texture.height * 0.3;
    menu_exit_game_button_start_x = (width / 2) - ((menu_exit_game_button_width / 2));
    menu_exit_game_button_start_y = menu_start_button_start_y + menu_start_button_height + 20;

    menu_resume_button_texture = LoadTexture("resources/menu/ButtonsText/PremadeButtons_Resume.png");
    if ((menu_resume_button_texture.width == 0) || (menu_resume_button_texture.height == 0)) {
        fprintf(stderr, "Не удалось загрузить текстуру %s!\n", "resources/menu/ButtonsText/PremadeButtons_Resume.png");
        exit(1);
    }
    menu_resume_button_width = menu_resume_button_texture.width * 0.3;
    menu_resume_button_height = menu_resume_button_texture.height * 0.3;
    menu_resume_button_start_x = (width / 2) - ((menu_resume_button_width / 2));
    menu_resume_button_start_y = (height / 2) - ((menu_resume_button_height / 2));

    scan_level_files();
}

const char* get_selected_level_path(void) {
    return selected_level_path;
}

int check_button_click(int button_x, int button_y, int button_width, int button_height) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Rectangle button_rect = { button_x, button_y, button_width, button_height };
        if (CheckCollisionPointRec(GetMousePosition(), button_rect)) {
            return 1;
        }
    }
    return 0;
}

static void draw_level_select(int x, int y, int width, int height) {
    int item_height = 24;
    int items_visible = height / item_height;
    if (items_visible > level_count) items_visible = level_count;


    DrawRectangle(x, y, width, height, (Color){ 0, 0, 0, 200 });
    DrawText("Select level:", x + 10, y + 5, 20, RAYWHITE);


    Rectangle back_btn = { x + width - 50, y + 2, 45, 20 };
    DrawRectangleRec(back_btn, DARKGRAY);
    DrawText("Back", back_btn.x + 4, back_btn.y + 2, 14, RAYWHITE);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), back_btn)) {
        level_select_active = false;
        return;
    }


    for (int i = 0; i < items_visible; i++) {
        int idx = i;
        if (idx >= level_count) break;

        Rectangle item_rect = { x + 10, y + 30 + i * item_height, width - 20, item_height - 2 };
        Color bg = (idx == level_select_index) ? SKYBLUE : DARKGRAY;
        DrawRectangleRec(item_rect, bg);
        DrawText(level_files[idx], item_rect.x + 5, item_rect.y + 4, 16, WHITE);


        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), item_rect)) {
            level_select_index = idx;

            snprintf(selected_level_path, sizeof(selected_level_path), "maps/%s.json", level_files[idx]); 
            level_select_active = false;

        }
    }
}

void draw_start_menu(engine_context_t *eng) {
    if (level_select_active) {
        draw_level_select(10, 10, 220, 320);
        if (strlen(selected_level_path) > 0) { 
            eng->game_state = GAME_RUN;       
            eng->current_map = get_map(eng,selected_level_path);
            if (!eng->current_map) {
                fprintf(stderr, "[ ERROR ] Failed to load map\n");
                eng->game_state = GAME_STOP;
                return;
            }
            memset(&eng->camera, 0, sizeof(Camera2D));
            player_t *player = (player_t *)eng->entity_manager->entities[eng->entity_manager->player_idx]->data;
            eng->camera.target = player->pos;
            eng->camera.offset = (Vector2){
                eng->screen_width / 2.0f,
                eng->screen_height / 2.0f
            };
            eng->camera.rotation = 0.0f;
            eng->camera.zoom = 1.5f;
            eng->game_load(eng);
            printf("%s\n",selected_level_path); 

        }
        return;
    }

    DrawTextureEx(menu_box_texture, (Vector2){ menu_box_start_x, menu_box_start_y}, 0, 0.35, WHITE);
    DrawTextureEx(menu_start_button_texture, (Vector2){ menu_start_button_start_x, menu_start_button_start_y}, 0, 0.3, WHITE);
    DrawTextureEx(menu_exit_game_button_texture, (Vector2){ menu_exit_game_button_start_x, menu_exit_game_button_start_y}, 0, 0.3, WHITE);

    if (check_button_click(menu_start_button_start_x, menu_start_button_start_y, menu_start_button_width, menu_start_button_height)) {
        level_select_active = true;
        level_select_index = 0;
        selected_level_path[0] = '\0';
    }

    if (check_button_click(menu_exit_game_button_start_x, menu_exit_game_button_start_y, menu_exit_game_button_width, menu_exit_game_button_height)) {
        eng->game_state = GAME_EXIT;

    }

#if DEBUG_MENU_MODE
    DrawRectangleLines(menu_start_button_start_x, menu_start_button_start_y, menu_start_button_width, menu_start_button_height, RED);
    DrawRectangleLines(menu_exit_game_button_start_x, menu_exit_game_button_start_y, menu_exit_game_button_width, menu_exit_game_button_height, RED);
#endif
}

void draw_pause_menu(game_state_t *resume_game) {
    DrawTextureEx(menu_box_texture, (Vector2){ menu_box_start_x, menu_box_start_y}, 0, 0.35, WHITE);
    DrawTextureEx(menu_resume_button_texture, (Vector2){ menu_resume_button_start_x, menu_resume_button_start_y}, 0, 0.3, WHITE);
    DrawTextureEx(menu_exit_game_button_texture, (Vector2){ menu_exit_game_button_start_x, menu_exit_game_button_start_y}, 0, 0.3, WHITE);

    /* Проверка нажатия кнопки "Продолжить игру" */
    if (check_button_click(menu_resume_button_start_x, menu_resume_button_start_y, menu_resume_button_width, menu_resume_button_height)) {  
        *resume_game = GAME_RUN;
    }

    /* Проверка нажатия кнопки "Выйти в меню" */
    if (check_button_click(menu_exit_game_button_start_x, menu_exit_game_button_start_y, menu_exit_game_button_width, menu_exit_game_button_height)) {
        *resume_game = GAME_STOP;
    }

#if DEBUG_MENU_MODE
    DrawRectangleLines(menu_resume_button_start_x, menu_resume_button_start_y, menu_resume_button_width, menu_resume_button_height, RED);
    DrawRectangleLines(menu_exit_game_button_start_x, menu_exit_game_button_start_y, menu_exit_game_button_width, menu_exit_game_button_height, RED);
#endif 
}

void draw_settings_menu() {

}

void unload_menu() {
    UnloadTexture(menu_box_texture);
    UnloadTexture(menu_start_button_texture);
    UnloadTexture(menu_exit_game_button_texture);
}