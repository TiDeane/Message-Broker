#include "logging.h"
#include "common.h"


#define PIPE_STRING_LENGTH (256)

static int pipe_subscriber;
static int pipe_server;

static int messages_received = 0;

static char *subscriber_pipe_path;
static char *server_pipe_path;

static char *box_name;

request req;
response resp;

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
        req.op_code = OP_CODE_REG_SUBSCRIBER;
        strcpy(req.u_client_pipe_path.client_pipe_path, subscriber_pipe_path);
        strcpy(req.u_box_name.box_name, box_name);

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

    // Opens subscriber pipe to read messages sent by the server
    // This waits for the server to open it for writing
    if ((pipe_subscriber = open(subscriber_pipe_path, O_RDONLY)) == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (read(pipe_subscriber, &resp, sizeof(response)) != sizeof(response)) {
        fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (resp.u_return_code.return_code == -1) {
        unlink(subscriber_pipe_path);
        // Request to create a new session was denied
        fprintf(stdout, "ERROR %s\n",  resp.u_response_message.error_message);
        exit(EXIT_FAILURE);
    }
    close(pipe_subscriber);

    // Read Response Messages
    {   
        resp.op_code = OP_CODE_SERVER_PUBLISHER_MESSAGE;
        // Opens subscriber pipe to read messages sent by the server
        // This waits for the server to open it for writing
        if ((pipe_subscriber = open(subscriber_pipe_path, O_RDONLY)) == -1) {
            unlink(subscriber_pipe_path);
            fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        while (true) {
            
            ssize_t ret = read(pipe_subscriber, &resp, sizeof(response));
            if (ret == 0 || ret == -1) {
                // ret == 0 indicates EOF
                fprintf(stdout, "%d\n", messages_received);
                break;
            }
            
            // tfs_read will read the file's entire data_block, sending all messages
            fprintf(stdout, "%s", resp.u_response_message.message);
            
            char *token = strtok(resp.u_response_message.message, "\n");
            while (token != NULL) {
                messages_received++;
                token = strtok(NULL, "\n");
            }
        }
        close(pipe_subscriber);
        unlink(subscriber_pipe_path);
    }
}
