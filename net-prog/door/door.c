#include <stdio.h>
#include <string.h>

#include "../net.h"

int main(int argc, char **argv) {
    char buf[BUFSIZ];

    if (atoi(argv[1]) == 1) {
        int serv_fd = listen_net();
        if (serv_fd < 0) {
            printf("Failed to start server (error code: %d)\n", serv_fd);
            return 1;
        }

        while(1) {
            int client = accept_net(serv_fd);
            if (client < 0) {
                perror("accept failed");
                continue;
            }
            
            int bytes_received = recv_net(client, buf, BUFSIZ - 1);
            if (bytes_received > 0) {
                buf[bytes_received] = '\0';
                printf("Received %d bytes: %s\n", bytes_received, buf);
                send_net(client, "1", 3);
            } else if (bytes_received == 0) {
                printf("Client disconnected\n");
            } else {
                perror("recv failed");
            }

            close_net(client);
        }
    } else if (atoi(argv[1]) == 2 && argc > 2) {
        int serv_fd = connect_net(argv[2]);
        if (serv_fd < 0) {
            printf("Failed to start server (error code: %d)\n", serv_fd);
            return 1;
        }

        while (1) {
            printf("-> ");
            if (fgets(buf, BUFSIZ, stdin) == NULL) {
                break;
            }
            
            send_net(serv_fd, buf, BUFSIZ);
            
            if (strncmp(buf, "exit", 4) == 0) {
                printf("Closing connection...\n");
                break;
            }
            
            memset(buf, 0, BUFSIZ);
            int bytes_received = recv_net(serv_fd, buf, BUFSIZ);
            
            if (bytes_received <= 0) {
                printf("Server disconnected\n");
                continue;
            }
            
            printf("Server: %s", buf);
        }
    } else {
        printf("АДРЕС УКАЗАН НЕВЕРНО!!!\n");
    }

    return 0;
}