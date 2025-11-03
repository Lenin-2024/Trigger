#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

int main() {
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

        char command[256] = "";
        int cmd_pos = 0;

        char last_command[256] = "";
        int expecting_output = 0;

        char output_buffer[8192] = "";
        int output_pos = 0;

        while (1) {
            char ch;
            ssize_t bytes = read(STDIN_FILENO, &ch, 1);

            if (bytes > 0) {
                if (ch == '\n' || ch == '\r') {
                    command[cmd_pos] = '\0';

                    if (strcmp(command, "exit") == 0) {
                        break;
                    }

                    strcpy(last_command, command);
                    expecting_output = 1;       // Ожидание вывода
                    output_buffer[0] = '\0';    // Отчистка буфера вывода
                    output_pos = 0;

                    write(stdin_pipe[1], command, cmd_pos);
                    write(stdin_pipe[1], "\n", 1);

                    cmd_pos = 0;
                } else if (cmd_pos < sizeof(command) - 1) {
                    command[cmd_pos++] = ch;
                }
            } else if (bytes < 0 && errno != EAGAIN) {
                perror("read stdin");
                break;
            }

            // Получение данных
            char buffer[1024];
            bytes = read(stdout_pipe[0], buffer, sizeof(buffer)-1);

            if (bytes > 0) {
                buffer[bytes] = '\0';

                if (expecting_output) {
                    strcat(output_buffer + output_pos, buffer);
                    output_pos += bytes;
                    
                    char *command_start = strstr(output_buffer, last_command);
                    if (command_start != NULL) {
                        char *output_start = command_start + strlen(last_command);

                        char *tilde_pos = strchr(output_start, '~');
                        char *result;
                        if (tilde_pos != NULL) {
                            result = output_start;
                            *tilde_pos = '\0';
                        } else {
                            result = output_start;
                        }

                        int len = strlen(result);
                        while (len > 0 && ((result[len-1] == '\r') || (result[len-1] == '\n') || (result[len-1] == ' '))) {
                            result[--len] = '\0';
                        }

                        while (*result == '\r' || *result == '\n' || *result == ' ') {
                            result++;
                        }

                        if (strlen(result) > 0) {
                            printf("%s\n", result);
                        } else {
                            // printf("(команда выполнена)\n");
                        }
                        expecting_output = 0;
                    }

                } else {
                    printf("%s", buffer);
                    fflush(stdout);
                }
            } else if (bytes < 0 && errno != EAGAIN) {
                perror("read stdout");
                break;
            } else if (bytes == 0) {
                printf("Temu завершил работу\n");
                break;
            }

            int status;
            if (waitpid(pid, &status, WNOHANG) == pid) {
                printf("Temu завершился\n");
                break;
            }   
        }

        printf("Завершение...\n");
        write(stdin_pipe[1], "poweroff\n", 9);

        usleep(100000);

        close(stdin_pipe[1]);
        close(stdout_pipe[0]);

        int status;
        waitpid(pid, &status, 0);

        printf("Temu завершен.\n");
    }

    return 0;
}