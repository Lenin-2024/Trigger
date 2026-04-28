#include "engine_context.h"

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "../menu.h"
#include "../door.h"

typedef struct {
    engine_msg_t id;
    const char *topic;
} topic_map_t;

static const topic_map_t TOPIC_REGISTRY[] = {
    { MSG_DOOR,     "door"   },
    { MSG_BOX,      "box"  },
    { MSG_UNKNOWN,  "unknow" }
};


/*----------------Прототипы внутренних функций---------------*/
static int engine_start_temu(engine_context_t *engine);
static void engine_update_console(engine_context_t *engine);
static engine_msg_t get_msg_id(const char* topicName);
static void onConnectSuccess(void* context, MQTTAsync_successData* response);
static void onConnectFailure(void* context, MQTTAsync_failureData* response);
static void connlost(void *context, char *cause);
int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message);


void engine_set_game_callbacks(engine_context_t *engine,
    void (*load)(engine_context_t*),
    void (*update)(engine_context_t*),
    void (*draw)(engine_context_t*),
    void (*unload)(engine_context_t*))
{
    engine->game_load = load;
    engine->game_update = update;
    engine->game_draw = draw;
    engine->game_unload = unload;
}


int engine_init(engine_context_t *engine, int width, int height) {
    memset(engine, 0, sizeof(engine_context_t));
    engine->screen_width = width;
    engine->screen_height = height;

    InitWindow(engine->screen_width, engine->screen_height, "Завод? Снова завон?! Не хочу на звод!");
    SetTargetFPS(60);

    memset(&engine->texture_manager, 0, sizeof(texture_manager_t));
    engine->entity_manager = craete_entity_manager(10);

    /* Инициализация mqtt */
    MQTTAsync_create(&engine->mqtt_client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTAsync_setCallbacks(engine->mqtt_client, engine, connlost, msgarrvd, NULL);

    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    engine->conn_opts = conn_opts;
    engine->conn_opts.keepAliveInterval = 20;
    engine->conn_opts.cleansession = 1;
    engine->conn_opts.onSuccess = onConnectSuccess;
    engine->conn_opts.onFailure = onConnectFailure;
    engine->conn_opts.context = engine;
    
    if (MQTTAsync_connect(engine->mqtt_client, &engine->conn_opts) != MQTTASYNC_SUCCESS) {
        fprintf(stderr, "[ ERROR ] MQTT connect failed\n");
        engine->game_run = -1;
    }

    /* Запуск эмулятора */
    engine_start_temu(engine); 

    menu_init(engine->screen_width, engine->screen_height);
    engine->game_run = 1;
    engine->temu_run = 0;

    return 0;
}

static int engine_start_temu(engine_context_t *engine) {
    if (pipe(engine->stdin_pipe) == -1 || pipe(engine->stdout_pipe) == -1) {
        perror("pipe");
        return 0;
    }
    fcntl(engine->stdout_pipe[0], F_SETFL, O_NONBLOCK);
    
    engine->temu_pid = fork();
    if (engine->temu_pid == 0) {
        // дочерний процесс
        close(engine->stdin_pipe[1]);
        close(engine->stdout_pipe[0]);

        dup2(engine->stdin_pipe[0], STDIN_FILENO);
        dup2(engine->stdout_pipe[1], STDOUT_FILENO);
        dup2(engine->stdout_pipe[1], STDERR_FILENO);

        close(engine->stdin_pipe[0]);
        close(engine->stdout_pipe[1]);

        execl("./tinyemu-2019-12-21/temu", "./temu", "-ctrlc", "./conf/root-riscv64.cfg", NULL);
        perror("execl");
        exit(1);
    } else if (engine->temu_pid > 0) {
        close(engine->stdin_pipe[0]);
        close(engine->stdout_pipe[1]);
        printf("[ INFO ] Temu запущен (PID: %d)\n", engine->temu_pid);
        return 0;
    } else {
        perror("fork");
        return 1;
    }
}

void engine_shutdown(engine_context_t *engine) {
    if (engine->temu_pid > 0) {
        kill(engine->temu_pid, SIGTERM);
        waitpid(engine->temu_pid, NULL, 0);
    }

    /* Закрытие каналов (если они открыты) */
    if (engine->stdin_pipe[0] > 0) {
        close(engine->stdin_pipe[0]);
    }

    if (engine->stdin_pipe[1] > 0) { 
        close(engine->stdin_pipe[1]);
    }

    if (engine->stdout_pipe[0] > 0) { 
        close(engine->stdout_pipe[0]);
    }

    if (engine->stdout_pipe[1] > 0) {
        close(engine->stdout_pipe[1]);
    }

    /* Отключение Mqtt */
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    disc_opts.timeout = 1000;
    MQTTAsync_disconnect(engine->mqtt_client, &disc_opts);
    MQTTAsync_destroy(&engine->mqtt_client);

    /* Освобождение сущьностей */
    if (engine->entity_manager) {
        destroy_entity(engine->entity_manager);
        free(engine->entity_manager->entities);
        free(engine->entity_manager);
    }

    /* Выгрузка карты и текстур */
    if (engine->current_map) {
        free_texture_manager(&engine->texture_manager);
        free_level_config(engine->current_map);
    }

    unload_menu();
    CloseWindow();
}

void engine_update(engine_context_t *engine) {
    if (engine->temu_run) {
        update_input(engine->stdin_pipe[1], &engine->console, &engine->temu_run);
        engine_update_console(engine);
    } else {
        if (engine->game_run == 1) {
            if (IsKeyPressed(KEY_Q)) { 
                engine->game_run = 2;
            }

            if (IsKeyPressed(KEY_P)) {
                engine->temu_run = 1;
            }

            if (engine->game_update) {
                engine->game_update(engine);
            }

            if (engine->entity_manager) {
                update_all_entities(engine); 
            }
        }
    }
}

void engine_draw(engine_context_t *engine) {
    BeginDrawing();
    ClearBackground(BLACK);
    switch (engine->game_run) {
        case 0: 
            draw_start_menu(&engine->game_run); 
            break;
        case 1:
            if (engine->temu_run) {
                draw_console(&engine->console);
            } else if (engine->game_draw) {

                engine->game_draw(engine);
            }
            break;
        case 2:
            draw_pause_menu(&engine->game_run);
            break;
        default:
            break;
    }

    DrawFPS(0, 0);
    EndDrawing();
}

void engine_update_console(engine_context_t* engine) {
    char buffer[1024];
    ssize_t bytes = read(engine->stdout_pipe[0], buffer, sizeof(buffer)-1);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        char *cleaned = clear_str(buffer);

        size_t clean_len = strlen(cleaned);
        if (engine->console.output_len + clean_len < sizeof(engine->console.output_buf) - 1) {
            strcat(engine->console.output_buf, cleaned);
            engine->console.output_len += clean_len;
        } else {
            memset(engine->console.output_buf, 0, BUFSIZ);
            engine->console.output_len = 0;
        }
    } else if (bytes < 0 && errno != EAGAIN) {
        perror("read stdout");
        engine->temu_run = 0;
        // break;
    } else if (bytes == 0) {
        engine->temu_run = 0;
        printf("Temu завершил работу\n");
        // break;
    }
}

/*-----------------Mqtt функции----------------*/
/* Функция переводящая сроковое имя топика в число (id) из enum engine_msg_t */
engine_msg_t get_msg_id(const char* topicName) {
    for (int i = 0; i < (int)(sizeof(TOPIC_REGISTRY) / sizeof(TOPIC_REGISTRY[0])); i++) {
        if (strcmp(topicName, TOPIC_REGISTRY[i].topic) == 0) {
            return TOPIC_REGISTRY[i].id;
        }
    }
    return MSG_UNKNOWN;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    engine_context_t *engine = (engine_context_t*)context;
    switch (get_msg_id(topicName)) {
        case MSG_DOOR: {
            int door_n, state;
            sscanf((char*)message->payload, "%d %d", &door_n, &state);
            printf("[ INFO ] door %d state %d\n", door_n, state);
            door_t *door = find_door_by_id(engine, door_n);
            if (door) {
                door->is_open = state;
            }

            break;
        }
        case MSG_BOX:
            printf("[ INFO ] box msg = %s\n", (char *)message->payload);
            break;
        default:
            break;
    }
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

/* Подключение к брокеру */
static void onConnectSuccess(void* context, MQTTAsync_successData* response) {
    engine_context_t *engine = (engine_context_t*)context;
    printf("[ INFO ] Try to connected to MQTT broker\n");
    
    MQTTAsync_responseOptions sub_opts = MQTTAsync_responseOptions_initializer;
    sub_opts.context = engine;
    
    int num_topic = sizeof(TOPIC_REGISTRY) / sizeof(TOPIC_REGISTRY[0]);
    for (int i = 0; i < num_topic; i++) {
        if (TOPIC_REGISTRY[i].id == MSG_UNKNOWN) {
            continue;
        }
        
        if (MQTTAsync_subscribe(engine->mqtt_client, TOPIC_REGISTRY[i].topic, QOS, &sub_opts) != MQTTASYNC_SUCCESS) {
            printf("[ ERROR ] Failed to subscribe to %s\n", TOPIC_REGISTRY[i].topic);
        } else {
            printf("[ INFO ] Subscribe to %s\n", TOPIC_REGISTRY[i].topic);
        }
    }
}

static void onConnectFailure(void* context, MQTTAsync_failureData* response) {
    engine_context_t *engine = (engine_context_t*)context;
    printf("[ ERROR ] MQTT connect failed, rc %d\n", response ? response->code : 0);
    engine->game_run = -1;
}

static void connlost(void *context, char *cause) {
    engine_context_t *engine = (engine_context_t*)context;
    printf("[ WARNING ] MQTT connection lost: %s\n", cause);
    /* Попытка переподключения */
    if (MQTTAsync_connect(engine->mqtt_client, &engine->conn_opts) != MQTTASYNC_SUCCESS) {
        engine->game_run = -1;
    }
}
/*---------------------------------------------------------------------------------*/

void update_all_entities(struct engine_context *engine) {
    if (!engine || !engine->entity_manager) {
        return;
    }
    entity_manager_t *manager = engine->entity_manager;
    for (int i = 0; i < manager->count; i++) {
        entity_t *e = manager->entities[i];
        if (e->active && e->vtable->update)
            e->vtable->update(e->data, engine);
    }
}

void draw_all_entities(struct engine_context *engine) {
    if (!engine || !engine->entity_manager) {
        return;
    }

    entity_manager_t *manager = engine->entity_manager;
    for (int i = 0; i < manager->count; i++) {
        entity_t *e = manager->entities[i];
        if (e->active && e->vtable->draw)
            e->vtable->draw(e->data, engine);
    }
}