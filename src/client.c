#include "client.h"
#include "client_helper.h"

int main(int argc, char *argv[]) {
    client* opts = NULL;
    ssize_t received_data;
    char buffer[256] = {0};
    char s_buffer[256] = {0};
    fd_set read_fds;
    int result;

    opts = createClientOps();
    options_init_client(opts);
    parse_arguments_client(argc, argv, opts);
    opts->server_socket = options_process_client(opts);
    if (opts->server_socket == -1) {
        printf("Connect() fail");
    }

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
                if (strstr(buffer, COMMAND_EXIT) != NULL) {
                    write(opts->server_socket, buffer, sizeof(buffer));
                    printf("Exit from the server");
                    close(opts->server_socket);
                    break;
                }
                if (strstr(buffer, "start") != NULL) {
                    memset(buffer, 0, sizeof(char) * 256);
                    // TODO: password_generator_func call
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
                    token = strtok(NULL, "/");
                    printf( "number of thread = %s\n", token); //printing each token
                    token = strtok(NULL, "/");
                    printf( "number of user = %s\n", token);
                }
                else {
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
