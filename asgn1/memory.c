#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {

    char read_buf[4096];

    int offset;

    int red = read(STDIN_FILENO, read_buf, 4096);

    char function[100], file_name[256];

    sscanf(read_buf, "%s %s %n", function, file_name, &offset);

    if (access(file_name, F_OK) != 0) {
        write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));
        return 1;
    }

    if (strcmp(function, "get") == 0) {
        if (red > offset) {
            write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command\n"));
            return 1;
        }
        int fd = open(file_name, O_RDONLY);

        char buffer[4096];

        int file_rd;

        while ((file_rd = read(fd, buffer, 4095)) > 0) {

            buffer[file_rd] = '\0';

            write(STDOUT_FILENO, buffer, file_rd);
        }

        close(fd);

        return 0;
    }
    if (strcmp(function, "set") == 0) {

        int set_read;
        char buff[4096];

        int fd2 = open(file_name, O_WRONLY | O_TRUNC);

        while ((set_read = read(STDIN_FILENO, buff, 4095)) > 0) {

            buff[set_read] = '\0';

            write(fd2, buff, set_read);
        }

        close(fd2);

    } else {
        write(STDERR_FILENO, "Invalid Command\n", strlen("Invalid Command"));
        return 1;
    }
}
