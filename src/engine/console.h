#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <stdio.h>

struct console {
    char command[256];
    int cmd_pos;
    char last_command[256];
    char output_buf[BUFSIZ];
    int output_len;
};
typedef struct console console_t;

void update_input(int pipe_to, console_t* consol, int *temu_run);
char* clear_str(char *output_start);
void draw_console(console_t* consol);

#endif // __CONSOLE_H__