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

static int messages_received = 0;

request reqRegistry;
response respMessage;

static void sig_handler(int sig) {
    // UNSAFE: This handler uses non-async-signal-safe functions (printf(),
    // exit();)
    if (sig == SIGINT) {

        if (signal(SIGINT, sig_handler) == SIG_ERR) {
            exit(EXIT_FAILURE);
        }
        // Caught SIGINT
        fprintf(stdout, "%d\n", messages_received);
        close(pipe_subscriber);
        unlink(subscriber_pipe_path);
        
        exit(EXIT_SUCCESS);
    }
}


int main(int argc, char **argv) {
    
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
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
        reqRegistry.op_code = OP_CODE_REG_SUBSCRIBER;
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
        // Opens subscriber pipe to read messages sent by the server
        // This waits for the server to open it for writing
        if ((pipe_subscriber = open(subscriber_pipe_path, O_RDONLY)) == -1) {
            fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (read(pipe_subscriber, &respMessage, sizeof(response)) != sizeof(response)) {
            fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (respMessage.u_return_code.return_code == -1)
            // Request to create a new session was denied
            exit(EXIT_FAILURE);

        while (true) {
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

            fprintf(stdout, "%s\n", respMessage.u_response_message.message);
            messages_received++;
        }
    }
    close(pipe_subscriber);
    unlink(subscriber_pipe_path);

    return 0;
}
