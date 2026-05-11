#include <string.h>
#include "engine/engine_context.h"
#include "map.h"
#include "player.h"
#include "door.h"

static void game_load(engine_context_t *eng) {
    eng->current_map = get_map(eng, "maps/file.json");
    if (!eng->current_map) {
        fprintf(stderr, "[ ERROR ] Failed to load map\n");
        eng->game_state = GAME_STOP;
    }
}

static void game_update(engine_context_t *eng) {
    if (eng->entity_manager) {
        update_all_entities(eng); 
    }
}

static void game_draw(engine_context_t *eng) {
    if (eng->current_map) {
        draw_map(eng, eng->current_map);
    }

    if (eng->entity_manager) {
        draw_all_entities(eng);
    }
}

static void game_unload(engine_context_t *eng) {
    if (eng->current_map) free_map(eng, eng->current_map);
}

int main(int argc, char **argv) {
    engine_context_t engine;
    memset(&engine, 0, sizeof(engine_context_t));

    engine_set_game_callbacks(&engine, game_load, game_update, game_draw, game_unload);

    // Иницилизация игры
    if (engine_init(&engine, 800, 600) != 0) {
        return 1;
    }

    while (!WindowShouldClose() && engine.game_state != GAME_EXIT) {
        engine_update(&engine);
        engine_draw(&engine);
    }

    engine_shutdown(&engine);

    return 0;
}