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
#include "MQTTClient.h"

#define ADDRESS     "tcp://192.168.3.1:1883"
#define CLIENTID    "SimplePoll"
#define TOPIC       "door"
#define QOS         0

const int width = 800;
const int height = 600;

MQTTClient client;
MQTTClient_connectOptions conn_opts;
MQTTClient_message *message = NULL;
char *topic = NULL;
int topicLen;
int rc;

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

    /* Инициализация mqtt */
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    MQTTClient_create(&client, ADDRESS, CLIENTID,
                      MQTTCLIENT_PERSISTENCE_NONE, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Ошибка подключения: %d\n", rc);
        return -1;
    }

    MQTTClient_subscribe(client, TOPIC, QOS); // подписка на топик

    menu_init(width, height);
    game->game_run = 0;
}

void update(game_state_t *game) {
    while (!WindowShouldClose()) {
        rc = MQTTClient_receive(client, &topic, &topicLen, &message, 1); // ожидаем сообщение
        if (rc == MQTTCLIENT_SUCCESS && message != NULL) {
            if (message->payloadlen == 1) {
                char value = ((char*)message->payload)[0];
                printf("Получено: %c\n", value);
            }
            
            MQTTClient_freeMessage(&message);
            MQTTClient_free(topic);
        }

        if (game->game_run == 1) { 
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

                /* Сеню паузы */
                if (IsKeyPressed(KEY_Q)) {
                    game->game_run = 2;
                }
            }
        } else if (game->game_run == -1) {
            break;
        }

        draw(game);
    }
}

void draw(game_state_t *game) {
    BeginDrawing();
        ClearBackground(BLACK);
        DrawFPS(0, 0);
        
        switch (game->game_run) {
            case 0:
                draw_start_menu(&game->game_run);
                break;
            case 1:
                if (game->temu_run) {
                    draw_console(&game->console);
                } else {
                    draw_player(game->player);   
                }
                break;
            case 2:
                draw_pause_menu(&game->game_run);
                break;
            default:
                break;
        }        
    EndDrawing();
}

void cleanup(game_state_t *game) {
    close(game->stdin_pipe[1]);
    close(game->stdout_pipe[0]);

    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);

    unload_menu();

    int status;
    if (waitpid(game->temu_pid, &status, 0)) {
        printf("[INFO] Temu завершился с статусом - %d\n", WEXITSTATUS(status));
    }

    CloseWindow();
}