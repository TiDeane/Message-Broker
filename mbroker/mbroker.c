#include "logging.h"
#include "common.h"
#include "mbroker.h"
#include "operations.h"
#include "state.h"

static int pipe_server;

static char *server_pipe_path;

// Array of strings that keeps all mailbox paths
static char **mailbox_paths;

// When mailbox_number reaches mailbox_alloc, mailbox_alloc is increased by 10
// and memory is allocated for 10 more
static int mailbox_number = 0;
static long unsigned mailbox_alloc = 10;

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    mailbox_paths = malloc(mailbox_alloc * sizeof(char*));

    request req;
    response resp;

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

        if (read(pipe_server, &req, sizeof(request)) != sizeof(request)) {
            fprintf(stderr, "[ERR]: manager read failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        switch (req.op_code) {
            case OP_CODE_REG_PUBLISHER:

                break;
            case OP_CODE_REG_SUBSCRIBER:

                break;
            case OP_CODE_CREAT_MAILBOX:
                op_create_mailbox(req);

                break;
            case OP_CODE_RM_MAILBOX:

                break;
            case OP_CODE_LIST_MAILBOX:

                break;
            case OP_CODE_PUBLISHER_SERVER_MESSAGE:

                break;
            default:
                break;
        }
    }

    return 0;
}

void register_publisher(request req) {

}

void register_subscriber(request req) {

}

void op_create_mailbox(request req) {
    response resp;
    int pipe_client;

    if ((pipe_client = open(req.u_client_pipe_path.client_pipe_path, O_WRONLY)) == -1) {
        fprintf(stderr, "[ERR]: client open failed: %s\n", strerror(errno));
        return;
    }

    char box_name_path[33] = "/";
    strcat(box_name_path, req.u_box_name.box_name);

    inode_t *root = inode_get(ROOT_DIR_INUM);
    int check_box_exists = tfs_lookup(box_name_path, root);

    int fhandle;

    if (check_box_exists != -1) {
        resp.u_return_code.return_code = -1;
        strcpy(resp.u_response_message.error_message, "[ERR]: box already exists\n");
    }
    else if ((fhandle = tfs_open(box_name_path, TFS_O_CREAT)) == -1) {
        resp.u_return_code.return_code = -1;
        strcpy(resp.u_response_message.error_message, "[ERR]: could not create box\n");
    } else {
        // Now the box has been created, so we have to run tfs_lookup again
        int mailbox_inumber = tfs_lookup(box_name_path, root);
        inode_t* mailbox_inode = inode_get(mailbox_inumber);

        mailbox new_mailbox;
        new_mailbox.n_subscribers = 0;
        new_mailbox.n_publishers = 0;
        new_mailbox.n_messages = 0;
        mailbox_inode->mailbox = &new_mailbox;

        // If mailbox_alloc has been reached by mailbox_number, allocates memory
        // for 10 more
        if (mailbox_number == mailbox_alloc) {
            mailbox_alloc += 10;
            char **temp = realloc(mailbox_paths, mailbox_alloc * sizeof(char*));
            if (temp == NULL) {
                printf("No memory\n");
                return;
            }
            mailbox_paths = temp;
        }

        // Adds the new mailbox's path to the array
        mailbox_paths[mailbox_number] = malloc(BOX_NAME_SIZE + 1);
        strcpy(mailbox_paths[mailbox_number], box_name_path);
        mailbox_number++;

        tfs_close(fhandle);
    }
}

void op_remove_mailbox(request req) {

}

void op_list_mailbox(request req) {

}

void publish_message(request req) {

}
