#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "menu.h"

Texture2D menu_box_texture;
int menu_box_width = 0;
int menu_box_height = 0;
int menu_box_start_x = 0;
int menu_box_start_y = 0;

void menu_init(int width, int height) {
    menu_box_texture = LoadTexture("resources/menu/BoxesBanners/Box_Orange_Rounded.png");
    menu_box_width = menu_box_texture.width;
    menu_box_height = menu_box_texture.height;
    menu_box_start_x = (width / 2) - ((menu_box_width / 2) * 0.35);
    menu_box_start_y = (height / 2) - ((menu_box_height / 2) * 0.35);



}

void draw_start_menu() {
    DrawTextureEx(menu_box_texture, (Vector2){ menu_box_start_x, menu_box_start_y}, 0, 0.35, WHITE);
}

void draw_pause_menu() {
    
}

void draw_settings_menu() {

}

void unload_menu() {
    UnloadTexture(menu_box_texture);
}