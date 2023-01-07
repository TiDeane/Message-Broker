#include "logging.h"
#include "operations.h"
#include "common.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define PIPE_STRING_LENGTH (256)

static int pipe_client;
static int pipe_server;

static char client_pipe_path[PIPE_STRING_LENGTH] = {0};

static void print_usage() {
    fprintf(stderr, "usage: \n"
                    "   manager <register_pipe_name> create <box_name>\n"
                    "   manager <register_pipe_name> remove <box_name>\n"
                    "   manager <register_pipe_name> list\n");
}

// Still unused
void send_msg(int pipe, char const *msg) {
    size_t len = strlen(msg);
    size_t written = 0;

    while (written < len) {
        ssize_t ret = write(pipe, msg + written, len - written);
        if (ret < 0) {
            fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        written += (size_t) ret;
    }
}

int main(int argc, char **argv) {
 
    if (argc < 4) {
        printf("Needs at least 4 arguments\n");
        return 1;
    }

    strcpy(client_pipe_path, argv[2]);

    // Remove session pipe if it exists
    if (unlink(client_pipe_path) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", argv[2],
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Creates the new session's named pipe
    if (mkfifo(client_pipe_path, 0640) != 0) {
        fprintf(stderr, "[ERR]: mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Opens the session pipe for reading
    if ((pipe_client = open(client_pipe_path, O_RDONLY)) == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Opens the server pipe for writing to request permission for a new session 
    if ((pipe_server = open(argv[1], O_WRONLY)) == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Sends message coded 3, 5 or 7 (depending on argv[3]) to the server pipe.
    
    /*
    if (strcmp(argv[3], "create")) {
        send_request(OP_CODE_CREAT_MAILBOX, argv[4]);
    } else if (strcmp(argv[3], "remove")) {
        send_request(OP_CODE_RM_MAILBOX, argv[4]);
    } else if (strcmp(argv[3], "list")) {
        send_request(OP_CODE_LIST_MAILBOX, "");
    }
    */

    print_usage();
    WARN("unimplemented"); // TODO: implement
    return -1;
}
