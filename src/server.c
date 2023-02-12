#include "server.h"
#include "server_helper.h"
#include "common.h"

const char *CONNECTION_SUCCESS =
        "Successfully connected to the server\n"; // when client connected server
// send this

int main(int argc, char *argv[]) {
    server *opts = NULL;
    LinkedList * user_list = NULL;
    char file_directory[30] = {0};
    char file_list[BUF_SIZE] = {0};
    struct sockaddr_in client_address;
    int client_socket;
    int max_socket_num; // IMPORTANT Don't forget to set +1
    char buffer[256] = {0};
    char response[256] = {0};
    int client_address_size = sizeof(struct sockaddr_in);
    ssize_t received_data;
    fd_set read_fds; // fd_set chasing reading status


    putenv("OMP_CANCELLATION=true");
    opts = createServerOps();       // TODO: FREE
    user_list = createLinkedList(); // TODO: FREE
    options_init_server(opts, user_list, file_directory);
    parse_arguments_server(argc, argv, file_directory, opts, user_list);
    options_process_server(opts);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(opts->server_socket, &read_fds);
        for (int i = 0; i < opts->client_count; i++) {
            FD_SET(opts->client_socket[i], &read_fds);
        }
        max_socket_num = get_max_socket_number(opts) + 1;
        printf("wait for client\n");
        if (select(max_socket_num, &read_fds, NULL, NULL, NULL) < 0) {
            printf("select() error");
            exit(1);
        }

        if (FD_ISSET(opts->server_socket, &read_fds)) {
            client_socket = accept(opts->server_socket, (struct sockaddr *) &client_address, &client_address_size);
            if (client_socket == -1) {
                printf("accept() error");
                exit(1);
            }

            add_new_client(opts, client_socket, &client_address);
            write(client_socket, CONNECTION_SUCCESS, strlen(CONNECTION_SUCCESS));
            printf("Successfully added client_fd to client_socket[%d]\n", opts->client_count - 1);
        }

        if (fgets(buffer, sizeof(buffer), stdin)) {
            if (strstr(buffer, COMMAND_SEND) != NULL) {
                for (int i = 0; i < opts->client_count; i++) {
                    memset(buffer, 0, sizeof(char) * 256);
                    sprintf(buffer, "%d/%d/%d", opts->client_socket[i], user_list->num_thread, opts->client_count);
                    write(opts->client_socket[i], buffer, sizeof(buffer));
                    printf("%s were sent to client_socket[%d]\n", buffer, opts->client_socket[i]);
                }
            }

            if (strstr(buffer, COMMAND_START) != NULL) {
                for (int i = 0; i < opts->client_count; i++) {
                    write(opts->client_socket[i], buffer, sizeof(buffer));
                }
                memset(buffer, 0, sizeof(char) * 256);
            }

            if (strstr(buffer, COMMAND_EXIT) != NULL) {
                memset(buffer, 0, sizeof(char) * 256);
            }
        }



        // RECEIVE DATA FROM CLIENT
        for (int i = 0; i < opts->client_count; i++) {
            if (FD_ISSET(opts->client_socket[i], &read_fds)) {
                received_data = read(opts->client_socket[i], buffer, sizeof(buffer));
                buffer[received_data] = '\0';
                if (strlen(buffer) != 0)
                    printf("[ client %d]: %s\n", opts->client_socket[i], buffer);
                if (received_data < 0) {
                    remove_client(&opts, opts->client_socket[i]);
                    break;
                }
                // when client type "exit"
                if (strstr(buffer, "exit") != NULL) {
                    remove_client(&opts, opts->client_socket[i]);
                    continue;
                }
                memset(buffer, 0, sizeof(char) * 256);
            }
        }
    }
    cleanup(&opts);
    return EXIT_SUCCESS;
}

