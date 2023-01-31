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

void handle_connection(int connfd){
    char buffer[4096] = "";

    int bytes_read = 0;
    
    bytes_read = read(connfd, buffer, 4096);

    write(STDOUT_FILENO, buffer, bytes_read);

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

    Listener_Socket *sock = NULL;

    listen_val = listener_init(sock, port);

    if(listen_val != 0){
        errx(1, "Invalid Port\n");
    }

    while(1){
        int connfd = listener_accept(sock);
        if(connfd < 0){
            warn("Accept Error");
            continue;
        }
        handle_connection(connfd);
        
        close(connfd);
        
    }


    return 0;
}

