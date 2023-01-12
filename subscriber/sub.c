#include "logging.h"
#include "common.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#define PIPE_STRING_LENGTH (256)

static int pipe_subscriber;
static int pipe_server;

static char *subscriber_pipe_path;
static char *server_pipe_path;
static char *box_name;

request reqRegistry;
response respMessage;

static void sig_handler(int sig) {
  static int count = 0;

  // UNSAFE: This handler uses non-async-signal-safe functions (printf(),
  // exit();)
  if (sig == SIGINT) {
    // In some systems, after the handler call the signal gets reverted
    // to SIG_DFL (the default action associated with the signal).
    // So we set the signal handler back to our function after each trap.
    //
    unlink(subscriber_pipe_path);
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
      exit(EXIT_FAILURE);
    }
    count++;
    fprintf(stderr, "Caught SIGINT (%d)\n", count);
    return; // Resume execution at point of interruption
  }

  // Must be SIGQUIT - print a message and terminate the process
  fprintf(stderr, "Caught SIGQUIT - that's all folks!\n");
  exit(EXIT_SUCCESS);
}


int main(int argc, char **argv) {
    
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        exit(EXIT_FAILURE);
    }
     if (signal(SIGQUIT, sig_handler) == SIG_ERR) {
        exit(EXIT_FAILURE);
    }

    if (argc < 4) {
        printf("Needs at least 4 arguments\n");
        return 1;
    }

    server_pipe_path = argv[1];
    subscriber_pipe_path = argv[2];
    box_name = argv[3];

    // Remove session pipe if it exists
    if (unlink(subscriber_pipe_path) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", argv[2],
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Creates the new session's named pipe
    if (mkfifo(subscriber_pipe_path, 0640) != 0) {
        fprintf(stderr, "[ERR]: mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Request Registry
    {
        reqRegistry.op_code = 2;
        strcpy(reqRegistry.u_client_pipe_path.client_pipe_path, subscriber_pipe_path);
        strcpy(reqRegistry.u_box_name.box_name, box_name);

        // Opens the server pipe for writing to request permission for a new session
        // This waits for the server to open it for reading 
        if ((pipe_server = open(server_pipe_path, O_WRONLY)) == -1) {
            fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (write(pipe_server, &reqRegistry, sizeof(request)) != sizeof(request)) {
            fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        close(pipe_server);
    }

    // Read Response Messages
    { 

        for (;;) {
            // Opens subscriber pipe to read messages sent by the server
            // This waits for the server to open it for writing
            if ((pipe_subscriber = open(subscriber_pipe_path, O_RDONLY)) == -1) {
                fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            // char buffer[MAX_CODE_10_SIZE];
            if (read(pipe_subscriber, &respMessage, sizeof(response)) != sizeof(response)) {
                fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            ssize_t ret = sizeof(respMessage.u_response_message.message);

            if (ret == -1) {
                // ret == -1 indicates error
                fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            fprintf(stderr, "[INFO]: received %zd B\n", ret);
            fprintf(stdout, "%s\n", respMessage.u_response_message.message);
            close(pipe_subscriber);
        }
    }
    unlink(subscriber_pipe_path);

    return 0;
}
