#include "logging.h"
#include "common.h"

static int pipe_client;
static int pipe_server;

static char *server_pipe_path;

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    server_pipe_path = argv[1];

    // Remove server pipe if it exists
    if (unlink(server_pipe_path) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: server unlink(%s) failed: %s\n", argv[2],
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Creates the server's named pipe
    if (mkfifo(server_pipe_path, 0640) != 0) {
        fprintf(stderr, "[ERR]: server mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Opens server pipe for reading
    if ((pipe_server = open(server_pipe_path, O_WRONLY)) == -1) {
        fprintf(stderr, "[ERR]: manager open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (true) {
        /* Opens and closes a dummy pipe to avoid having active wait */
        int tmp_pipe = open(server_pipe_path, O_RDONLY);
        if (tmp_pipe == -1) {
            if (errno == ENOENT) {
                /* If pipe doesn't exist, it means we've exited */
                return 0;
            }
            fprintf(stderr, "[ERR]: server open failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        close(tmp_pipe);

        

        break;
    }

    fprintf(stderr, "usage: mbroker <pipename>\n");
    WARN("unimplemented"); // TODO: implement
    return -1;
}
