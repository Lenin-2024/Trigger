#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "menu.h"

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

void menu_init(int width, int height) {
    menu_box_texture = LoadTexture("resources/menu/BoxesBanners/Box_Orange_Rounded.png");
    if (menu_box_texture.width == 0 || menu_box_texture.height == 0) {
        fprintf(stderr, "Не удалось загрузить текстуру %s!\n", "resources/menu/BoxesBanners/Box_Orange_Rounded.png");
        exit(1);
    }
    menu_box_width = menu_box_texture.width * 0.35;
    menu_box_height = menu_box_texture.height * 0.35;
    menu_box_start_x = (width / 2) - ((menu_box_width / 2));
    menu_box_start_y = (height / 2) - ((menu_box_height / 2));

    menu_start_button_texture = LoadTexture("resources/menu/ButtonsText/ButtonText_Large_Orange_Round.png");
    if (menu_start_button_texture.width == 0 || menu_start_button_texture.height == 0) {
        fprintf(stderr, "Не удалось загрузить текстуру %s!\n", "resources/menu/ButtonsText/ButtonText_Large_Orange_Round.png");
        exit(1);
    }
    menu_start_button_width = menu_start_button_texture.width * 0.3;
    menu_start_button_height = menu_start_button_texture.height * 0.3;
    menu_start_button_start_x = (width / 2) - ((menu_start_button_width / 2));
    menu_start_button_start_y = (height / 2) - ((menu_start_button_height / 2));

    menu_exit_game_button_texture = LoadTexture("resources/menu/ButtonsText/PremadeButtons_ExitOrange.png");
    if (menu_exit_game_button_texture.width == 0 || menu_exit_game_button_texture.height == 0) {
        fprintf(stderr, "Не удалось загрузить текстуру %s!\n", "resources/menu/ButtonsText/PremadeButtons_ExitOrange.png");
        exit(1);
    }
    menu_exit_game_button_width = menu_exit_game_button_texture.width * 0.3;
    menu_exit_game_button_height = menu_exit_game_button_texture.height * 0.3;
    menu_exit_game_button_start_x = (width / 2) - ((menu_exit_game_button_width / 2));
    menu_exit_game_button_start_y = menu_start_button_start_y + menu_start_button_height + 20;
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

void draw_start_menu(int *start_game) {
    DrawTextureEx(menu_box_texture, (Vector2){ menu_box_start_x, menu_box_start_y}, 0, 0.35, WHITE);
    DrawTextureEx(menu_start_button_texture, (Vector2){ menu_start_button_start_x, menu_start_button_start_y}, 0, 0.3, WHITE);
    DrawTextureEx(menu_exit_game_button_texture, (Vector2){ menu_exit_game_button_start_x, menu_exit_game_button_start_y}, 0, 0.3, WHITE);

    /* Проверка нажатия кнопки "Начать игру" */
    if (check_button_click(menu_start_button_start_x, menu_start_button_start_y, menu_start_button_width, menu_start_button_height)) {
        *start_game = 1;
    }

    /* Проверка нажатия кнопки "Выйти из игры" */
    if (check_button_click(menu_exit_game_button_start_x, menu_exit_game_button_start_y, menu_exit_game_button_width, menu_exit_game_button_height)) {
        *start_game = -1;
    }

#if DEBUG_MENU_MODE
    DrawRectangleLines(menu_start_button_start_x, menu_start_button_start_y, menu_start_button_width, menu_start_button_height, RED);
    DrawRectangleLines(menu_exit_game_button_start_x, menu_exit_game_button_start_y, menu_exit_game_button_width, menu_exit_game_button_height, RED);
#endif

}

void draw_pause_menu() {
    
}

void draw_settings_menu() {

}

void unload_menu() {
    UnloadTexture(menu_box_texture);
    UnloadTexture(menu_start_button_texture);
    UnloadTexture(menu_exit_game_button_texture);
}