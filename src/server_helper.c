#include "server_helper.h"



server *createServerOps() {
    server *pReturn = NULL;

    pReturn = (server *) malloc(sizeof(server));

    if (pReturn != NULL) {
        memset(pReturn, 0, sizeof(server));
    } else {
        printf("ERROR: Memory allocation server option failed.\n");
        return NULL;
    }

    return pReturn;
}


void options_init_server(server* opts, LinkedList* user_list, char* file_directory) {
    opts->server_port = DEFAULT_PORT_SERVER;
    user_list->num_thread = DEFAULT_SINGLE_THREAD;
    opts->client_count = 0;
    strcpy(file_directory, DEFAULT_DIR);
}


void parse_arguments_server(int argc, char *argv[], char* file_directory, server* opts, LinkedList* user_list) {
    int c;

    while ((c = getopt(argc, argv, ":f:p:t:")) != -1) { // NOLINT(concurrency-mt-unsafe)
        switch (c) {
            case 'f': {
                strcpy(file_directory, optarg);
                break;
            }
            case 'p': {
                opts->server_port = parse_port(
                        optarg,
                        10); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
                break;
            }
            case 't': {
//                if (atoi(optarg) > omp_get_max_threads()) {
//                    printf("Thread number is higher than your computer's max thread");
//                    exit(1);
//                }
                user_list->num_thread = atoi(optarg);
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

    if (optind < argc) {
        int i = 0;
        while (argv[optind + i] != NULL) {
            ListNode user = {0,};
            strcpy(user.id, argv[optind + i]);
            addLLElement(user_list, i, user);
            i++;
        }
    }

}


void options_process_server(server* opts) {
    struct sockaddr_in server_address;
    int option = TRUE;

    for (int i = 0; i < opts->client_count; i++) {
        opts->client_socket[i] = 0;
    }

    opts->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (opts->server_socket == -1) {
        printf("socket() ERROR\n");
        exit(1);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(opts->server_port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (server_address.sin_addr.s_addr == (in_addr_t) - 1) {
        fatal_errno(__FILE__, __func__, __LINE__, errno, 2);
    }

    option = 1;
    setsockopt(opts->server_socket, SOL_SOCKET, SO_REUSEADDR, &option,
               sizeof(option));

    if (bind(opts->server_socket, (struct sockaddr *) &server_address,
             sizeof(struct sockaddr_in)) == -1) {
        printf("bind() ERROR\n");
        exit(1);
    }

    if (listen(opts->server_socket, BACKLOG) == -1) {
        printf("listen() ERROR\n");
        exit(1);
    }
}

int add_new_client(server* opts, int client_socket,
                    struct sockaddr_in *new_client_address) {
    int index = 0;
    char buffer[20];

    inet_ntop(AF_INET, &new_client_address->sin_addr, buffer, sizeof(buffer));
    printf("New client: [ %s ]\n", buffer);
    opts->client_count++;
    for (int i = 0; i < opts->client_count; i++) {
        if (opts->client_socket[i] == 0) {
            opts->client_socket[i] = client_socket;
            index = i;
            break;
        }
    }

    printf("Current client count = %d\n", opts->client_count);
    return index;
}

void remove_client(server* opts, int client_socket) {
    for (int i = 0; i < opts->client_count; i++) {
        if (opts->client_socket[i] == client_socket) {
            close(opts->client_socket[i]);
            opts->client_socket[i] = 0;
            opts->client_count--;
        }
    }

    printf("Client exited. Current client count = %d\n", opts->client_count);
}

// Finding maximum socket number
int get_max_socket_number(server* opts) {
    // Minimum socket number start with server socket(opts->server_socket)
    int max = opts->server_socket;
    int i;

    for (i = 0; i < opts->client_count; i++)
        if (opts->client_socket[i] > max)
            max = opts->client_socket[i];
    return max;
}

void cleanup(const server* opts) { close(opts->server_socket); }


void set_nonblocking_mode(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}
