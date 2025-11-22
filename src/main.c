#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include "raylib.h"

char command[256] = "";
int cmd_pos = 0;

char last_command[256] = "";

char output_buf[BUFSIZ] = "";
int output_len = 0;

void updateInput(int pipe_to);
char* clear_str(char *output_start);
void drawGame();

int main() {
    InitWindow(800, 600, "IPC");

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
            updateInput(stdin_pipe[1]);
            /*----------конец обработки ввода----------*/

            /*----------начало чтения----------*/
            char buffer[1024];
            ssize_t bytes = read(stdout_pipe[0], buffer, sizeof(buffer)-1);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                char *cleaned = clear_str(buffer);

                size_t clean_len = strlen(cleaned);
                if (output_len + clean_len < sizeof(output_buf) - 1) {
                    strcat(output_buf, cleaned);
                    output_len += clean_len;
                } else {
                    memset(output_buf, 0, BUFSIZ);
                    output_len = 0;
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

            drawGame();
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

void updateInput(int pipe_to) {
    int key = GetCharPressed();
    if ((key > 0) && (cmd_pos < sizeof(command) - 1)) {
        command[cmd_pos++] = key;
    }

    if (IsKeyPressed(KEY_ENTER)) {
        command[cmd_pos] = '\0';
                    
        if (strcmp(command, "exit") == 0) {
            //break;
        }
                    
        strcpy(last_command, command);

        output_buf[0] = '\0';
        write(pipe_to, command, cmd_pos);
        write(pipe_to, "\n", 1);

        memset(command, 0, 256);

        cmd_pos = 0;
    }

    if (IsKeyPressed(KEY_BACKSPACE) && cmd_pos > 0) {
        command[--cmd_pos] = '\0';
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

void drawGame() {
    BeginDrawing();
        ClearBackground(BLACK);
        
        if (strstr(output_buf, last_command)) {
            char* res = strstr(output_buf, last_command) + strlen(last_command);
            strcpy(output_buf, res);
        }

        DrawText(output_buf, 10, 40, 24, GREEN);
        DrawText(command, 10, 10, 24, GREEN);
    EndDrawing();
}