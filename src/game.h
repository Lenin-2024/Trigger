#ifndef __GAME_H__
#define __GAME_H__

#include <unistd.h>

#include "console.h"
#include "player.h"

struct game_state {
    console_t console;
    int stdin_pipe[2];
    int stdout_pipe[2];
    pid_t temu_pid;
    int temu_run;

    int game_run;
    int level;

    player_t player;
    int show_console;
};
typedef struct game_state game_state_t;

void draw(game_state_t *game);
void update(game_state_t *game);
void init(game_state_t *game);
void cleanup(game_state_t *game);


#endif // __GAME_H__