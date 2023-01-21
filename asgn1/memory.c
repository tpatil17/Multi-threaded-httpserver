#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

int is_file(char name[]) {

    struct stat n;
    memset(&n, 0, sizeof(struct stat));
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

    char temp[4096] = "";

    ret = strchr(read_buf, '\n');

    if (ret == NULL) {

        write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));
        return 1;
    }
    strcpy(init, ret);

    token = strtok(read_buf, "\n");

    char function[100] = "";
    char file_name[256] = "";

    int j = 0;

    int num = strlen(token);

    //printf("num: %d\n", num);
    //printf("red: %d\n", red);
    //printf("red-num: %d\n", red - num);

    for (j = 0; j < (red - num); j++) {

        temp[j] = read_buf[num + j];
    }

    sscanf(token, "%s %s %n", function, file_name, &offset);
    int val = 0;
    val = is_file(file_name);

    if (val != 0) {

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
        // printf("or the ctr: %d\n", ctr);
        return 1;
    }

    if (strcmp(function, "get") == 0) {
        if (red > offset + 1) {
            write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));

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

        int set_read = 0;
        char buff[4096] = "";

        int fd2 = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0777);

        if (fd2 < 0) {
            write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));

            return 1;
        }

        token = strtok(NULL, "");

        if (token != NULL) {

            write(fd2, token, strlen(token));
        }

        int i = 0;

        char new_temp[4096] = "";

        for (i = 0; i < ((red - num)); i++) {

            new_temp[i] = read_buf[(num + 1) + i];
        }

        //printf("len of temp: %d\n", (red - num)-1);

        write(fd2, new_temp, (red - num) - 1);

        while ((set_read = read(STDIN_FILENO, buff, 4095)) > 0) {

            buff[set_read] = '\0';

            write(fd2, buff, set_read);
        }

        close(fd2);
        write(STDOUT_FILENO, "OK\n", strlen("OK\n"));
        return 0;

    } else {

        write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));
        return 1;
    }
}
