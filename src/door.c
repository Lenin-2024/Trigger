#include <stdio.h>
#include <stdlib.h>

#include "door.h"

void init_door(door_t *cdoor, Vector2 pos, int num) {
    cdoor[num].pos = pos;
    cdoor[num].is_open = 0;
    cdoor[num].max_height = pos.y - 32;
    sprintf(cdoor[num].num, "%d", num);
}

void update_door(door_t *cdoor) {
    printf("door.y = %f | door->max_heifht = %f\n", cdoor->pos.y, cdoor->max_height);
    if (cdoor->is_open && (cdoor->pos.y > cdoor->max_height)) {
        cdoor->pos.y -= 0.25f;
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