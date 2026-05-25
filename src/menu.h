#ifndef __MENU_H__
#define __MENU_H__

#include "engine/engine_context.h"

void menu_init(int width, int height);
void draw_start_menu(engine_context_t *eng);
void draw_pause_menu(game_state_t *resume_game);
void draw_settings_menu();
void unload_menu();

#endif // __MENU_H__