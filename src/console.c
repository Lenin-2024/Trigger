#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "console.h"
#include "raylib.h"

void update_input(int pipe_to, console_t* consol) {
    int key = GetCharPressed();
    if ((key > 0) && (consol->cmd_pos < sizeof(consol->command) - 1)) {
        consol->command[consol->cmd_pos++] = key;
    }

    if (IsKeyPressed(KEY_ENTER)) {
        consol->command[consol->cmd_pos] = '\0';
                    
        if (strcmp(consol->command, "exit") == 0) {
            //break;
        }
                    
        strcpy(consol->last_command, consol->command);

        consol->output_buf[0] = '\0';
        write(pipe_to, consol->command, consol->cmd_pos);
        write(pipe_to, "\n", 1);

        memset(consol->command, 0, 256);

        consol->cmd_pos = 0;
    }

    if (IsKeyPressed(KEY_BACKSPACE) && consol->cmd_pos > 0) {
        consol->command[--consol->cmd_pos] = '\0';
    }
}

char* clear_str(char *output_start) {
    char *result = output_start;
    char *src = output_start;
    char *dst = output_start;
    int in_escape = 0;
    
    char *tilde_pos = strchr(output_start, '~');
    if (tilde_pos != NULL) {
        *tilde_pos = '\0';
    }    

    char *invite = strstr(output_start, "/ #");
    if (invite != NULL) {
        *invite = '\0';
    } 
    
    while (*src) {
        if (in_escape) {
            if (*src == 'm') {
                in_escape = 0;
            }
            src++;
            continue;
        }
        
        if (*src == '\033' && *(src + 1) == '[') {
            in_escape = 1;
            src += 2;
            continue;
        }

        if (*src >= 0 && *src < 32 && *src != '\n') {
            src++;
        }

        *dst++ = *src++;
    }
    *dst = '\0';

    return result;
}

void draw_console(console_t* consol) {        
    if (strstr(consol->output_buf, consol->last_command)) {
        char* res = strstr(consol->output_buf, consol->last_command) + strlen(consol->last_command);
        strcpy(consol->output_buf, res);
    }

    DrawText(consol->output_buf, 10, 40, 24, GREEN);
    DrawText(consol->command, 10, 10, 24, GREEN);
}