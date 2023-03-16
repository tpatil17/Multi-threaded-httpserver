
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
#include <sys/file.h>

#define OPTIONS "t:"

int threads;

pthread_mutex_t creator_lock = PTHREAD_MUTEX_INITIALIZER; // file creator lock

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

// write the audit log

void write_log(conn_t conn, const Response_t *res, char *uri){
    
    char *req_id = conn_get_header(conn, "Request-Id"); //req id

    int req;

    req = atoi(req_id);

    unit16_t code = response_get_code(res); // status code

    const Request_t *method = conn_get_request(conn); // get the method

    const char *opr = request_get_str(method);

    fprintf(stderr, "%s,/%s,%d,%d\n", opr, uri, (int) code, req);

    return;
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
 
        pthread_create(&thread_pool[j], NULL, worker_threads, NULL);
  
    }

    //fprintf(stderr, "threads have been created\n");

    // initialize the queue

    task_queue = queue_new(10);


    while (1) {
 
        intptr_t connfd;
        connfd = listener_accept(&sock);

        queue_push(task_queue, (void *)connfd); // Push the task to queue

        
    }
    return EXIT_SUCCESS;
}

void *worker_threads(){
    intptr_t conn;

    while(true){
  
        
        queue_pop(task_queue,(void **)&conn);
        

  
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



    char *uri = conn_get_uri(conn);

    const Response_t *res = NULL;

    pthread_mutex_lock(&creator_lock);    

    struct stat st = {0};
    stat(uri, &st);

    if(S_ISDIR(st.st_mode)!= 0){
        res = &RESPONSE_FORBIDDEN;
        conn_send_response(conn, res);
      
    }

    int fd = open(uri, O_RDONLY);
    
    if(fd < 0){
        if(access(uri, F_OK) != 0){
            res = &RESPONSE_NOT_FOUND;
            conn_send_response(conn, res);
            write_log(conn, res, uri);
            pthread_mutex_unlock(&creator_lock);
            close(fd);
            return;
  
        }
        if(errno == EACCES || errno == EISDIR){
            res = &RESPONSE_FORBIDDEN;
            conn_send_response(conn, res);
            write_log(conn, res, uri);
            pthread_mutex_unlock(&creator_lock);
            close(fd);
            return;

        }
        else{
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            conn_send_response(conn, res);
            write_log(conn, res, uri);
            pthread_mutex_unlock(&creator_lock);
            close(fd);
            return;

        }
    }


    flock(fd, LOCK_SH); // acquire reader lock

    pthread_mutex_unlock(&creator_lock);

    uint64_t size = st.st_size;

    res = conn_send_file(conn, fd, size); // send contents

    write_log(conn, res, uri);

    close(fd);

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

    //debug("%s existed? %d", uri, existed);
    char *Req_id = conn_get_header(conn, "Request-Id");

    int code = 0;

    // Open the file..

    // Check if file already exists before opening it.
    pthread_mutex_lock(&creator_lock);

    bool existed = access(uri, F_OK) == 0;

    int fd = open(uri, O_CREAT | O_WRONLY, 0600);
    
    if (fd < 0) {
        //debug("%s: %d", uri, errno);
        if (errno == EACCES || errno == EISDIR || errno == ENOENT) {
            res = &RESPONSE_FORBIDDEN;
            pthread_mutex_unlock(&creator_lock);
            goto out2;
        } else {
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            pthread_mutex_unlock(&creator_lock);
            goto out2;
        }
    }

    flock(fd, LOCK_EX);
    
    pthread_mutex_unlock(&creator_lock);

    ftruncate(fd, 0);


    res = conn_recv_file(conn, fd);

    if (res == NULL && existed) {
        res = &RESPONSE_OK;
    } else if (res == NULL && !existed) {
        res = &RESPONSE_CREATED;
    }

   
out2:

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

    close(fd);
}
