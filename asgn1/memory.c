#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void get(int fd) {

  char buff[1024];
  int red;

  while ((red = read(fd, buff, 1023)) > 0) {

    buff[red] = '\0';

    if ((write(STDOUT_FILENO, buff, red)) == -1) {
      perror("write");
      exit(1);
    }
  }

  return;
}

int main(int argc, char *argv[]) {

  if (strcmp(argv[1], "get") == 0) {

    if (argc < 3) {
      printf("Invalid Command\n");
      return 1;
    }

    if (argc >= 4) {
      if (argc > 4) {
        printf("Invalid Command\n");
        return 1;
      }

      if (strcmp(argv[3], "n") != 0) {
        printf("Invalid Command\n");
        return 1;
      }

      else {

        if (access(argv[2], F_OK) != 0) {
          printf("Invalid Command\n");
          return 1;
        } else {

          int fd = open(argv[2], O_RDONLY);
          get(fd);
          close(fd);
          return 0;
        }
      }

    }

    else {

      if (access(argv[2], F_OK) != 0) {
        printf("Invalid Command\n");

        return 1;
      } else {

        int fd = open(argv[2], O_RDONLY);
        get(fd);
        close(fd);
        return 0;
      }
    }
  }
  if (strcmp(argv[1], "set") == 0) {

    char buff[1024];

    if (argc < 3) {
      printf("Invalid Command\n");
      return 1;
    }

    if (access(argv[2], F_OK) != 0) {
      printf("Invlaid Command\n");

      return 1;
    }

    int fd = open(argv[2], O_WRONLY | O_TRUNC);

    int red;

    while ((red = read(0, buff, 1023)) > 0) {

      buff[red] = '\0';

      if ((write(fd, buff, red)) == -1) {
        perror("write");
        exit(1);
      }
      // memset(buff, NULL, 1024);
    }

    close(fd);

    return 0;
  } else {

    printf("Invalid Command\n");
    return 1;
  }
}
