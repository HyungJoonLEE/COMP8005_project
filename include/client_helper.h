#ifndef COMP8005_PROJECT_CLIENT_HELPER_H
#define COMP8005_PROJECT_CLIENT_HELPER_H

#include "client.h"
#include "common.h"

client *createClientOps();
void options_init_client(struct options *opts);
void parse_arguments_client(int argc, char *argv[], struct options *opts);
int options_process_client(struct options *opts);
void cleanup(const struct options *opts);
void* listen_server(void* arg);
#endif //COMP8005_PROJECT_CLIENT_HELPER_H
