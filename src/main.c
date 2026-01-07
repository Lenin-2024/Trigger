#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include "raylib.h"
#include "console.h"

struct game_state {
    console_t console;
    int stdin_pipe[2];
    int stdout_pipe[2];
    pid_t temu_pid;
    int temu_run;

    int game_run;
    int level;

    int show_console;
};
typedef struct game_state game_state_t;

void draw(game_state_t *game);
void update(game_state_t *game);
void init(game_state_t *game);
void cleanup(game_state_t *game);

int main(int argc, char **argv) {
    game_state_t game;
    init(&game);

    update(&game);

    cleanup(&game);
    return 0;
}

void init(game_state_t *game) {
    InitWindow(800, 600, "IPC");
    SetTargetFPS(60);

    memset(game, 0, sizeof(game_state_t));

    if (pipe(game->stdin_pipe) == -1 || pipe(game->stdout_pipe) == -1) {
        perror("pipe");
        exit(1);
    }

    fcntl(game->stdout_pipe[0], F_SETFL, O_NONBLOCK);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    game->temu_pid = fork();
    if (game->temu_pid == 0) {
        close(game->stdin_pipe[1]);
        close(game->stdout_pipe[0]);

        dup2(game->stdin_pipe[0], STDIN_FILENO);
        dup2(game->stdout_pipe[1], STDOUT_FILENO);
        dup2(game->stdout_pipe[1], STDERR_FILENO);

        close(game->stdin_pipe[0]);
        close(game->stdout_pipe[1]);

        execl("./tinyemu-2019-12-21/temu", "./temu", "-ctrlc", "./conf/root-riscv64.cfg", NULL);
        perror("execl");
        exit(1);
    } else if (game->temu_pid > 0) {
        close(game->stdin_pipe[0]);
        close(game->stdout_pipe[1]);

        printf("[INFO] Temu запущен (PID: %d)\n", game->temu_pid);
    } else {
        fprintf(stderr, "Не удалось запустить эмулятор.\n");
        exit(1);
    }
}

void update(game_state_t *game) {
    while (!WindowShouldClose()) {
        update_input(game->stdin_pipe[1], &game->console);
        char buffer[1024];
        ssize_t bytes = read(game->stdout_pipe[0], buffer, sizeof(buffer)-1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            char *cleaned = clear_str(buffer);

            size_t clean_len = strlen(cleaned);
            if (game->console.output_len + clean_len < sizeof(game->console.output_buf) - 1) {
                strcat(game->console.output_buf, cleaned);
                game->console.output_len += clean_len;
            } else {
                memset(game->console.output_buf, 0, BUFSIZ);
                game->console.output_len = 0;
            }
        } else if (bytes < 0 && errno != EAGAIN) {
            perror("read stdout");
            break;
        } else if (bytes == 0) {
            printf("Temu завершил работу\n");
            break;
        }
        /*----------завершение чтения----------*/
        draw(game);
    }
}

void draw(game_state_t *game) {
    BeginDrawing();
        ClearBackground(BLACK);
        draw_console(&game->console);
    EndDrawing();
}

void cleanup(game_state_t *game) {
    close(game->stdin_pipe[1]);
    close(game->stdout_pipe[0]);

    int status;
    if (waitpid(game->temu_pid, &status, 0)) {
        printf("[INFO] Temu завершился с статусом - %d\n", WEXITSTATUS(status));
    }

    CloseWindow();
}