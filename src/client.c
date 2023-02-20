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
    fd_set read_fds;
    LinkedList * user_list = NULL;

    user_list = createLinkedList(); // TODO: FREE
    opts = createClientOps();
    options_init_client(opts);
    parse_arguments_client(argc, argv, opts);
    opts->server_socket = options_process_client(opts);
    if (opts->server_socket == -1) {
        printf("Connect() fail");
    }

    pthread_create(&th, NULL, &listen_server, &opts);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(0, &read_fds);
        FD_SET(fileno(stdin), &read_fds);
        FD_SET(opts->server_socket, &read_fds);
        struct timeval timeout;
        // receive time out config
        // Set 1 ms timeout counter
        // TODO: Sender can change the timeout to resend the packet
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        if (select(opts->server_socket + 1, &read_fds, NULL, NULL, NULL) < 0) {
            printf("select() error");
            exit(1);
        }

        if (FD_ISSET(fileno(stdin), &read_fds)) {
            if (fgets(buffer, sizeof(buffer), stdin)) {
                if (strstr(buffer, COMMAND_EXIT)) {
                    write(opts->server_socket, buffer, sizeof(buffer));
                    printf("Exit from the server");
                    free_heap_memory(user_list);
                    deleteLinkedList(user_list);
                    close(opts->server_socket);
                    break;
                }
                else {
                    write(opts->server_socket, buffer, sizeof(buffer));
                    memset(buffer, 0, sizeof(char) * 256);
                }
            }
        }

        if (FD_ISSET(opts->server_socket, &read_fds)) {
            received_data = read(opts->server_socket, s_buffer, sizeof(buffer));
            if (received_data > 0) {
                if (strstr(s_buffer, COMMAND_SEND)) {
                    char * token = strtok(s_buffer, " ");
                    // loop through the string to extract all other tokens
                    token = strtok(NULL, "/");
                    printf("socket id = %s\n", token);
                    user_list->socket_id = atoi(token);

                    token = strtok(NULL, "/");
                    printf( "number of thread = %s\n", token); //printing each token
                    user_list->num_thread = atoi(token);

                    token = strtok(NULL, "/");
                    printf( "number of client = %s\n", token);
                    user_list->client_count = atoi(token);

                    token = strtok(NULL, "/");
                    printf( "number of user = %s\n", token);
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
#pragma omp parallel num_threads(user_list->num_thread)
                    {
                        for (int i = 0; i < PASS_LEN + 1; ++i) {
                            int ptr1[i], ptr2[i];
                            for (int j = 0; j < i; j++)
                                ptr1[j] = ptr2[j] = 0;

                            #pragma omp for schedule(dynamic)
                            for (k = (PASS_ARR_LEN / user_list->client_count + 1) * (user_list->socket_id - 5);
                                    k < (PASS_ARR_LEN / user_list->client_count + 1) * (user_list->socket_id - 4); k++) {
                                if (strlen(getLLElement(user_list, index)->password) > 0 || opts->found == true) {
                                    memset(buffer, 0, sizeof(char) * 256);
                                    sprintf(buffer, "found: %d %s", index, getLLElement(user_list, index)->password);
                                    write(opts->server_socket, buffer, sizeof(buffer));
                                    #pragma omp cancel for
                                    continue;
                                }
                                else {
                                    ptr1[0] = k;
                                    ptr2[0] = k + 1;
                                    password_generator(ptr1, ptr2, i, user_list, index);
                                }
                                #pragma omp cancellation point for
                            }
                            opts->found = FALSE;
                        }
                    }
                    // TODO: password_generator_func call
                }
                else {
                    if (strlen(s_buffer) != 0)
                        if (!strstr(s_buffer, COMMAND_FOUND))
                            printf("[ server ]: %s", s_buffer);
                }
                memset(s_buffer, 0, 256);
            }
        }
    }

    cleanup(opts);
    free(opts);
    return EXIT_SUCCESS;
}
