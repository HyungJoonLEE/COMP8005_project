#include "client.h"
#include "client_helper.h"

int main(int argc, char *argv[]) {
    client* opts = NULL;
    int max_socket_num; // IMPORTANT Don't forget to set +1
    char buffer[256] = {0};
    char response[256] = {0};
    fd_set read_fds;
    int result;

    opts = createClientOps();
    options_init_client(opts);
    parse_arguments_client(argc, argv, opts);
    opts->server_socket = options_process_client(opts);
    if (opts->server_socket == -1) {
        printf("Connect() fail");
    }

    max_socket_num = opts->server_socket;

    while (1) {
        FD_SET(0, &read_fds);
        FD_SET(opts->server_socket, &read_fds);
        struct timeval timeout;
        // receive time out config
        // Set 1 ms timeout counter
        // TODO: Sender can change the timeout to resend the packet
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        if (strlen(buffer) == 0) {
            if (FD_ISSET(0, &read_fds)) {
                if (fgets(buffer, sizeof(buffer), stdin)) {
                    if (strstr(buffer, "exit") != NULL) {
                        write(opts->server_socket, buffer, sizeof(buffer));
                        printf("Exit from the server");
                        close(opts->server_socket);
                        break;
                    }

                    if (strstr(buffer, "start") != NULL) {
                        memset(buffer, 0, sizeof(char) * 256);
                        // TODO: password_generator_func call
                    }
                }
            }
        }
        result = select(max_socket_num + 1, &read_fds, NULL, NULL, &timeout);

        if (result < 0) {
            printf("select fail");
            exit(1);
        } else if (result == 0) {
            write(opts->server_socket, buffer, sizeof(buffer));
        } else {
            read(opts->server_socket, response, sizeof(response));
            printf("receiver = [ %s ]\n", response);
            memset(buffer, 0, sizeof(char) * 256);
            memset(response, 0, sizeof(char) * 256);
        }
    }

    cleanup(opts);
    free(opts);
    return EXIT_SUCCESS;
}
