#include "logging.h"

#define PIPE_STRING_LENGTH (256)

static int pipe_client;
static int pipe_server;
static int session_id;

static char client_named_pipe_path[PIPE_STRING_LENGTH] = {0};

static void print_usage() {
    fprintf(stderr, "usage: \n"
                    "   manager <register_pipe_name> create <box_name>\n"
                    "   manager <register_pipe_name> remove <box_name>\n"
                    "   manager <register_pipe_name> list\n");
}

int main(int argc, char **argv) {
 
    if (argc < 4) {
        printf("Needs at least 4 arguments\n");
        return 1;
    }

    // Remove session pipe if it does exist
    if (unlink(argv[2]) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", FIFO_PATHNAME,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Creates the new session's named pipe
    if (mkfifo(argv[2], 0640) != 0) {
        fprintf(stderr, "[ERR]: mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Opens the session pipe for reading
    if ((pipe_client = open(argv[2], O_RDONLY)) == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Opens the server pipe for writing to request permission for a new session 
    if ((pipe_server = open(argv[1], O_WRONLY)) == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // TODO: sends messages coded 3, 5 or 7 (depending on argv[3]) to the server pipe.
    // Make function that reads until it finds a "|"? (similar to the IAED airport project)

    print_usage();
    WARN("unimplemented"); // TODO: implement
    return -1;
}
