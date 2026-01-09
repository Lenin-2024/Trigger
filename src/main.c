#include "game.h"

int main(int argc, char **argv) {
    game_state_t game;
    init(&game);
    update(&game);
    cleanup(&game);
    
    return 0;
}