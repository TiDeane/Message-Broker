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

    // Opens the server pipe for writing to request permission for a new session
    // This waits for the server to open it for reading 
    if ((pipe_server = open(argv[1], O_WRONLY)) == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // The following code makes an array of strings with the elements
    // argv[1], argv[2] and argv[4] (excluding argv[4] if argc = 4)
    char **msg_elements = malloc (3 * sizeof(char*));
    if (msg_elements == NULL) {
        unlink(client_pipe_path);
        close(pipe_server);
        return 1;
    }

    msg_elements[0] = malloc(2);
    msg_elements[1] = malloc(256);
    if (msg_elements[0] == NULL || msg_elements[1] == NULL) {
        unlink(client_pipe_path);
        close(pipe_server);
        return 1;
    }

    if (strcmp(argv[3], "create") && argc == 5) {
        msg_elements[2] = malloc(32);

        strcpy(msg_elements[0], "3");
        strcpy(msg_elements[1], argv[2]);
        strcpy(msg_elements[2], argv[4]);

    } else if (strcmp(argv[3], "remove") && argc == 5) {
        msg_elements[2] = malloc(32);

        strcpy(msg_elements[0], "5");
        strcpy(msg_elements[1], argv[2]);
        strcpy(msg_elements[2], argv[4]);
        
    } else if (strcmp(argv[3], "list") && argc == 4) {
        char **temp = realloc(msg_elements, 2 * sizeof(char*));
        if (temp != NULL)
            msg_elements = temp;
        else {
            unlink(client_pipe_path);
            close(pipe_server);
            return 1;
        }
        
        strcpy(msg_elements[0], "7");
        strcpy(msg_elements[1], argv[2]);

    } else { // [Invalid arguments] : Should not happen
        unlink(client_pipe_path);
        close(pipe_server);
        return 1;
    }

    // Constructs the message that is going to be sent to the server
    char *message = construct_message(msg_elements);
    if (message == NULL)
        return 1;

    // Sends the request to the server
    send_msg(pipe_server, message);

    // Opens the session pipe for reading
    // This waits for the server to open it for writing
    if ((pipe_client = open(client_pipe_path, O_RDONLY)) == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // The server will read and deconstruct the message, then answer the
    // request through the client_pipe. TODO: read from client_pipe and
    // interpret the answer (creating, removing, or listing boxes in the process)

    // Pensar em método de listar as caixas (elas são files no TFS)

    /*while (true) {
        char buffer[MAX_ANSWER_SIZE];
        ssize_t ret = read(pipe_client, buffer, MAX_ANSWER_SIZE - 1);
        if (ret == 0) {
            // ret == 0 signals EOF
            break;
        } else if (ret == -1) {
            // ret == -1 signals error
            fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        buffer[ret] = 0;
    }
    // Deconstructs the answer sent by the server
    char **msg_elements = deconstruct_message(buffer);

    if (strcmp(msg_elements[0]), "4") {

    } else if (strcmp(msg_elements[0]), "6") {

    } else if (strcmp(msg_elements[0]), "8") {

    } else {
        close(pipe_client);
        close(pipe_server);
        unlink(client_pipe_path);
        exit(EXIT_FAILURE);
    }
    */

    close(pipe_client);
    close(pipe_server);
    unlink(client_pipe_path);

    print_usage();
    WARN("unimplemented"); // TODO: implement
    return -1;
}
