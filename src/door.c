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
    printf("door.y = %f | door->max_heifht = %f\n", game->doors[num].pos.y, game->doors[num].max_height);
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

    if (CheckCollisionRecs(player_rect, door_rect) && 
        game->player.pos.y + 64 <= door_rect.y + door_rect.height + 5 &&
        game->player.velocity.y >= 0) {
            
            game->player.pos.y = door_rect.y - 64;
            game->player.on_ground = 1;
            game->player.velocity.y = 0;
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