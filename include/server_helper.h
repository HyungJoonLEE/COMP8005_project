#ifndef COMP8005_PROJECT_SERVER_HELPER_H
#define COMP8005_PROJECT_SERVER_HELPER_H


#include "server.h"
#include "server_helper.h"
#include "common.h"
#include "linkedList.h"
#include "error.h"
#include "conversion.h"

#define DEFAULT_DIR "/etc/shadow"


server *createServerOps();
void options_init_server(server* opts, LinkedList* user_list, char* file_directory);
void parse_arguments_server(int argc, char *argv[], char* file_directory, server* opts, LinkedList* user_list);
void options_process_server(server* opts);
int add_new_client(server* opts, int client_socket, struct sockaddr_in *new_client_address);
void remove_client(server* opts, int client_socket);
int get_max_socket_number(server* opts);
void cleanup(const server* opts);

#endif //COMP8005_PROJECT_SERVER_HELPER_H

