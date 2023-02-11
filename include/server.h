#ifndef COMP8005_PROJECT_SERVER_H
#define COMP8005_PROJECT_SERVER_H

#include "common.h"

#define BACKLOG 25

typedef struct option{
  char *client_ip;
  in_port_t server_port;
  int server_socket;
  int client_socket[25];
  int client_count;
}server;


#endif // COMP8005_PROJECT_SERVER_H
