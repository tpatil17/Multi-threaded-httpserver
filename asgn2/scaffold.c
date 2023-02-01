#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "asgn2_helper_funcs.h"
#include <string.h>
#include <errno.h>
#include <err.h>

/**
   Converts a string to an 16 bits unsigned integer.
   Returns 0 if the string is malformed or out of the range.
 */
uint16_t strtouint16(char number[]) {
  char *last;
  long num = strtol(number, &last, 10);
  if (num <= 0 || num > UINT16_MAX || *last != '\0') {
    return 0;
  }
  return num;
}

/*
a struct to store the response
*/

struct Response {
  char version[24] = "";
  int status_code = 0;
  char status_phrase[100] = "";
  char header[19] = "";
  long length = 0;
  char message[64] = "";
};

void handle_connection(int connfd){
    char buffer[4096] = "";

    char resp_buffer[4096] = "";

    int bytes_read = 0;

    struct Response res;
    
    bytes_read = read(connfd, buffer, 4096);

    if (write(STDOUT_FILENO, buffer, bytes_read) < 0){
        res.status_code = 400;
        strcpy(res.version, "HTTP/1.1");
        strcpy(res.status_phrase, "Bad Request");
        res.length = strlen("Bad Request\n");
        strcpy(res.header, "Content-Length");
        strcpy(res.message, "Bad Request\n");
        sprintf(resp_buf, "%s %d %s\r\n%s: %ld\r\n\r\n%s", res.version,
            res.status_code, res.status_phrase, res.header, res.length,
            res.message);
        write(connfd, resp_buf, strlen(resp_buf));

        strcpy(resp_buf, "");

    }
        res.status_code = 200;
        strcpy(res.version, "HTTP/1.1");
        strcpy(res.status_phrase, "OK");
        res.length = strlen("OK\n");
        strcpy(res.header, "Content-Length");
        strcpy(res.message, "OK\n");
        sprintf(resp_buf, "%s %d %s\r\n%s: %ld\r\n\r\n%s", res.version,
            res.status_code, res.status_phrase, res.header, res.length,
            res.message);
        write(connfd, resp_buf, strlen(resp_buf));

        strcpy(resp_buf, "");


    return;


}

int main(int argc, char *argv[]){

    int listen_val = 0;

    uint16_t port = 0;

    if(argc != 2){
        errx(EXIT_FAILURE, "Wrong Arguments: %s port_num", argv[0]);
    }

    port = strtouint16(argv[1]);

    if (port < 0 | port > 65535){
        errx(1, "Invalid Port\n");
    }

    Listener_Socket sock;

    listen_val = listener_init(&sock, port);

    if(listen_val != 0){
        errx(1, "Invalid Port\n");
    }

    while(1){
        int connfd = listener_accept(&sock);
        if(connfd < 0){
            warn("Accept Error");
            continue;
        }
        handle_connection(connfd);
        
        close(connfd);
        
    }


    return 0;
}

