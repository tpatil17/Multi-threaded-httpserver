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

    return S_ISDIR(n.st_mode);
}

int main() {

    char read_buf[4096] = "";

    int offset = 0;

    int red = read(STDIN_FILENO, read_buf, 4096);

    char *ret;

    char init[4096] = "";

    char *token;
    //printf("try out\n");

    ret = strchr(read_buf, '\n');

    if (ret == NULL) {

        //  write(STDERR_FILENO, "It's me strchr\n", strlen("It's me strchr\n"));
        write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));
        return 1;
    }
    strcpy(init, ret);

    //printf("%s\n", ret); // ----i-- remove --

    token = strtok(read_buf, "\n");

    char function[100] = "";
    char file_name[256] = "";

    sscanf(token, "%s %s %n", function, file_name, &offset);

    int val = is_file(file_name);

    // printf("token: %s\n", token); // remove --------------

    if (val != 0) {
        //printf("odly its me\n");
        //printf("%s\n", file_name);
        //	printf("%d\n", val);
        //	printf("still a problem\n");
        write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));
        return 1;
    }

    char *token2;
    char safe[4096] = "";

    strcpy(safe, token);

    int ctr = 0;

    token2 = strtok(safe, " ");

    while (token2 != NULL) {

        ctr += 1;
        token2 = strtok(NULL, " ");
    }

    if (ctr > 2) {

        write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));
        printf("or the ctr: %d\n", ctr);
        return 1;
    }

    if (strcmp(function, "get") == 0) {
        if (red > offset + 1) {
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

        //write(STDOUT_FILENO,"in set\n" ,strlen("in set\n"));

        int set_read = 0;
        char buff[4096] = "";

        int fd2 = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0777);

        if (fd2 < 0) {
            write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));
            //printf("%d\n", fd2);
            //perror( "Error: \n");
            return 1;
        }

        //printf("token again: %s\n", token);

        //	int off = 0;

        //	off = strlen(token);

        token = strtok(NULL, "");

        //printf("Current token: %s\n", token);
        if (token != NULL) {

            write(fd2, token, strlen(token));
        }

        unsigned long i = 0;

        char new_init[4096] = "";

        for (i = 0; i < (strlen(init) - 1); i++) {

            new_init[i] = init[i + 1];
        }

        //lseek(fd2, 1, SEEK_CUR);
        //if (ret != NULL){
        // printf("%lu\n", strlen(init)); // ---- remove

        write(fd2, new_init, strlen(new_init));
        //}
        //printf("%s\n", token); /////// remove --------------------

        //printf("token passed\n");
        //

        //lseek(fd2, 0, SEEK_CUR);

        while ((set_read = read(STDIN_FILENO, buff, 4095)) > 0) {

            buff[set_read] = '\0';

            //	    printf("%s", buff);

            write(fd2, buff, set_read);
        }

        //printf("reached the end\n");

        close(fd2);
        write(STDOUT_FILENO, "OK\n", strlen("OK\n"));
        return 0;

    } else {
        //write(STDOUT_FILENO, "problem found\n", strlen("problem found\n"));
        write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));
        return 1;
    }
}
