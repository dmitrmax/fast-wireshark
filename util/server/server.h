
#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <glib.h>

void chain_func (int s, pid_t p);
void listener_die (int sig);
char* hex_byte (guint n, guint8* buf);
void receive_string (int sock);

int spawn_listeners (const struct addrinfo* crit,
                     const char* host,
                     const char* service,
                     void (* listener_fn) (int));

#endif

