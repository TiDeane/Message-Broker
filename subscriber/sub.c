#include "logging.h"
#include "common.h"

#include <signal.h>

#define PIPE_STRING_LENGTH (256)

static int pipe_subscriber;
static int pipe_server;

static char subscriber_pipe_path[PIPE_STRING_LENGTH] = {0};
static char server_pipe_path[PIPE_STRING_LENGTH] = {0};

char message[MESSAGE_SIZE] = {0};


static void sig_handler(int sig) {
  static int count = 0;

  // UNSAFE: This handler uses non-async-signal-safe functions (printf(),
  // exit();)
  if (sig == SIGINT) {
    // In some systems, after the handler call the signal gets reverted
    // to SIG_DFL (the default action associated with the signal).
    // So we set the signal handler back to our function after each trap.
    //
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

    strcpy(server_pipe_path, argv[1]);
    strcpy(subscriber_pipe_path, argv[2]);

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

    // Registry
    {
        char **reg_elements = malloc (3 * sizeof(char*));
        if (reg_elements == NULL) {
            unlink(subscriber_pipe_path);
            close(pipe_server);
            return 1;
        }

        reg_elements[0] = malloc(2);
        reg_elements[1] = malloc(256);
        reg_elements[2] = malloc(32);

        if (reg_elements[0] == NULL || reg_elements[1] == NULL ||
            reg_elements[2] == NULL) {
            unlink(subscriber_pipe_path);
            close(pipe_server);
            return 1;
        }

        strcpy(reg_elements[0], "2");
        strcpy(reg_elements[1], argv[2]);
        strcpy(reg_elements[2], argv[3]);

        // Constructs the message that is going to be sent to the server
        char *registry = construct_message(reg_elements);
        if (message == NULL)
        return 1;

        // Opens the server pipe for writing to request permission for a new session
        // This waits for the server to open it for reading 
        if ((pipe_server = open(server_pipe_path, O_WRONLY)) == -1) {
            fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        send_msg(pipe_server, registry);
        close(pipe_server);
    }

    // Read Messages
    { 
        char **msg_elements = malloc (2 * sizeof(char*));
        if (msg_elements == NULL) {
            unlink(subscriber_pipe_path);
            close(pipe_server);
            return 1;
        }
        msg_elements[0] = malloc(2);
        msg_elements[1] = malloc(1024);

        for (;;) {
            // Opens subscriber pipe to read messages sent by the server
            // This waits for the server to open it for writing
            if ((pipe_subscriber = open(subscriber_pipe_path, O_RDONLY)) == -1) {
                fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            // char buffer[MAX_CODE_10_SIZE];
            ssize_t ret = read(pipe_subscriber, message, MESSAGE_SIZE - 1);
            if (ret == 0) {
                // ret == 0 indicates EOF
                unlink(subscriber_pipe_path);
                fprintf(stderr, "[INFO]: pipe closed\n");
                return 0;
            } else if (ret == -1) {
                // ret == -1 indicates error
                unlink(subscriber_pipe_path);
                fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            // Deconstructs the answer sent by the server
            // char **msg_elements = deconstruct_message(buffer);

            fprintf(stderr, "[INFO]: received %zd B\n", ret);
            message[ret] = 0;
            fprintf(stdout, "%s\n", message);
            close(pipe_subscriber);
        }
    }

    return 0;
}
