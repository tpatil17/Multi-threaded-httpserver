#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

int is_file(char name[]) {

    struct stat n;
    stat(name, &n);

    return S_ISREG(n.st_mode);
}

int main() {

    char read_buf[4096] = "";

    int offset = 0;

    int red = read(STDIN_FILENO, read_buf, 4096);

    char *ret;

    char *token;

    ret = strchr(read_buf, '\n');

    if( ret == NULL){

        //printf("It's me strchr\n");
        write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));
        return 1;

    }

    token = strtok(read_buf, "\n");

    char function[100] = "";
    char file_name[256] = "";

    sscanf(token, "%s %s %n", function, file_name, &offset);

    int val = is_file(file_name);

    if (access(file_name, F_OK) != 0 | val == 0) {
        //printf("odly its me\n");
        write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));
        return 1;
    }

    char *token2;
    char safe[4096] = "";

    strcpy(safe, token);

    int ctr = 0;

    token2 = strtok(safe, " ");

    while( token2 != NULL){

        ctr +=1;
        token2 = strtok(NULL, " ");
    }

    if(ctr > 2){

         write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));
         //printf("or the ctr: %d\n", ctr);
         return 1;

    }

    if (strcmp(function, "get") == 0) {
        if (red > offset+1) {
            write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));
            //printf("more chars than exp red: %d, exp: %d\n", red, offset);
            return 1;
        }
        int fd = open(file_name, O_RDONLY);

        if (fd < 0) {
            write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));
            return 1;
        }

        char buffer[4096] = "";

        int file_rd = 0;

        while ((file_rd = read(fd, buffer, 4095)) > 0) {

            buffer[file_rd] = '\0';

            write(STDOUT_FILENO, buffer, file_rd);
        }

        close(fd);

        return 0;
    }
    if (strcmp(function, "set") == 0) {

        //printf("in set\n");

        int set_read = 0;
        char buff[4096] = "";

        int fd2 = open(file_name, O_WRONLY | O_TRUNC);

        if (fd2 < 0) {
            write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));
            return 1;
        }

        token = strtok(NULL, "");
        //printf("Current token: %s\n", token);
        if (token != NULL){

            write(fd2, token, strlen(token));
        }

        //printf("token passed\n");

        while ((set_read = read(STDIN_FILENO, buff, 4095)) > 0) {

            buff[set_read] = '\0';

            write(fd2, buff, set_read);
        }

        //printf("reached the end\n");

        close(fd2);
        return 0;

    } else {
        write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));
        return 1;
    }
}
