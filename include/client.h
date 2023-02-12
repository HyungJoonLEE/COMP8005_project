#ifndef COMP8005_PROJECT_CLIENT_H
#define COMP8005_PROJECT_CLIENT_H

#include "common.h"

typedef struct options {
  char *server_ip;
  in_port_t server_port;
  int fd_in;
  int fd_out;
  int server_socket;
}client;


#endif // COMP8005_PROJECT_CLIENT_H
