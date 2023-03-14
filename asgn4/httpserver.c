// Asgn 2: A simple HTTP server.
// By: Eugene Chou
//     Andrew Quinn
//     Brian Zhao

#include "asgn2_helper_funcs.h"
#include "connection.h"
#include "response.h"
#include "request.h"
#include "debug.h"
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "queue.h"
#include <sys/stat.h>

void handle_connection(int);

void handle_get(conn_t *);
void handle_put(conn_t *);
void handle_unsupported(conn_t *);
void *worker_threads();

queue_t *task_queue = NULL;

int main(int argc, char **argv) {
    if (argc < 2) {
        warnx("wrong arguments: %s port_num", argv[0]);
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int threads;

    if (argc == 3){
        char *endptr = NULL;
        threads = (int) strtoull(argv[2], &endptr, 10);
        if (endptr && *endptr != '\0') {
            warnx("invalid thread number: %s", argv[2]);
            return EXIT_FAILURE;
        }

    }else if (argc == 2)
    {
        threads = 4;// Default number of threads is 4
        
    }
    

    char *endptr = NULL;
    size_t port = (size_t) strtoull(argv[1], &endptr, 10);
    if (endptr && *endptr != '\0') {
        warnx("invalid port number: %s", argv[1]);
        return EXIT_FAILURE;
    }

    signal(SIGPIPE, SIG_IGN);
    Listener_Socket sock;
    listener_init(&sock, port);

    int j;

    pthread_t thread_pool[threads]; // array of threads

    for(j = 0; j < threads; j++){
        pthread_create(&thread_pool[j], NULL, worker_threads, NULL);
    }

    // initialize the queue

    task_queue = queue_new(10);


    while (1) {
        int *connfd;
        connfd = listener_accept(&sock);
        queue_push(task_queue, connfd); // Push the task to queue
        //handle_connection(connfd);
        //close(connfd);

    }
    queue_delete(&task_queue);
    return EXIT_SUCCESS;
}

void *worker_threads(){
    while(true){
        uintptr_t conn;
        queue_pop(task_queue,(void **)&conn);
        handle_connection(conn);
        close(conn);
    }
}

void handle_connection(int connfd) {

    conn_t *conn = conn_new(connfd);

    const Response_t *res = conn_parse(conn);

    if (res != NULL) {
        conn_send_response(conn, res);
    } else {
        debug("%s", conn_str(conn));
        const Request_t *req = conn_get_request(conn);
        if (req == &REQUEST_GET) {
            handle_get(conn);
        } else if (req == &REQUEST_PUT) {
            handle_put(conn);
        } else {
            handle_unsupported(conn);
        }
    }

    conn_delete(&conn);
}

void handle_get(conn_t *conn) {

    char *uri = conn_get_uri(conn);
    char *Req_id = conn_get_header(conn, "Request-Id");

    debug("GET request not implemented. But, we want to get %s", uri);

    // What are the steps in here?

    // 1. Open the file.
    // If  open it returns < 0, then use the result appropriately
    //   a. Cannot access -- use RESPONSE_FORBIDDEN
    //   b. Cannot find the file -- use RESPONSE_NOT_FOUND
    //   c. other error? -- use RESPONSE_INTERNAL_SERVER_ERROR
    // (hint: check errno for these cases)!

    // 2. Get the size of the file.
    // (hint: checkout the function fstat)!

    // Get the size of the file.

    // 3. Check if the file is a directory, because directories *will*
    // open, but are not valid.
    // (hint: checkout the macro "S_IFDIR", which you can use after you call fstat!)


    // 4. Send the file
    // (hint: checkout the conn_send_file function!)

    const Response_t *res = NULL;
    debug("handling get requests for %s", uri);

    int fd = open(uri, O_RDONLY);
    if(fd < 0){
        if(access(uri, F_OK) != 0){
            res = &RESPONSE_NOT_FOUND;
            goto out;
        }
        if(errno == EACCES || errno == EISDIR){
            res = &RESPONSE_FORBIDDEN;
            goto out;
        }
        else{
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            goto out;
        }
    }

    struct stat st = {0};
    stat(uri, &st);

    uint64_t size = st.st_size;

    if(S_ISDIR(st.st_mode)!= 0){
        res = &RESPONSE_FORBIDDEN;
        goto out;
    }

    res = conn_send_file(conn, fd, size);

    res = &RESPONSE_OK;

    close(fd);

out:
    fprintf(STDERR_FILENO, "GET,/%s,%p,%s\n", uri, res, Req_id);
    conn_send_response(conn, res);

}

void handle_unsupported(conn_t *conn) {
    debug("handling unsupported request");

    // send responses
    Request_t *method = conn_get_request(conn); // get the method
    char* req = conn_get_header(conn, "Request-Id");
    char *uri = conn_get_uri(conn);
    fprintf(STDERR_FILENO, "%s,/%s,%s,%s\n", method, uri, &RESPONSE_NOT_IMPLEMENTED, req);
    conn_send_response(conn, &RESPONSE_NOT_IMPLEMENTED);
    
}

void handle_put(conn_t *conn) {

    char *uri = conn_get_uri(conn);
    const Response_t *res = NULL;
    debug("handling put request for %s", uri);

    // Check if file already exists before opening it.
    bool existed = access(uri, F_OK) == 0;
    debug("%s existed? %d", uri, existed);

    // Open the file..
    int fd = open(uri, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    if (fd < 0) {
        debug("%s: %d", uri, errno);
        if (errno == EACCES || errno == EISDIR || errno == ENOENT) {
            res = &RESPONSE_FORBIDDEN;
            goto out;
        } else {
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            goto out;
        }
    }

    res = conn_recv_file(conn, fd);

    if (res == NULL && existed) {
        res = &RESPONSE_OK;
    } else if (res == NULL && !existed) {
        res = &RESPONSE_CREATED;
    }

    close(fd);

out:
    char *req = conn_get_header(conn, "Request-Id");
    fprintf(STDOUT_FILENO, "PUT,/%s,%s,%s\n",uri, res, req);
    conn_send_response(conn, res);
}
