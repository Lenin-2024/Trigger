#include "engine/engine_context.h"
#include "map.h"
#include "player.h"
#include "door.h"

static void game_load(engine_context_t *eng) {
    eng->current_map = get_map(eng, "maps/file.json");
    if (!eng->current_map) {
        fprintf(stderr, "[ ERROR ] Failed to load map\n");
        eng->game_run = -1;
    }
}

static void game_update(engine_context_t *eng) {
    // TODO
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
    if (engine_init(&engine, 800, 600) != 0) return 1;
    engine_set_game_callbacks(&engine, game_load, game_update, game_draw, game_unload);
    if (engine.game_load) engine.game_load(&engine);
    while (!WindowShouldClose() && engine.game_run != -1) {
        engine_update(&engine);
        engine_draw(&engine);
    }
    engine_shutdown(&engine);

    return 0;
}