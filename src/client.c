#include "client.h"
#include "client_helper.h"

int main(int argc, char *argv[]) {
    client* opts = NULL;
    ssize_t received_data;
    char buffer[256] = {0};
    char s_buffer[256] = {0};
    int u = 0;
    int index = 0;
    int k;
    pthread_t th;
    LinkedList * user_list = NULL;
    int epfd, event_cnt, stdin_fd;
    struct epoll_event event;
    struct epoll_event* ep_events;

    user_list = createLinkedList(); // TODO: FREE
    opts = createClientOps();
    options_init_client(opts);
    parse_arguments_client(argc, argv, opts);
    opts->server_socket = options_process_client(opts);
    if (opts->server_socket == -1) {
        printf("Connect() fail");
    }

    epfd = epoll_create(EPOLL_SIZE);
    if (epfd == -1) {
        perror("epoll create: ");
        exit(EXIT_FAILURE);
    }
    ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);


    event.events = EPOLLIN;
    event.data.fd = opts->client_socket;
    epoll_ctl(epfd, EPOLL_CTL_ADD, opts->client_socket, &event);
    event.data.fd = opts->server_socket;
    epoll_ctl(epfd, EPOLL_CTL_ADD, opts->server_socket, &event);

    event.data.fd = 0;
    epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &event);

    stdin_fd = fileno(stdin);
    set_nonblocking_mode(opts->client_socket);
    set_nonblocking_mode(stdin_fd);

    while (1) {
        if (opts->exit_flag == 1) break;
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if (event_cnt == -1) {
            perror("epoll_wait() error: ");
            exit(EXIT_FAILURE);
        }

        for (int e = 0; e < event_cnt; e++) {
            if (ep_events[e].data.fd == opts->server_socket) {
                received_data = read(opts->server_socket, s_buffer, sizeof(buffer));
                if (received_data > 0) {
                    if (strstr(s_buffer, COMMAND_SEND)) {
                        char *token = strtok(s_buffer, " ");
                        // loop through the string to extract all other tokens
                        token = strtok(NULL, "/");
                        printf("socket id = %s\n", token);
                        user_list->socket_id = atoi(token);

                        token = strtok(NULL, "/");
                        printf("number of thread = %s\n", token); //printing each token
                        user_list->num_thread = atoi(token);

                        token = strtok(NULL, "/");
                        printf("number of client = %s\n", token);
                        user_list->client_count = atoi(token);

                        token = strtok(NULL, "\n");
                        printf("number of user = %s\n", token);
                        user_list->currentElementCount = atoi(token);
                        continue;
                    }
                    if (strstr(s_buffer, COMMAND_USER)) {
                        ListNode user = {0,};
                        char *token = strtok(s_buffer, " ");
                        // loop through the string to extract all other tokens
                        token = strtok(NULL, " ");
                        strcpy(user.id, token);
                        printf("id = %s\n", token);

                        token = strtok(NULL, " ");
                        strcpy(user.salt_setting, token);
                        printf("salt_setting = %s\n", token);

                        token = strtok(NULL, "\n");
                        strcpy(user.original, token);
                        printf("original = %s\n", token); //printing each token

                        addLLElement(user_list, u, user);
                        u++;
                        continue;
                    }
                    if (strstr(s_buffer, COMMAND_START)) {
                        opts->found = 0;
                        memset(getLLElement(user_list, index)->password, 0, 10);
                        pthread_create(&th, NULL, &listen_server, opts);
#pragma omp parallel num_threads(user_list->num_thread)
                        {
                            for (int i = 0; i < PASS_LEN + 1; ++i) {
                                int ptr1[i], ptr2[i];
                                for (int j = 0; j < i; j++)
                                    ptr1[j] = ptr2[j] = 0;

#pragma omp for schedule(dynamic)
                                for (k = (PASS_ARR_LEN / user_list->client_count + 1) * (user_list->socket_id - 5);
                                     k <
                                     (PASS_ARR_LEN / user_list->client_count + 1) * (user_list->socket_id - 4); k++) {
                                    if (opts->found == 1) {
#pragma omp cancel for
                                        continue;
                                    }
                                    if (strlen(getLLElement(user_list, index)->password) > 0) {
                                        memset(buffer, 0, sizeof(char) * 256);
                                        sprintf(buffer, "found: %d %s", index,
                                                getLLElement(user_list, index)->password);
                                        write(opts->server_socket, buffer, sizeof(buffer));
                                        opts->found = 1;
#pragma omp cancel for
                                        continue;
                                    } else {
                                        ptr1[0] = k;
                                        ptr2[0] = k + 1;
                                        password_generator(ptr1, ptr2, i, user_list, index, opts);
                                    }
#pragma omp cancellation point for
                                }
                            }
                        }
                    }
                    else {
                        pthread_join(th, NULL);
                        if (strlen(s_buffer) != 0)
                            if (!strstr(s_buffer, COMMAND_FOUND))
                                printf("[ server ]: %s", s_buffer);
                    }
                }
                memset(s_buffer, 0, 256);
            }

            if (ep_events[e].data.fd == 0) {
                if (fgets(buffer, sizeof(buffer), stdin)) {
                    if (strstr(buffer, COMMAND_EXIT)) {
                        write(opts->server_socket, buffer, sizeof(buffer));
                        printf("Exit from the server");
//                        free_heap_memory(user_list);
                        deleteLinkedList(user_list);
                        close(opts->server_socket);
                        opts->exit_flag = 1;
                        break;
                    } else {
                        write(opts->server_socket, buffer, sizeof(buffer));
                        memset(buffer, 0, sizeof(char) * 256);
                    }
                }
            }
        }
    }
    free(ep_events);
    cleanup(opts);
    free(opts);
    return EXIT_SUCCESS;
}
