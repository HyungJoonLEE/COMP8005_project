#include "server.h"
#include "server_helper.h"
#include "common.h"

const char *CONNECTION_SUCCESS =
        "Successfully connected to the server\n"; // when client connected server send this

int main(int argc, char *argv[]) {
    server *opts = NULL;
    LinkedList * user_list = NULL;
    char file_directory[30] = {0};
    char file_list[BUF_SIZE] = {0};
    struct sockaddr_in client_address;
    int client_socket, stdin_fd;
    int index;
    int user_no = 0;
    char buffer[256] = {0};
    int client_address_size = sizeof(struct sockaddr_in);
    ssize_t received_data;
    int epfd, event_cnt;
    struct epoll_event event;
    struct epoll_event* ep_events;
    struct timespec start, finish;
    double time;

    putenv("OMP_CANCELLATION=true");
    opts = createServerOps();       // TODO: FREE
    user_list = createLinkedList(); // TODO: FREE
    options_init_server(opts, user_list, file_directory);
    parse_arguments_server(argc, argv, file_directory, opts, user_list);
    read_from_shadow(file_directory, file_list);
    find_user(file_list, user_list);
    salt_setting(user_list);
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
        puts("\n\n\n==== SELECT OPTION TO RUN ====");
        printf("Current Computer Count = %d\n", opts->client_count);
        puts("1. Send Socket Info");
        puts("2. Send User Info");
        puts("3. Start Cracking");
        puts("4. Show Result");
        puts("5. Flush User Info");
        puts("6. EXIT Program");
        puts("==============================");
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
                    if (strstr(buffer, OPT_ONE)) {
                        for (int i = 0; i < opts->client_count; i++) {
                            sprintf(buffer, "send %d/%d/%d/%d\n", opts->client_socket[i], user_list->num_thread,
                                    opts->client_count, user_list->currentElementCount);
                            write(opts->client_socket[i], buffer, sizeof(buffer));
                            printf("[Socket id][thread][client count][user count] sent to client_socket[%d] successfully\n",
                                   opts->client_socket[i]);
                            memset(buffer, 0, sizeof(char) * 256);
                        }
                        continue;
                    }
                    if (strstr(buffer, OPT_TWO) != NULL) {
                        for (int i = 0; i < opts->client_count; i++) {
                            for (int u = 0; u < user_list->currentElementCount; u++) {
                                sprintf(buffer, "user %s %s %s\n", getLLElement(user_list, u)->id,
                                        getLLElement(user_list, u)->salt_setting,
                                        getLLElement(user_list, u)->original);
                                write(opts->client_socket[i], buffer, sizeof(buffer));
                                memset(buffer, 0, sizeof(char) * 256);
                            }
                            printf("[user id][salt][salt setting] sent to client_socket[%d] successfully\n",
                                   opts->client_socket[i]);
                        }
                        continue;
                    }
                    if (strstr(buffer, OPT_THREE)) {
                        if (strlen(getLLElement(user_list, user_no)->password) > 0) {
                            puts("WE ALREADY CRACKED THE PASSWORD\n"
                                 "TYPE 4 TO SEE THE CRACKED PASSWORD");
                            break;
                        }
                        clock_gettime(CLOCK_MONOTONIC, &start);
                        for (int i = 0; i < opts->client_count; i++) {
                            write(opts->client_socket[i], COMMAND_START, strlen(COMMAND_START));
                        }
                        memset(buffer, 0, sizeof(char) * 256);
                    }
                    if (strstr(buffer, OPT_FOUR)) {
                        displayLinkedList(user_list);
                        memset(buffer, 0, sizeof(char) * 256);
                        continue;
                    }
                    if (strstr(buffer, OPT_FIVE)) {
                        for (int i = 0; i < opts->client_count; i++) {
                            write(opts->client_socket[i], COMMAND_FLUSH, strlen(COMMAND_FLUSH));
                        }
                        memset(buffer, 0, sizeof(char) * 256);
                        continue;
                    }
                    if (strstr(buffer, OPT_SIX)) {
                        close(opts->server_socket);
                        free(ep_events);
                        close(epfd);
                        cleanup(opts);
                        free(opts);
                        deleteLinkedList(user_list);
                        return EXIT_SUCCESS;
                    }
                    else {
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
                if (ep_events[e].data.fd == opts->server_socket) {
                    continue;
                }
                else {
                    if (strlen(buffer) != 0)
                        if (strstr(buffer, COMMAND_FOUND)) {
                            opts->dup_count++;
                            if (opts->dup_count > 1) continue;
                            else {
                                clock_gettime(CLOCK_MONOTONIC, &finish);
                                time = (finish.tv_sec - start.tv_sec);
                                time += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
                                opts->found = 1;
                                getLLElement(user_list, user_no)->time = time;
                                char *token = strtok(buffer, " ");
                                token = strtok(NULL, " ");
                                user_no = atoi(token);
                                token = strtok(NULL, " ");
                                strcpy(getLLElement(user_list, user_no)->password, token);
                                token = strtok(NULL, " ");
                                getLLElement(user_list, user_no)->count = atoi(token);
                                memset(buffer, 0, sizeof(char) * 256);
                                printf("[FOUND] %s : %s\n", getLLElement(user_list, user_no)->id, getLLElement(user_list, user_no)->password);
                                for (int i = 0; i < opts->client_count; i++) {
                                    strcpy(buffer, COMMAND_FOUND);
                                    write(opts->client_socket[i], buffer, sizeof(buffer));
                                }
                                memset(buffer, 0, sizeof(char) * 256);
                                opts->found = 0;
                                opts->dup_count = 0;
                                user_no++;
                            }
                            if (user_no == user_list->currentElementCount) {
                                puts("Cracked Everything");
                                user_no = 0;
                                break;
                            }
                            else {
                                printf("Starting crack password : %s in 5 seconds\n",
                                       getLLElement(user_list, user_no)->id);
                                for (int s = 5; s > 0; s--) {
                                    sleep(1);
                                    printf("%d ... \n", s);
                                }
                                for (int i = 0; i < opts->client_count; i++) {
                                    strcpy(buffer, COMMAND_START);
                                    write(opts->client_socket[i], buffer, sizeof(buffer));
                                    clock_gettime(CLOCK_MONOTONIC, &start);
                                }
                            }
                        }
                        else
                            printf("[ client %d ]: %s", ep_events[e].data.fd, buffer);
                    if (received_data < 0) {
                        break;
                    }
                    // when client type "exit"
                    if (strstr(buffer, COMMAND_EXIT)) {
                        remove_client(opts, ep_events[e].data.fd);
                        continue;
                    }
                    memset(buffer, 0, sizeof(char) * 256);
                }
                opts->found = 0;
            }
        }
    }
}


