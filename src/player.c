#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "raymath.h"
#include "player.h"

#define DEBUG_PLAYER_MODE 1

Texture2D player_texture;
int frame = 0;
int max_frames = 10;
int scale = 2;
const int speed = 1;

void init_player(player_t *player, Vector2 pos) {
    player_texture = LoadTexture("resources/male_hero_template.png");
    if ((player_texture.height == 0) || (player_texture.width == 0)) {
        fprintf(stderr, "Не удалось загрузить текстуру %s!\n", "resources/male_hero_template.png");
        exit(1);
    }

    player->current_frame = 0;
    player->pos = pos;
    player->velocity = (Vector2){0, 0};
    player->flip = 1;
    player->tile_size = player_texture.width / 10;
    player->state = IDLE;
}

void update_player(player_t *player) {
    player->velocity = Vector2Zero();
    if (IsKeyDown(KEY_D)) {
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            player->velocity.x += speed * 3;
            player->state = RUN;
        } else {
            player->velocity.x += speed;
            player->state = GO;
        }
        player->flip = 1;
    }
    if (IsKeyDown(KEY_A)) {
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            player->velocity.x -= speed * 3;
            player->state = RUN;
        } else {
            player->velocity.x -= speed;
            player->state = GO;
        }
        player->flip = -1;
    }

    if (player->velocity.x == 0) {
        player->state = IDLE;   
    }

    frame++;
    if (frame % 6 == 0) {
        player->current_frame = (player->current_frame + 1) % 10;
        frame = 0;
    }

    player->pos = Vector2Add(player->pos, player->velocity);
}

void draw_player(player_t player) {
#if DEBUG_PLAYER_MODE == 1
    char debug_txt[255];
    sprintf(debug_txt, "Player(x = %.2f  | y = %.2f) speed = %.2f", player.pos.x, player.pos.y, player.velocity.x);
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