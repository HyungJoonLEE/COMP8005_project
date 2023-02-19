#include "server.h"
#include "server_helper.h"
#include "common.h"

const char *CONNECTION_SUCCESS =
        "Successfully connected to the server\n"; // when client connected server send this

int main(int argc, char *argv[]) {
    server *opts = NULL;
    LinkedList * user_list = NULL;
    char file_directory[30] = {0};
    struct sockaddr_in client_address;
    int client_socket, stdin_fd;
    int index;
    char buffer[256] = {0};
    int client_address_size = sizeof(struct sockaddr_in);
    ssize_t received_data;
    int epfd, event_cnt;
    struct epoll_event event;
    struct epoll_event* ep_events;


    putenv("OMP_CANCELLATION=true");
    opts = createServerOps();       // TODO: FREE
    user_list = createLinkedList(); // TODO: FREE
    options_init_server(opts, user_list, file_directory);
    parse_arguments_server(argc, argv, file_directory, opts, user_list);
    options_process_server(opts);
    printf("Server Initialized Successfully\n"
                  "Waiting for connection ...\n\n\n");


    epfd = epoll_create(EPOLL_SIZE);
    if (epfd == -1) {
        perror("epoll create: ");
        exit(EXIT_FAILURE);
    }
    ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);


    event.events = EPOLLIN;
    event.data.fd = opts->server_socket;
    epoll_ctl(epfd, EPOLL_CTL_ADD, opts->server_socket, &event);

    event.data.fd = 0;
    epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &event);

    stdin_fd = fileno(stdin);
    set_nonblocking_mode(opts->server_socket);
    set_nonblocking_mode(stdin_fd);

    while (1) {
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if (event_cnt == -1) {
            perror("epoll_wait() error: ");
            exit(EXIT_FAILURE);
        }

        for (int e = 0; e < event_cnt; e++) {
            if (ep_events[e].data.fd == opts->server_socket) {
                client_socket = accept(opts->server_socket, (struct sockaddr *) &client_address, &client_address_size);
                if (client_socket == -1) {
                    printf("accept() error");
                    exit(1);
                }

                index = add_new_client(opts, client_socket, &client_address);
                write(client_socket, CONNECTION_SUCCESS, strlen(CONNECTION_SUCCESS));
                printf("Successfully added client socket(%d) to client_socket[%d]\n", client_socket, index);

                event.events = EPOLLIN;
                event.data.fd = client_socket;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket, &event);

            }
            if (ep_events[e].data.fd == 0) {
                if (fgets(buffer, sizeof(buffer), stdin)) {
                    if (strstr(buffer, COMMAND_SEND) != NULL) {
                        for (int i = 0; i < opts->client_count; i++) {
                            sprintf(buffer, "send %d/%d/%d\n", opts->client_socket[i], user_list->num_thread,
                                    opts->client_count);
                            write(opts->client_socket[i], buffer, sizeof(buffer));
                            printf("[Socket id][thread #][client count] sent to client_socket[%d] successfully\n",
                                   opts->client_socket[i]);
                            memset(buffer, 0, sizeof(char) * 256);
                        }
                    }
                    if (strstr(buffer, COMMAND_START) != NULL) {
                        for (int i = 0; i < opts->client_count; i++) {
                            write(opts->client_socket[i], buffer, sizeof(buffer));
                        }
                        memset(buffer, 0, sizeof(char) * 256);
                    }
                    if (strstr(buffer, COMMAND_EXIT) != NULL) {
                        for (int i = 0; i < opts->client_count; i++) {
                            remove_client(opts, opts->client_socket[i]);
                        }
                        close(opts->server_socket);
                        free(ep_events);
                        close(epfd);
                        cleanup(opts);
                        free(opts);
                        free_heap_memory(user_list);
                        deleteLinkedList(user_list);
                        return EXIT_SUCCESS;
                    } else {
                        if (strlen(buffer) != 0) {
                            buffer[strlen(buffer)] = 0;
                            for (int i = 0; i < opts->client_count; i++) {
                                write(opts->client_socket[i], buffer, sizeof(buffer));
                            }
                        }
                    }
                }
            }
            else {
                // RECEIVE DATA FROM CLIENT
                received_data = read(ep_events[e].data.fd, buffer, 256);
                if (ep_events[e].data.fd == 3) {
                    continue;
                }
                else {
                    if (strlen(buffer) != 0)
                        printf("[ client %d ]: %s", ep_events[e].data.fd, buffer);
                    if (received_data < 0) {
                        break;
                    }
                    // when client type "exit"
                    if (strstr(buffer, COMMAND_EXIT) != NULL) {
                        remove_client(opts, ep_events[e].data.fd);
                        continue;
                    }
                    memset(buffer, 0, sizeof(char) * 256);
                }
            }
        }
    }
}


