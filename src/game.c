#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include "game.h"
#include "menu.h"

const int width = 800;
const int height = 600;

void init(game_state_t *game) {
    InitWindow(width, height, "IPC");
    SetTargetFPS(60);

    memset(game, 0, sizeof(game_state_t));
    game->temu_run = 0;
    init_player(&game->player, (Vector2){32, 32});

    if (pipe(game->stdin_pipe) == -1 || pipe(game->stdout_pipe) == -1) {
        perror("pipe");
        exit(1);
    }

    fcntl(game->stdout_pipe[0], F_SETFL, O_NONBLOCK);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    /* иницилизация tinyEMU (запуск нового потка) */
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

    menu_init(width, height);
    game->game_run = 0;
}

void update(game_state_t *game) {
    while (!WindowShouldClose()) {
        if (game->game_run) { 
            if (game->temu_run) {
                /* обработка чтения */
                update_input(game->stdin_pipe[1], &game->console, &game->temu_run);
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
            } else {
                if (IsKeyPressed(KEY_P)) {
                    game->temu_run = 1;
                }
                update_player(&game->player);
            }
        }

        /* отрисовка игры */
        draw(game);
    }
}

void draw(game_state_t *game) {
    BeginDrawing();
        ClearBackground(BLACK);
        if (game->game_run) {
            if (game->temu_run) {
                draw_console(&game->console);
            } else {
                draw_player(game->player);   
            }
        } else {
            draw_start_menu();
        }
    EndDrawing();
}

void cleanup(game_state_t *game) {
    close(game->stdin_pipe[1]);
    close(game->stdout_pipe[0]);

    unload_menu();

    int status;
    if (waitpid(game->temu_pid, &status, 0)) {
        printf("[INFO] Temu завершился с статусом - %d\n", WEXITSTATUS(status));
    }

    CloseWindow();
}