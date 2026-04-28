#ifndef __ENGINE_CONTEXT_H__
#define __ENGINE_CONTEXT_H__

#include <unistd.h>
#include <wait.h>

#include "../config/config.h"
#include "texture_manager.h"
#include "engine.h"
#include "console.h"
#include "../../net-prog/paho.mqtt.c/src/MQTTAsync.h"

#define ADDRESS     "tcp://192.168.3.1:1883"
#define CLIENTID    "SimplePoll"
#define QOS         0

typedef enum {
    MSG_UNKNOWN = 0,
    MSG_DOOR,
    MSG_BOX
} engine_msg_t;

typedef struct engine_context {
    int game_run;
    
    // Ресурсы
    level_config_t *current_map;   
    texture_manager_t texture_manager;
    
    // Сущьности
    entity_manager_t *entity_manager;

    // Консоль и эмулятор
    console_t console;
    int stdin_pipe[2];
    int stdout_pipe[2];
    pid_t temu_pid;
    int temu_run;

    int screen_width, screen_height;

    // MQTT
    MQTTAsync mqtt_client;
    MQTTAsync_connectOptions conn_opts;

    void (*game_update)(struct engine_context *engine);
    void (*game_draw)(struct engine_context *engine);
    void (*game_load)(struct engine_context *engine);
    void (*game_unload)(struct engine_context *engine);
} engine_context_t;

void engine_set_game_callbacks(engine_context_t *engine,
    void (*load)(engine_context_t*),
    void (*update)(engine_context_t*),
    void (*draw)(engine_context_t*),
    void (*unload)(engine_context_t*)
);

int engine_init(engine_context_t *engine, int width, int height);
void engine_shutdown(engine_context_t *engine);
void engine_update(engine_context_t *engine);
void engine_draw(engine_context_t *engine);

#endif /* __ENGINE_CONTEXT_H__ */