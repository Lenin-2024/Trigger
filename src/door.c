#include <stdio.h>
#include <stdlib.h>

#include "door.h"
#include "game.h"

void init_door(door_t *cdoor, Vector2 pos, int num) {
    cdoor[num].pos = pos;
    cdoor[num].is_open = 0;
    cdoor[num].max_height = pos.y - 32;
    sprintf(cdoor[num].num, "%d", num);
}

void update_door(game_state_t *game, int num) {
    if (game->doors[num].is_open && (game->doors[num].pos.y > game->doors[num].max_height)) {
        game->doors[num].pos.y -= 0.25f;
    }

    Rectangle player_rect = {
        game->player.pos.x, game->player.pos.y, 32, 64
    };
        
    Rectangle door_rect = {
        game->doors[num].pos.x, game->doors[num].pos.y,
        32, 32
    };

    if (CheckCollisionRecs(player_rect, door_rect)) {
        float dx_left = (player_rect.x + player_rect.width) - door_rect.x;
        float dx_right = (door_rect.x + door_rect.width) - player_rect.x;
        float dy_top = (player_rect.y + player_rect.height) - door_rect.y;
        float dy_bottom = (door_rect.y + door_rect.height) - player_rect.y;

        float min_x = (dx_left < dx_right) ? dx_left : dx_right;
        float min_y = (dy_top < dy_bottom) ? dy_top : dy_bottom;

        if (min_x < min_y) {
            if (dx_left < dx_right) {
                game->player.pos.x = door_rect.x - player_rect.width;
            } else {
                game->player.pos.x = door_rect.x + door_rect.width;
            }
        } else {
            if (dy_top < dy_bottom) {
                game->player.pos.y = door_rect.y - player_rect.height;
                game->player.on_ground = 1;
                game->player.velocity.y = 0;
            } else {
                game->player.pos.y = door_rect.y + door_rect.height;
                game->player.velocity.y = 0;
            }
        }
    }
}

void draw_door(door_t *cdoor) {
    DrawRectangle(cdoor->pos.x, cdoor->pos.y, 32, 32, RED);
    DrawText(cdoor->num, cdoor->pos.x + 8, cdoor->pos.y, 32, BLACK);
}

void free_door(door_t *cdoor) {
    if (cdoor) {
        free(cdoor);
    }
}