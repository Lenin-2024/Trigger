#ifndef __NET_H__
#define __NET_H__

#include <stdint.h>
#include <stdlib.h>

int listen_net();
int connect_net(char *address);

int accept_net(int listener);

int close_net(int conn);

int send_net(int conn, char *buffer, size_t size);
int recv_net(int conn, char *buffer, size_t size);

#endif /* __NET_H__ */