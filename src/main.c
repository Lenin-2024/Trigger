#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include "raylib.h"


struct console {
    char command[256];
    int cmd_pos;

    char last_command[256];

    char output_buf[BUFSIZ];
    int output_len;
};

typedef struct console console_t;

/*
char command[256] = "";
int cmd_pos = 0;

char last_command[256] = "";

char output_buf[BUFSIZ] = "";
int output_len = 0;
*/

void updateInput(int pipe_to, console_t* consol);
char* clear_str(char *output_start);
void drawGame(console_t* consol);

int main() {
    InitWindow(800, 600, "IPC");
    console_t consol = {0};

    int stdin_pipe[2], stdout_pipe[2];    
    if (pipe(stdin_pipe) == -1 || pipe(stdout_pipe) == -1) {
        perror("pipe");
        exit(1);
    }

    fcntl(stdout_pipe[0], F_SETFL, O_NONBLOCK);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    pid_t pid = fork();
    if (pid == 0) {
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);

        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stdout_pipe[1], STDERR_FILENO);

        close(stdin_pipe[0]);
        close(stdout_pipe[1]);

        execl("./tinyemu-2019-12-21/temu", "./temu", "-ctrlc", "./conf/root-riscv64.cfg", NULL);
        perror("execl");
        exit(1);
    } else {
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);

        printf("Temu запущен (PID: %d)\n", pid);
        printf("Готов к приему команд:\n");

        while (!WindowShouldClose()) {
            /*----------обработка ввода от пользователя----------*/
            updateInput(stdin_pipe[1], &consol);
            /*----------конец обработки ввода----------*/

            /*----------начало чтения----------*/
            char buffer[1024];
            ssize_t bytes = read(stdout_pipe[0], buffer, sizeof(buffer)-1);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                char *cleaned = clear_str(buffer);

                size_t clean_len = strlen(cleaned);
                if (consol.output_len + clean_len < sizeof(consol.output_buf) - 1) {
                    strcat(consol.output_buf, cleaned);
                    consol.output_len += clean_len;
                } else {
                    memset(consol.output_buf, 0, BUFSIZ);
                    consol.output_len = 0;
                }

                // printf("cleaned: '%s'\n", cleaned);
                // fflush(stdout);
            } else if (bytes < 0 && errno != EAGAIN) {
                perror("read stdout");
                break;
            } else if (bytes == 0) {
                printf("Temu завершил работу\n");
                break;
            }
            /*----------завершение чтения----------*/

            drawGame(&consol);
        }

        close(stdin_pipe[1]);
        close(stdout_pipe[0]);

        int status;
        if (waitpid(pid, &status, 0)) {
            printf("Child exit status - %d\n", WEXITSTATUS(status));
        }
        printf("Temu завершен.\n");
    }

    CloseWindow();
    return 0;
}

void updateInput(int pipe_to, console_t* consol) {
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

void drawGame(console_t* consol) {
    BeginDrawing();
        ClearBackground(BLACK);
        
        if (strstr(consol->output_buf, consol->last_command)) {
            char* res = strstr(consol->output_buf, consol->last_command) + strlen(consol->last_command);
            strcpy(consol->output_buf, res);
        }

        DrawText(consol->output_buf, 10, 40, 24, GREEN);
        DrawText(consol->command, 10, 10, 24, GREEN);
    EndDrawing();
}