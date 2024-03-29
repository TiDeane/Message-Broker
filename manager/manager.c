#include "logging.h"
#include "operations.h"
#include "common.h"

static int pipe_client;
static int pipe_server;

static char *client_pipe_path;
static char *server_pipe_path;

request req;
response resp;
box_listing listing;

static void sig_handler(int sig) {

  // UNSAFE: This handler uses non-async-signal-safe functions (printf(),
  // exit();)
  if (sig == SIGINT) {
    // In some systems, after the handler call the signal gets reverted
    // to SIG_DFL (the default action associated with the signal).
    // So we set the signal handler back to our function after each trap.
    //
    unlink(client_pipe_path);
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        exit(EXIT_FAILURE);
    }
    return; // Resume execution at point of interruption
  }

  // Must be SIGQUIT - print a message and terminate the process
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {

    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        exit(EXIT_FAILURE);
    }
     if (signal(SIGQUIT, sig_handler) == SIG_ERR) {
        exit(EXIT_FAILURE);
    }

    if (argc < 4) {
        printf("Needs at least 4 arguments\n");
        return 1;
    }

    server_pipe_path = argv[1];
    client_pipe_path = argv[2];

    // Remove session pipe if it exists
    if (unlink(client_pipe_path) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: manager unlink(%s) failed: %s\n", argv[2],
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Creates the new session's named pipe
    if (mkfifo(client_pipe_path, 0640) != 0) {
        fprintf(stderr, "[ERR]: manager mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    

    if (!strcmp(argv[3], "create") && argc == 5) {
        req.op_code = 3;
        strcpy(req.u_client_pipe_path.client_pipe_path, client_pipe_path);
        strcpy(req.u_box_name.box_name, argv[4]);
    } else if (!strcmp(argv[3], "remove") && argc == 5) {
        req.op_code = 5;
        strcpy(req.u_client_pipe_path.client_pipe_path, client_pipe_path);
        strcpy(req.u_box_name.box_name, argv[4]);
    } else if (!strcmp(argv[3], "list") && argc == 4) {
        req.op_code = 7;
        strcpy(req.u_client_pipe_path.client_pipe_path, client_pipe_path);
    } else {
        printf("Invalid arguments\n");
        unlink(client_pipe_path);
        close(pipe_server);
        exit(EXIT_FAILURE);
    }
    
    // Opens the server pipe for writing to request permission for a new session
    // This waits for the server to open it for reading 
    if ((pipe_server = open(server_pipe_path, O_WRONLY)) == -1) {
        fprintf(stderr, "[ERR]: server open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // Sends the request to the server
    if (write(pipe_server, &req, sizeof(request)) != sizeof(request)) {
        fprintf(stderr, "[ERR]: server write failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    close(pipe_server);

    // Opens the session pipe for reading
    // This waits for the server to open it for writing
    if ((pipe_client = open(client_pipe_path, O_RDONLY)) == -1) {
        fprintf(stderr, "[ERR]: manager open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Reads the response from the server
    if (read(pipe_client, &resp, sizeof(response)) != sizeof(response)) {
        fprintf(stderr, "[ERR]: manager read failed: %s\n", strerror(errno));
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
            listing = resp.u_box_listing.box_listing;

            if (listing.last == 1  && strcmp(listing.box_name, "") == 0) {
                fprintf(stdout, "NO BOXES FOUND\n");
                break;
            }
            else {
                while (true) {
                    listing = resp.u_box_listing.box_listing;

                    fprintf(stdout, "%s %zu %zu %zu\n", listing.box_name, 
                                    listing.box_size, listing.n_publishers,
                                    listing.n_subscribers);

                    if (listing.last == 1)
                        break;
                    else {  // Reads the next box's information
                        if (read(pipe_client, &resp, sizeof(response)) != sizeof(response)) {
                            fprintf(stderr, "[ERR]: manager read failed: %s\n", strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                    }
                }
            }
            break;
        default:
            break;
    }

    close(pipe_client);
    unlink(client_pipe_path);
}
