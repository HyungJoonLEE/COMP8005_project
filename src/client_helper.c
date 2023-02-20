#include "client.h"
#include "client_helper.h"
#include "common.h"
#include "error.h"
#include "conversion.h"


client *createClientOps() {
    client *pReturn = NULL;

    pReturn = (client *) malloc(sizeof(client));

    if (pReturn != NULL) {
        memset(pReturn, 0, sizeof(client));
    } else {
        printf("ERROR: Memory allocation client option failed.\n");
        return NULL;
    }

    return pReturn;
}


void options_init_client(struct options *opts) {
    memset(opts, 0, sizeof(struct options));
    opts->server_port = DEFAULT_PORT_SERVER;
}

void parse_arguments_client(int argc, char *argv[], struct options *opts) {
    int c;

    while ((c = getopt(argc, argv, ":s:p:")) !=
           -1) // NOLINT(concurrency-mt-unsafe)
    {
        switch (c) {
            case 's': {
                opts->server_ip = optarg;
                break;
            }
            case 'p': {
                opts->server_port = parse_port(
                        optarg,
                        10); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
                break;
            }
            case ':': {
                fatal_message(
                        __FILE__, __func__, __LINE__, "\"Option requires an operand\"",
                        5); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
                break;
            }
            case '?': {
                fatal_message(
                        __FILE__, __func__, __LINE__, "Unknown",
                        6); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            }
            default: {
                assert("should not get here");
            };
        }
    }
}

int options_process_client(struct options *opts) {
    ssize_t server_connection_test_fd;
    char message[50] = {0};

    if (opts->server_ip) {
        struct sockaddr_in server_addr;

        opts->server_socket = socket(AF_INET, SOCK_STREAM, 0);

        if (opts->server_socket == -1) {
            fatal_errno(__FILE__, __func__, __LINE__, errno, 2);
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(opts->server_port);
        server_addr.sin_addr.s_addr = inet_addr(opts->server_ip);

        if (server_addr.sin_addr.s_addr == (in_addr_t) -1) {
            fatal_errno(__FILE__, __func__, __LINE__, errno, 2);
        }

        opts->client_socket = connect(opts->server_socket, (struct sockaddr *) &server_addr,
                         sizeof(struct sockaddr_in));
        if (opts->client_socket < 0) fatal_errno(__FILE__, __func__, __LINE__, errno, 2);
        else {
            server_connection_test_fd = read(opts->server_socket, message, sizeof(message));
            if (server_connection_test_fd < 0) {
                printf("You are not connected to server\n");
            }
            else printf("%s \n", message);
        }
    }
    return opts->server_socket;
}

void cleanup(const struct options *opts) {
    if (opts->server_ip) {
        close(opts->server_socket);
    }
}


void* listen_server(void* arg) {
    char found_response[40] = {0};
    client* th_opts = (client*)arg;
    while(1) {
        if (read(th_opts->server_socket, found_response, sizeof(found_response)) > 0) {
            if (strstr(found_response, COMMAND_FOUND)) {
                printf("%s\n", found_response);
                th_opts->found = 1;
                break;
            }
        }
    }
}


void set_nonblocking_mode(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}
