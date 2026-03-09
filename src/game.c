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
#include "door.h"
#include "map.h"
#include "MQTTAsync.h"

#define ADDRESS     "tcp://192.168.3.1:1883"
#define CLIENTID    "SimplePoll"
#define TOPIC       "door"
#define QOS         0

const int width = 800;
const int height = 600;

MQTTAsync client;
MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;

char *topic = NULL;
int topicLen;
int rc;

map_t *map;

// Функция обработки полученных сообщений
int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    game_state_t *game = (game_state_t*)context;

    int door_n, state;
    sscanf((char*)message->payload, "%d %d", &door_n, &state);
    printf("%d %d\n", door_n, state);
    if (door_n >= 0 && door_n < game->count_doors) {
        game->doors[door_n].is_open = state;
    }
    
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

void connlost(void *context, char *cause, game_state_t *game) {
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start connect, return code %d\n", rc);
        game->game_run = 0;
    }
}

void onConnectSuccess(void* context, MQTTAsync_successData* response) {
    game_state_t *game = (game_state_t*)context;
    printf("Connected to MQTT broker\n");
    
    // Подписка после успешного подключения
    MQTTAsync_responseOptions sub_opts = MQTTAsync_responseOptions_initializer;
    sub_opts.context = game;
    
    if (MQTTAsync_subscribe(client, TOPIC, QOS, &sub_opts) != MQTTASYNC_SUCCESS) {
        printf("Failed to subscribe\n");
    }
}

void onConnectFailure(void* context, MQTTAsync_failureData* response) {
    game_state_t *game = (game_state_t*)context;
    printf("Connect failed, rc %d\n", response ? response->code : 0);
    game->game_run = 0;
}

void init(game_state_t *game) {
    InitWindow(width, height, "Завод? Снова завон?! Не хочу на звод!");
    SetTargetFPS(60);

    memset(game, 0, sizeof(game_state_t));
    game->temu_run = 0;
    game->doors = NULL;
    game->count_doors = 0;

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
    MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);    
    MQTTAsync_setCallbacks(client, game, connlost, msgarrvd, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = onConnectSuccess;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = game;
    
    if (MQTTAsync_connect(client, &conn_opts) != MQTTASYNC_SUCCESS) {
        printf("Ошибка подключения\n");
        game->game_run = 0;
    }

    menu_init(width, height);

    map = get_map("maps/map1-1.lvl", game);
    printf("%d %d\n", map->rows, map->cols);

    game->game_run = 0;
}

void update(game_state_t *game) {
    while (!WindowShouldClose()) {
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
                update_player(&game->player, map);
                
                for (int i = 0; i < game->count_doors; i++) {
                    update_door(game, i);
                }

                /* Меню паузы */
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
        
        switch (game->game_run) {
            case 0:
                draw_start_menu(&game->game_run);
                break;
            case 1:
                if (game->temu_run) {
                    draw_console(&game->console);
                } else {
                    for (int i = 0; i < game->count_doors; i++) {
                        draw_door(game->doors + i);
                    }
                    draw_map(map);
                    draw_player(game->player);
                }
                break;
            case 2:
                draw_pause_menu(&game->game_run);
                break;
            default:
                break;
        }

    DrawFPS(0, 0);

    EndDrawing();
}

void cleanup(game_state_t *game) {
    /* Закрытие труб */
    if (game->stdin_pipe[1] > 0) {
        close(game->stdin_pipe[1]);
    }

    if (game->stdin_pipe[0] > 0) {
        close(game->stdout_pipe[0]);
    }

    /* Отключение mqtt */
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    disc_opts.onSuccess = NULL;
    disc_opts.timeout = 1000;
    
    rc = MQTTAsync_disconnect(client, &disc_opts);
    if (rc != MQTTASYNC_SUCCESS) {
        printf("Failed to disconnect MQTT, return code %d\n", rc);
    }
    MQTTAsync_destroy(&client);
    
    /* Выгрузка меню (текстур) */
    unload_menu();

    /* Отчистка дверей */
    free(game->doors);
    game->doors = NULL;
    game->count_doors = 0;

    free_map(map);

    int status;
    if (waitpid(game->temu_pid, &status, 0)) {
        printf("[INFO] Temu завершился с статусом - %d\n", WEXITSTATUS(status));
    }

    CloseWindow();
}