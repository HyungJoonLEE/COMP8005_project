#ifndef COMP8005_PROJECT_CLIENT_H
#define COMP8005_PROJECT_CLIENT_H

#include "common.h"
#include "conversion.h"
#include "error.h"
#include <arpa/inet.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

struct options {
  char *server_ip;
  in_port_t server_port;
  int fd_in;
  int fd_out;
  int server_socket;
};

/**
 * Initialize default option for client
 * STDIN, STDOUT and port number saved
 *
 * @param opts client struct settings
 */
static void options_init(struct options *opts);

/**
 * Parse input from user server IP, server port, client text files are included.
 * Set the client option struct after parsing.
 * If there is no input, it will use DEFAULT value.
 *
 * @param argc number of argument
 * @param argv client's input
 * @param opts client option struct settings
 */
static void parse_arguments(int argc, char *argv[], struct options *opts);

/**
 * Initialize connection with server.
 * After successful connection, client will get message from server to confirm
 * it is connected.
 *
 * @param opts client option struct settings
 */
static int options_process(struct options *opts);

/**
 * Free variables that were used for client option struct setting.
 *
 * @param opts client option struct settings
 */
static void cleanup(const struct options *opts);

#endif // COMP8005_PROJECT_CLIENT_H
