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

static char publisher_pipe_path[PIPE_STRING_LENGTH] = {0};
static char server_pipe_path[PIPE_STRING_LENGTH] = {0};
static char box_name[BOX_NAME_SIZE] = {0};

request reqRegistry;
request reqMessage;

// Unused
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

    strcpy(server_pipe_path, argv[1]);
    strcpy(publisher_pipe_path, argv[2]);
    strcpy(box_name, argv[3]);

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
        reqRegistry.op_code = 1;
        strcpy(reqRegistry.u_client_pipe_path.client_pipe_path,publisher_pipe_path);
        strcpy(reqRegistry.u_box_name.box_name,box_name);

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

    // Request to Write Messages
    { 
        reqMessage.op_code = 9;
        char *message = malloc(1024);
        for(;;) {
            fprintf(stdout,"Enter message: ");
            ssize_t ret = fscanf(stdin,"%1023[^\n]", message);
            fprintf(stderr, "[INFO]: received %zd B\n", ret);
            strcpy(reqMessage.u_publisher_message.message,message);
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
            if (write(pipe_server, &reqMessage, sizeof(request)) != sizeof(request)) {
                fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            close(pipe_publisher);
        }
    }

    return 0;
}
