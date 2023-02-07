#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "asgn2_helper_funcs.h"
#include <string.h>
#include <errno.h>
#include <err.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>


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
  char version[24];
  int status_code;
  char status_phrase[100];
  char header[19];
  long length;
  char message[64];
};

struct Request
{
    char method[10];
    char uri[64];
    char version[25];
    char header[50];
    char value[50];
    int off_set;
    int err_flag; // 1: Bad Request, 2: Internal Server Error, 3: File not Found
    int length;
    int size;

};

struct Request process_request(char req_buffer[]){

    struct Request req;

    char p_buf[1024] = "";

    strcpy(req.method, "");
    strcpy(req.uri, "");
    strcpy(req.version, "");
    strcpy(req.header, "");
    strcpy(req.value, "");
    
    //struct Response res;

  const char delim[2] = "\n";

  char *token;

  int total = 0;

  char perm_header[25] = "";

  char perm_val[24] = "";

  int check = 0;

  req.err_flag = 0;

  int ctr = 0;

  req.size = strlen(req_buffer);

  token = strtok(req_buffer, delim);

  strcpy(p_buf, token);

     if(token == NULL){

        req.err_flag = 5; // nc -zv do not wirte to log just return

        return req;

    }

  if ((check = sscanf(p_buf, "%s /%s %s %n", req.method, req.uri, req.version,
                      &req.off_set)) != 3) {

    
    strcpy(req.method, "");
    strcpy(req.version, "");
    strcpy(req.uri, "");

    sscanf(p_buf, "%s %s %s", req.method, req.uri, req.version);

    if (strcmp(req.uri, "/") == 0 && strcmp(req.version, "HTTP/1.1") == 0) {

    //   strcpy(res.version, "HTTP/1.1");
    //   res.status_code = 500;
    //   strcpy(res.status_phrase, "Internal Server Error");
    //   strcpy(res.header, "Content-Length");
    //   strcpy(res.message, "Internal Server Error\n");
    //   res.length = strlen(res.message);
    //   sprintf(buffer, "%s %d %s\r\n%s: %ld\r\n\r\n%s", res.version,
    //           res.status_code, res.status_phrase, res.header, res.length,
    //           res.message);
    //   write(connfd, buffer, strlen(buffer));

      req.err_flag = 2;

      strcpy(p_buf, "");
      return req;
    }

    // strcpy(res.version, "HTTP/1.1");
    // res.status_code = 400;
    // strcpy(res.status_phrase, "Bad Request");
    // strcpy(res.header, "Content-Length");
    // res.length = 12;
    // sprintf(buffer, "%s %d %s\r\n%s: %ld\r\n\r\n", res.version, res.status_code,
    //         res.status_phrase, res.header, res.length);
    // write(connfd, buffer, strlen(buffer));
    // write(connfd, "Bad Request\n", 12);

    req.err_flag = 1;

    strcpy(p_buf, "");
    return req;
  }

  token = strtok(NULL, delim);

  total = req.off_set;

  while (token != NULL) {

    strcpy(p_buf, token);

    sscanf(p_buf, "%s %s %n", req.header, req.value, &req.off_set);

    char temp_1[25] = "";
    char temp_2[25] = "";
    char temp_3[25] = "";

    if (sscanf(p_buf, "%s %s %s", temp_1, temp_2, temp_3) == 3) {

    //   strcpy(res.version, "HTTP/1.1");
    //   res.status_code = 400;
    //   strcpy(res.status_phrase, "Bad Request");
    //   strcpy(res.header, "Content-Length");
    //   res.length = 12;
    //   sprintf(buffer, "%s %d %s\r\n%s: %ld\r\n\r\n", res.version,
    //           res.status_code, res.status_phrase, res.header, res.length);
    //   write(connfd, buffer, strlen(buffer));
    //   write(connfd, "Bad Request\n", 12);

      req.err_flag = 1;

      strcpy(p_buf, "");
      strcpy(temp_1, "");
      strcpy(temp_2, "");
      strcpy(temp_3, "");

      return req;
    }

    if (strcmp(req.header, "") == 0 && strcmp(req.value, "") == 0) {

      if (ctr == 0 &&
          (strcmp(req.method, "GET") != 0 | strcmp(req.method, "get") != 0)) {

        // strcpy(res.version, "HTTP/1.1");
        // res.status_code = 400;
        // strcpy(res.status_phrase, "Bad Request");
        // strcpy(res.header, "Content-Length");
        // res.length = 12;
        // sprintf(buffer, "%s %d %s\r\n%s: %ld\r\n\r\n", res.version,
        //         res.status_code, res.status_phrase, res.header, res.length);
        // write(connfd, buffer, strlen(buffer));
        // write(connfd, "Bad Request\n", 12);

        req.err_flag = 1;

        strcpy(p_buf, "");
        return req;
      }

      break;
    }

    if (strcmp(req.header, "Content-Length:") == 0) {

      strcpy(perm_header, req.header);

      char temp1[25] = "";
      char temp2[25] = "";
      char temp3[25] = "";

      int bad_flag = 0;

      if (sscanf(p_buf, "%s %s %s", temp1, temp2, temp3) == 3) {

        bad_flag = 1;

        strcpy(temp1, "");
        strcpy(temp2, "");
        strcpy(temp3, "");
      }

      int val_ln = 0;

      val_ln = strlen(req.value);

      int i;

      for (i = 0; i < val_ln; i++) {

        if (!isdigit(req.value[i])) {

          bad_flag = 1;
          break;
        }
      }

      if (strcmp(req.value, "") == 0 | bad_flag == 1) {
        // strcpy(res.version, "HTTP/1.1");
        // res.status_code = 400;
        // strcpy(res.status_phrase, "Bad Request");
        // strcpy(res.header, "Content-Length");
        // res.length = 12;
        // sprintf(buffer, "%s %d %s\r\n%s: %ld\r\n\r\n", res.version,
        //         res.status_code, res.status_phrase, res.header, res.length);
        // write(connfd, buffer, strlen(buffer));
        // write(connfd, "Bad Request\n", 12);

        req.err_flag = 1;

        strcpy(p_buf, "");
        return req; // bad request
      }

      strcpy(perm_val, req.value);
    }

    if (strcmp(req.header, "") == 0) {

    //   strcpy(res.version, "HTTP/1.1");
    //   res.status_code = 400;
    //   strcpy(res.status_phrase, "Bad Request");
    //   strcpy(res.header, "Content-Length");
    //   res.length = 12;
    //   sprintf(buffer, "%s %d %s\r\n%s: %ld\r\n\r\n", res.version,
    //           res.status_code, res.status_phrase, res.header, res.length);
    //   write(connfd, buffer, strlen(buffer));
    //   write(connfd, "Bad Request\n", 12);

      req.err_flag = 1;

      strcpy(p_buf, "");

      return req;
    }

    strcpy(req.header, "");

    strcpy(req.value, "");

    token = strtok(NULL, delim);

    total += req.off_set;

    ctr += 1;
  }

  strcpy(req.header, perm_header);

  strcpy(req.value, perm_val);

  req.off_set = total + 8;

  if (ctr == 1) {

    req.off_set -= 4;
  }
  if (req.off_set > 2048){
    req.err_flag = 7;
  }

  req.length = atoi(perm_val);

  return req;
}




 int Get(char file[], int connfd){

    int fd = 0;

    struct Response res;

    struct stat st;

    stat(file, &st);
    int size = st.st_size;

   
    strcpy(res.status_phrase, "");
    strcpy(res.message, "");
    strcpy(res.header, "");
    strcpy(res.version, "");

    fd = open(file, O_RDONLY);

  

    if(fd < 0){
        errx(EXIT_FAILURE, "File did not open succesfully");
    }
   

    res.status_code = 200;
    strcpy(res.version, "HTTP/1.1");
    strcpy(res.status_phrase, "OK");
    res.length = size;
    strcpy(res.header, "Content-Length");

    dprintf(connfd, "%s %d %s\r\n%s: %ld\r\n\r\n", res.version,
            res.status_code, res.status_phrase, res.header, res.length
            );
   
    strcpy(res.status_phrase, "");
    strcpy(res.message, "");

   

    int passed;
    if((passed = pass_bytes(fd, connfd, size)) < 0){

      errx(EXIT_FAILURE, "pass bytes returned -1");

    }
    
    close(fd);

    return 0;
}

int Put(int connfd, char file[], struct Request req, char buffer[], int bytes_read){

  struct Response res;

  int fd;

  strcpy(res.header, "");
  strcpy(res.message, "");
  strcpy(res.status_phrase, "");
  strcpy(res.version, "");

  strcpy(res.version, "HTTP/1.1");



  if(access(file, F_OK) == 0){
    fd = open(file, O_WRONLY | O_TRUNC);
    res.status_code = 200;
    strcpy(res.status_phrase, "OK");
    res.length = 3;
    strcpy(res.message, "OK\n");
    strcpy(res.header, "Content-Length");
    

  }
  else{
    fd = open(file, O_WRONLY | O_CREAT, 0777);
    res.status_code = 201;
    strcpy(res.status_phrase, "Created");
    res.length = 8;
    strcpy(res.message, "created\n");
    strcpy(res.header, "Content-Length");
    

  }

  int ctr = 0;
  char put_buf[4096] = "";
  while (ctr < (bytes_read - req.off_set)){
    put_buf[ctr] = buffer[req.off_set + ctr];
    ctr+=1;
  }

  if (req.length <= (bytes_read - req.off_set)){

   // dprintf(connfd, "HERE as expected\n");

    if ( (write_all(fd, put_buf, req.length)) < 0){
      errx(1, "fail in writing\n");
    }
    
  close(fd);

  //  dprintf(connfd, "Write done\n");
  dprintf(connfd, "%s %d %s\r\n%s: %ld\r\n\r\n%s\n", res.version,
         res.status_code, res.status_phrase, res.header, res.length, res.message
          );


  }
  
  else{

  //  dprintf(connfd, "should not venture here\n");

    int written = write_all(fd, put_buf, bytes_read - req.off_set);
    
    pass_bytes(connfd, fd, req.length - written );

    close(fd);

    dprintf(connfd, "%s %d %s\r\n%s: %ld\r\n\r\n%s\n", res.version,
         res.status_code, res.status_phrase, res.header, res.length, res.message
          );



  }

  return 0;


}

void handle_connection(int connfd){
    char buffer[4096] = "";

    int bytes_read = 0;
    
    bytes_read = read(connfd, buffer, 4096);

    struct Request req;

    req = process_request(buffer);
    //int val = -1;

    if ((strcmp(req.method, "GET") == 0 )| (strcmp(req.method, "get") == 0)){
        //printf("request processed succesfully, implement get\n");
        //write(connfd,"get is the method to be implemented\n", strlen("get is the method to be implemented\n") );
        
        Get(req.uri, connfd);

    }
    if (strcmp(req.method, "PUT") == 0 | strcmp(req.method, "put") == 0){

      dprintf(connfd, "No problem here lets try to execute function put\n");
        
      Put(connfd, req.uri, req, buffer ,bytes_read);


    }


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

