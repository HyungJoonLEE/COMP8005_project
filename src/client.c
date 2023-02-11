#include "client.h"

int main(int argc, char *argv[]) {
  int max_socket_num; // IMPORTANT Don't forget to set +1
  char buffer[256] = {0};
  char response[256] = {0};
  fd_set read_fds;
  int result;

  struct options opts;
  options_init(&opts);
  parse_arguments(argc, argv, &opts);
  opts.server_socket = options_process(&opts);
  if (opts.server_socket == -1) {
    printf("Connect() fail");
  }

  max_socket_num = opts.server_socket;

  while (1) {
    FD_SET(0, &read_fds);
    FD_SET(opts.server_socket, &read_fds);
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
            write(opts.server_socket, buffer, sizeof(buffer));
            printf("Exit from the server");
            close(opts.server_socket);
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
      write(opts.server_socket, buffer, sizeof(buffer));
    } else {
      read(opts.server_socket, response, sizeof(response));
      printf("receiver = [ %s ]\n", response);
      memset(buffer, 0, sizeof(char) * 256);
      memset(response, 0, sizeof(char) * 256);
    }
  }

  cleanup(&opts);
  return EXIT_SUCCESS;
}

static void options_init(struct options *opts) {
  memset(opts, 0, sizeof(struct options));
  opts->fd_in = STDIN_FILENO;
  opts->fd_out = STDOUT_FILENO;
  opts->server_port = DEFAULT_PORT_SERVER;
}

static void parse_arguments(int argc, char *argv[], struct options *opts) {
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

static int options_process(struct options *opts) {
  ssize_t server_connection_test_fd;
  int result;
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

    if (server_addr.sin_addr.s_addr == (in_addr_t)-1) {
      fatal_errno(__FILE__, __func__, __LINE__, errno, 2);
    }

    result = connect(opts->server_socket, (struct sockaddr *)&server_addr,
                     sizeof(struct sockaddr_in));

    if (result == -1) {
      fatal_errno(__FILE__, __func__, __LINE__, errno, 2);
    }
    server_connection_test_fd =
        read(opts->server_socket, message, sizeof(message));
    if (server_connection_test_fd == -1) {
      printf("You are not connected to server\n");
    }
    printf("%s \n", message);
  }
  return opts->server_socket;
}

static void cleanup(const struct options *opts) {
  if (opts->server_ip) {
    close(opts->fd_out);
  }
}
