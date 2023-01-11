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

    // Registry
    {
        char **reg_elements = malloc (3 * sizeof(char*));
        if (reg_elements == NULL) {
            unlink(publisher_pipe_path);
            close(pipe_server);
            return 1;
        }

        reg_elements[0] = malloc(2);
        reg_elements[1] = malloc(256);
        reg_elements[2] = malloc(32);

        if (reg_elements[0] == NULL || reg_elements[1] == NULL ||
            reg_elements[2] == NULL) {
            unlink(publisher_pipe_path);
            close(pipe_server);
            return 1;
        }

        strcpy(reg_elements[0], "1");
        strcpy(reg_elements[1], argv[2]);
        strcpy(reg_elements[2], argv[3]);

        // Constructs the message that is going to be sent to the server
        char *registry = construct_message(reg_elements);
        if (registry == NULL)
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

    // Write Messages
    { 
        char **pub_elements = malloc (2 * sizeof(char*));
        if (pub_elements == NULL) {
            unlink(publisher_pipe_path);
            close(pipe_server);
            return 1;
        }
        pub_elements[0] = malloc(2);
        pub_elements[1] = malloc(1024);
        
        char *message = malloc (MESSAGE_SIZE * sizeof(char*));
        
        for(;;) {
            fprintf(stdout,"Enter message: ");
            ssize_t ret = fscanf(stdin,"%1023[^\n]", message);
            fprintf(stderr, "[INFO]: received %zd B\n", ret);

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

            strcpy(pub_elements[0], "9");
            strcpy(pub_elements[1], message);
            char *publication = construct_message(pub_elements);
            if (publication == NULL)
                return 1;

            // Opens publisher pipe to write messages
            // This waits for the server to open it for reading
            if ((pipe_publisher = open(publisher_pipe_path, O_WRONLY)) == -1) {
                fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            send_msg(pipe_publisher, publication);
            close(pipe_publisher);
        }
    }

    return 0;
}
