#ifndef COMP8005_PROJECT_SERVER_H
#define COMP8005_PROJECT_SERVER_H

#include "common.h"
#include "conversion.h"
#include "error.h"
#include "linkedList.h"
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define BACKLOG 25

struct options {
  char *client_ip;
  in_port_t server_port;
  int server_socket;
  int client_socket[25];
  int client_count;
  LinkedList *user_list;
};

/**
 * Initialize default option for client.
 * Memory setting for server option struct to 0 and DEFAULT port number saved.
 *
 * @param opts client struct settings
 */
static void options_init_server(struct options *opts);

/**
 * Parse input from server IP, server port, server download directory are
 * included. Set the server option struct after parsing. If there is no input,
 * it will use DEFAULT value.
 *
 * @param argc number of argument
 * @param argv server's input
 * @param opts server option struct settings
 */
static void parse_arguments_server(int argc, char *argv[],
                                   struct options *opts);

/**
 * Initialize network settings in server from protocol, ip, port and wait for
 * client connection.
 *
 * @param opts client option struct settings
 */
static void options_process_server(struct options *opts);

/**
 * Free variables that were used for client option struct setting.
 *
 * @param opts
 */
static void cleanup(const struct options *opts);

static int options_process(struct options *opts);

void add_new_client(struct options *opts, int client_socket,
                    struct sockaddr_in *newcliaddr);
int get_max_socket_number(struct options *opts);
void remove_client(struct options *opts, int client_socket);
bool data_receive_rate_process(struct options *opts);
bool ack_receive_rate_process(struct options *opts);
static void select_process_proxy(struct options *opts);

#endif // COMP8005_PROJECT_SERVER_H
