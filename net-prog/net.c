#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netdb.h>

#include "net.h"

#define PORT 8080

enum error {
    SOCKET_ERR  = -1,
    SETOPT_ERR  = -2,
    BIND_ERR    = -3,
    LISTEN_ERR  = -4,
    CONNECT_ERR = -5,
    PARSE_ERR = -6
};
typedef enum error error_t;

static int8_t _parse_address(char *address, char *ipv4, char *port);

int listen_net() {
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        return SOCKET_ERR;
    }

    int opt = 1;
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server);
        return SETOPT_ERR;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(server, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(server);
        return BIND_ERR;
    }

    if (listen(server, SOMAXCONN) < 0) {
        close(server);
        return LISTEN_ERR;
    }
    
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    
    struct hostent *host = gethostbyname(hostname);
    if (host != NULL) {
        printf("Local IP addresses:\n");
        for (int i = 0; host->h_addr_list[i] != NULL; i++) {
            struct in_addr ip_addr;
            memcpy(&ip_addr, host->h_addr_list[i], sizeof(struct in_addr));
            printf("[INFO] %s\n", inet_ntoa(ip_addr));
        }
    }
    
    return server;
}

int connect_net(char *address) {
    int conn = socket(AF_INET, SOCK_STREAM, 0);
	if (conn < 0) {
		return SOCKET_ERR;
	}

	char ipv4[16];
	char port[6];
	if (_parse_address(address, ipv4, port) != 0) {
		return PARSE_ERR;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));
	addr.sin_addr.s_addr = inet_addr(ipv4);
	if (connect(conn, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
		return CONNECT_ERR;
	}
	
    return conn;
}

int accept_net(int listener) {
    return accept(listener, NULL, NULL);
}

int close_net(int conn) {
    close(conn);
}

int send_net(int conn, char *buffer, size_t size) {
    return send(conn, buffer, size, 0);
}

int recv_net(int conn, char *buffer, size_t size) {
    return recv(conn, buffer, size, 0);
}

static int8_t _parse_address(char *address, char *ipv4, char *port) {
	size_t i = 0, j = 0;
	for (; address[i] != ':'; i++) {
		if (address[i] == '\0') {
			return 1;
		}
		if (i >= 15) {
			return 2;
		}
		ipv4[i] = address[i];
	}
	ipv4[i] = '\0';
	for (i += 1; address[i] != '\0'; i++, j++) {
		if (j >= 5) {
			return 3;
		}
		port[j] = address[i];
	}
	port[j] = '\0';
	return 0;
}