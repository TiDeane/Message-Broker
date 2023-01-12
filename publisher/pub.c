#include "logging.h"
#include "common.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define PIPE_STRING_LENGTH (256)

static int pipe_publisher;
static int pipe_server;

static char *publisher_pipe_path;
static char *server_pipe_path;
static char *box_name;

request req;

int main(int argc, char **argv) {

    if (argc < 4) {
        printf("Needs at least 4 arguments\n");
        return 1;
    }

    server_pipe_path = argv[1];
    publisher_pipe_path = argv[2];
    box_name = argv[3];

    // Remove session pipe if it exists
    if (unlink(publisher_pipe_path) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", argv[2],
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Creates the new session's named pipe
    if (mkfifo(publisher_pipe_path, 0640) != 0) {
        fprintf(stderr, "[ERR]: mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Request Registry 
    {
        req.op_code = 1;
        strcpy(req.u_client_pipe_path.client_pipe_path,publisher_pipe_path);
        strcpy(req.u_box_name.box_name,box_name);

        // Opens the server pipe for writing to request permission for a new session
        // This waits for the server to open it for reading 
        if ((pipe_server = open(server_pipe_path, O_WRONLY)) == -1) {
            fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (write(pipe_server, &req, sizeof(request)) != sizeof(request)) {
            fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        close(pipe_server);
    }

    // Request to Write Messages
    { 
        req.op_code = 9;
        memset(req.u_box_name.box_name,'\0',BOX_NAME_SIZE);
        char *message = malloc(1024);

        for(;;) {
            fprintf(stdout,"Enter message: ");
            ssize_t ret = fscanf(stdin,"%1023[^\n]", message);
            fprintf(stderr, "[INFO]: received %zd B\n", ret);
            strcpy(req.u_publisher_message.message,message);
            if (ret == 0) {
                // ret == 0 indicates EOF
                unlink(publisher_pipe_path);
                fprintf(stderr, "[INFO]: pipe closed\n");
                return 0;
            } else if (ret == -1) {
                // ret == -1 indicates error
                unlink(publisher_pipe_path);
                fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            // Opens publisher pipe to write messages
            // This waits for the server to open it for reading
            if ((pipe_publisher = open(server_pipe_path, O_WRONLY)) == -1) {
                fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (write(pipe_server, &req, sizeof(request)) != sizeof(request)) {
                fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            close(pipe_publisher);
        }
    }

    return 0;
}
