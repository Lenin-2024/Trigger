#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "raymath.h"
#include "player.h"
#include "map.h"
#include "engien/engien.h"

#define DEBUG_PLAYER_MODE 1
#define TILE_SIZE 32

extern level_config_t *map;

Texture2D player_texture;
int frame = 0;
int max_frames = 10;

int count_jump_frame = 6;

int scale = 2;
const int speed = 1;

const object_v_table_t player_vtable = {
    .update = player_entity_update,
    .draw = player_entity_draw,
    .destroy = player_entity_destroy
};

player_t *create_player(Vector2 pos) {
    player_t *player = (player_t *)malloc(sizeof(player_t));
    if (!player) {
        return NULL;
    }

    player_texture = LoadTexture("resources/male_hero_template.png");
    if ((player_texture.height == 0) || (player_texture.width == 0)) {
        fprintf(stderr, "Не удалось загрузить текстуру %s!\n", "resources/male_hero_template.png");
        exit(1);
    }

    player->current_frame = 0;
    player->pos = pos;
    player->velocity = Vector2Zero();
    player->flip = 1;
    player->tile_size = player_texture.width / 10;
    player->on_ground = 0;
    player->state = IDLE;

    return player;
}

void update_player(player_t *player) {
    int moving_horizontally = 0;

    if (IsKeyDown(KEY_D)) {
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            player->velocity.x += speed * 3;
        } else {
            player->velocity.x += speed;
        }
        player->flip = 1;
        moving_horizontally = 1;
    }
    if (IsKeyDown(KEY_A)) {
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            player->velocity.x -= speed * 3;
        } else {
            player->velocity.x -= speed;
        }
        player->flip = -1;
        moving_horizontally = 1;
    }
    if (IsKeyPressed(KEY_SPACE) && player->on_ground) {
        player->velocity.y -= 6.0f;
        player->on_ground = 0;
        player->current_frame = 0;
    }

    player->velocity.y += fminf(player->velocity.y, 2.f) < 2.f ? 0.2f : 0.0f;
    if (player->velocity.y > 0) {
        player->on_ground = 0;
    }

    player->pos.x += player->velocity.x;
    check_collision_pl(map, player, 0);

    player->pos.y += player->velocity.y + 1;
    check_collision_pl(map, player, 1);

    if (!player->on_ground) {
        player->state = JUMP;
    } else if (moving_horizontally) {
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            player->state = RUN;
        } else {
            player->state = GO;
        }
    } else {
        player->state = IDLE;
    }

    frame++;
    if (frame % 6 == 0) {
        if (player->state == JUMP) {
            if ((player->current_frame + 1) % 6 == 0) {
                // TODO
            } else {
                player->current_frame = (player->current_frame + 1) % 6;
            }
        } else {
            player->current_frame = (player->current_frame + 1) % 10;
            frame = 0;
        }
    }

    player->velocity.x = 0;
}

void check_collision_pl(level_config_t *map, player_t *player, int dir) {
    int start_x = (int)(player->pos.x / TILE_SIZE);
    int start_y = (int)(player->pos.y / TILE_SIZE);
    int end_x   = (int)((player->pos.x + 32) / TILE_SIZE);
    int end_y   = (int)((player->pos.y + 64) / TILE_SIZE);

    for (int i = start_y; i <= end_y; i++) {
        for (int j = start_x; j <= end_x; j++) {
            if (i < 0 || i >= map->layout->rows || j < 0 || j >= map->layout->cols) {
                continue;
            }

            if (map->layout->data[i][j] > 0) {
                if ((player->velocity.x > 0) && (dir == 0)) {
                    player->pos.x = (j * TILE_SIZE) - 32 - 1;
                    player->velocity.x = 0;
                }

                if ((player->velocity.x < 0) && (dir == 0)) {
                    player->pos.x = (j * TILE_SIZE) + 32 + 1;
                    player->velocity.x = 0;
                }

                if ((player->velocity.y > 0) && (dir == 1)) {
                    player->pos.y = (i * TILE_SIZE) - 64 - 1;
                    player->on_ground = 1;
                    player->velocity.y = 0;
                }

                if ((player->velocity.y < 0) && (dir == 1)) {
                    player->pos.y = (i * TILE_SIZE) + TILE_SIZE;
                    player->on_ground = 0;
                    player->velocity.y = 0;
                }
            }
        }
    }
}

void draw_player(player_t player) {
#if DEBUG_PLAYER_MODE == 1
    char debug_txt[255];
    sprintf(debug_txt, "Player(x = %.2f  | y = %.2f and x1 = %.2f  | y2 = %.2f) \nonGround = %d vel.x = %.2f  vel.y = %.2f", 
                        player.pos.x, player.pos.y, 
                        player.pos.x + 32, player.pos.y + 64, 
                        player.on_ground,
                        player.velocity.x, player.velocity.y);
    DrawText(debug_txt, 0, 20, 20, GREEN);
    DrawRectangle(player.pos.x, player.pos.y, 32, 64, RED);
#endif
    // Кадр в персонажа
    Rectangle source_rec = {
        128 * player.current_frame, player.state * 128, player.flip * 128, 128
    };

    // Размер и позиция
    Rectangle dest_rec = {
        player.pos.x - 128 + 16, player.pos.y - 128 + 32, player.tile_size * scale, player.tile_size * scale
    };
    DrawTexturePro(player_texture, source_rec, dest_rec, (Vector2){0, 0}, 0, WHITE);
}

void player_entity_update(void *data) {
    player_t *player = (player_t*)data;
    update_player(player);
}

void player_entity_draw(void *data) {
    player_t *player = (player_t*)data;
    draw_player(*player);
}

void player_entity_destroy(void *data) {
    player_t *player = (player_t*)data;
    if (player) {
        free(player);
    }
}