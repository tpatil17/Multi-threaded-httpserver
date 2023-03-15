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

#define OPTIONS "t:"

int threads;

void handle_connection(int);

void handle_get(conn_t *);
void handle_put(conn_t *);
void handle_unsupported(conn_t *);
void *worker_threads();

queue_t *task_queue = NULL;

static size_t strtouint16(char number[]) {
    char *last;
    long num = strtol(number, &last, 10);
    if (num <= 0 || num > UINT16_MAX || *last != '\0') {
        return 0;
    }
    return num;
}


int main(int argc, char **argv) {

    //fprintf(stdout, "Inside the main thread\n");
    
    
    int opt = 0;
    threads = 4; // Default value

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 't':
            threads = strtol(optarg, NULL, 10);
            if (threads <= 0) {
                errx(EXIT_FAILURE, "bad number of threads");
            }
            break;
   
        default: 
            fprintf(stderr, "./httpserver [-t] <port>\n");
            return 1;
        }
    }

    //fprintf(stdout, "terminal entries checked\n");

    if (optind >= argc) {
        warnx("wrong number of arguments");
        fprintf(stderr, "./httpserver [-t threads] <port>\n");
        return 1;
    }

    //fprintf(stderr, "testing the file\n");

    uint16_t port = strtouint16(argv[optind]);
    if (port == 0) {
        errx(1, "bad port number: %s", argv[1]);
    }

    //fprintf(stderr, "does not seem to have problem in getting port Port: %d\n", port);


    signal(SIGPIPE, SIG_IGN);
    Listener_Socket sock;
    listener_init(&sock, port);

    //fprintf(stdout, "Binding of socket and listener done right\n");

    //fprintf(stderr, "listener works\n");

    int j;

    //fprintf(stderr, "threads %d\n", threads);

    pthread_t thread_pool[threads]; // array of threads

    //fprintf(stderr, "threads %d, a trial to see if thread pool is allocated\n", threads);

    for(j = 0; j < threads; j++){
      //  fprintf(stderr, "creating thread\n");
     //   fprintf(stdout, "creating thread\n");
        pthread_create(&thread_pool[j], NULL, worker_threads, NULL);
   //     fprintf(stdout, "Thread created\n");
   //     fprintf(stderr, "smoothly created\n");
    }

    //fprintf(stderr, "threads have been created\n");

    // initialize the queue

    task_queue = queue_new(10);


    while (1) {
     //   fprintf(stdout, "Inside dispatcher threads while loop\n");
        intptr_t connfd;
        connfd = listener_accept(&sock);
        //printf(stderr, "The pushed value of conn: %d\n", connfd);
        //void *ptr = (void *)connfd;
        queue_push(task_queue, (void *)connfd); // Push the task to queue
    //    fprintf(stderr, "pushed the conn\n");
        
    }
    return EXIT_SUCCESS;
}

void *worker_threads(){
    intptr_t conn;

    while(true){
    //    fprintf(stdout, "Inside the worker threads function\n");
    //    fprintf(stderr, "thread  is in\n");
        
        queue_pop(task_queue,(void **)&conn);
        

    //    fprintf(stderr, "queue pop is smooth\n");
        handle_connection(conn);
        close(conn);
    }
}

void handle_connection(int connfd) {

//    fprintf(stdout, "Handle connection called\n");

    conn_t *conn = conn_new(connfd);

    const Response_t *res = conn_parse(conn);

    if (res != NULL) {
        conn_send_response(conn, res);
    } else {
        //debug("%s", conn_str(conn));
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

  //  fprintf(stdout, "Get successfully called\n");

    char *uri = conn_get_uri(conn);
    //char *Req_id = conn_get_header(conn, "Request-Id");

    //debug("GET request not implemented. But, we want to get %s", uri);

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
    //debug("handling get requests for %s", uri);

    int code = 0;

    char *Req_id = conn_get_header(conn, "Request-Id");


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
    flock(fd, LOCK_SH); // acquire reader lock

    res = conn_send_file(conn, fd, size); // send contents

    flock(fd, LOCK_UN); // release the reader lock

    res = &RESPONSE_OK;

    close(fd);

//    fprintf(stdout, "get completed\n");
    

out:

   
    if(res == &RESPONSE_OK){
        code = 200;

    }else if (res == &RESPONSE_BAD_REQUEST)
    {
        code = 400;

    }else if (res == &RESPONSE_FORBIDDEN)
    {
        
        code = 403;/* code */
    }else if (res == &RESPONSE_NOT_FOUND)
    {
        /* code */
        code = 404;
    }else if (res == &RESPONSE_INTERNAL_SERVER_ERROR)
    {
        /* code */
        code = 500;

    }else if (res == &RESPONSE_NOT_IMPLEMENTED)
    {
        /* code */
        code = 501;

    }else
    {
        code = 505;
        /* code */
    }

    if (Req_id == NULL){
        Req_id = "0";
    }
    
    fprintf(stderr, "GET,/%s,%d,%s\n", uri, code, Req_id);
    //fprintf(stdout, "GET,/%s,%d,%s\n", uri, code, Req_id);
//    fprintf(stdout, "Log for get written\n");

    conn_send_response(conn, res);

}

void handle_unsupported(conn_t *conn) {
    //debug("handling unsupported request");

    // send responses
    fprintf(stdout, "bad operation caught\n");
    const Request_t *method = conn_get_request(conn); // get the method
    const char *opr = request_get_str(method);
    char* req = conn_get_header(conn, "Request-Id");
    char *uri = conn_get_uri(conn);
    //fprintf(stderr, "Unnsu,/%s,%s,%s\n", method, uri, &RESPONSE_NOT_IMPLEMENTED, req);
    int code = 501;
    
    if (req == NULL){
        req = "0";
    }
    fprintf(stderr, "%s,/%s,%d,%s\n",opr, uri, code, req);
    //fprintf(stdout, "%s,/%s,%d,%s\n",opr, uri, code, req);


    conn_send_response(conn, &RESPONSE_NOT_IMPLEMENTED);
    
}

void handle_put(conn_t *conn) {
//    fprintf(stdout, "Put called\n");

    char *uri = conn_get_uri(conn);
    const Response_t *res = NULL;
    //debug("handling put request for %s", uri);

    // Check if file already exists before opening it.
    bool existed = access(uri, F_OK) == 0;
    //debug("%s existed? %d", uri, existed);
    char *Req_id = conn_get_header(conn, "Request-Id");

    int code = 0;

    // Open the file..
    int fd = open(uri, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    if (fd < 0) {
        //debug("%s: %d", uri, errno);
        if (errno == EACCES || errno == EISDIR || errno == ENOENT) {
            res = &RESPONSE_FORBIDDEN;
            goto out;
        } else {
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            goto out;
        }
    }

    flock(fd, LOCK_EX);

    res = conn_recv_file(conn, fd);

    if (res == NULL && existed) {
        res = &RESPONSE_OK;
    } else if (res == NULL && !existed) {
        res = &RESPONSE_CREATED;
    }

     flock(fd, LOCK_UN);

    close(fd);

   
out:

    if(res == &RESPONSE_OK){
        code = 200;
    }
    else if (res ==  &RESPONSE_CREATED)
    {
        /* code */
        code = 201;
    }
    else if (res == &RESPONSE_BAD_REQUEST)
    {
        code = 400;

    }else if (res == &RESPONSE_FORBIDDEN)
    {
        
        code = 403;/* code */
    }else if (res == &RESPONSE_NOT_FOUND)
    {
        /* code */
        code = 404;
    }else if (res == &RESPONSE_INTERNAL_SERVER_ERROR)
    {
        /* code */
        code = 500;

    }else if (res == &RESPONSE_NOT_IMPLEMENTED)
    {
        /* code */
        code = 501;

    }else
    {
        code = 505;
        /* code */
    }
    if (Req_id == NULL){
        Req_id = "0";
    }
    
    
    fprintf(stderr, "PUT,/%s,%d,%s\n", uri ,code , Req_id);
    //fprintf(stdout, "PUT,/%s,%d,%s\n", uri, code , Req_id);


    conn_send_response(conn, res);
}
