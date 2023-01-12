#include "logging.h"
#include "operations.h"
#include "common.h"

static int pipe_client;
static int pipe_server;

static char client_pipe_path[CLIENT_PIPE_PATH_SIZE] = {0};

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
    
    request req;
    response resp;

    if (strcmp(argv[3], "create") && argc == 5) {
        req.op_code = 3;
        strcpy(req.u_client_pipe_path.client_pipe_path, client_pipe_path);
        strcpy(req.u_box_name.box_name, argv[4]);
    } else if (strcmp(argv[3], "remove") && argc == 5) {
        req.op_code = 5;
        strcpy(req.u_client_pipe_path.client_pipe_path, client_pipe_path);
        strcpy(req.u_box_name.box_name, argv[4]);
    } else if (strcmp(argv[3], "list") && argc == 4) {
        req.op_code = 7;
        strcpy(req.u_client_pipe_path.client_pipe_path, client_pipe_path);
    } else {
        printf("Invalid arguments\n");
        unlink(client_pipe_path);
        close(pipe_server);
        exit(EXIT_FAILURE);
    }

    // Sends the request to the server
    if (write(pipe_server, &req, sizeof(request)) != sizeof(request)) {
        fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Opens the session pipe for reading
    // This waits for the server to open it for writing
    if ((pipe_client = open(client_pipe_path, O_RDONLY)) == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Reads the response from the server
    if (read(pipe_client, &resp, sizeof(response)) != sizeof(response)) {
        fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    switch (resp.op_code) {
        case OP_CODE_CREAT_MAILBOX_ANS:
            if (resp.u_return_code.return_code == 0)
                fprintf(stdout, "OK\n");
            else if (resp.u_return_code.return_code == -1)
                fprintf(stdout, "ERROR %s\n",  resp.u_response_message.error_message);

            break;
        case OP_CODE_RM_MAILBOX_ANS:
            if (resp.u_return_code.return_code == 0)
                fprintf(stdout, "OK\n");
            else if (resp.u_return_code.return_code == -1)
                fprintf(stdout, "ERROR %s\n",  resp.u_response_message.error_message);

            break;
        case OP_CODE_LIST_MAILBOX_ANS:
            for (;;) {
                box_listing listing = resp.u_box_listing.box_listing;

                if (listing.last == 1  && strcmp(listing.box_name, "") == 0) {
                    fprintf(stdout, "NO BOXES FOUND\n");
                    break;
                }

                fprintf(stdout, "%s %zu %zu %zu\n", listing.box_name, 
                                listing.box_size, listing.n_publishers,
                                listing.n_subscribers);

                if (listing.last == 1)
                    break;

                else {  // Reads the next box's information
                    if (read(pipe_client, &resp, sizeof(response)) != sizeof(response)) {
                        fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                }
            }
            
            break;
        default:

    }

    close(pipe_client);
    close(pipe_server);
    unlink(client_pipe_path);

    return 0;
}
