#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define FILE_SIZE 1024 * 1024 * 1024 // 1G大小

int main() {
    int fd = open("input.txt", O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    char buffer[1024]; // 缓冲区大小
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = rand() % 256; // 随机生成0-255之间的数
    }

    for (int i = 0; i < FILE_SIZE / sizeof(buffer); i++) {
        ssize_t bytes_written = write(fd, buffer, sizeof(buffer));
        if (bytes_written == -1) {
            perror("write");
            exit(1);
        }
    }

    close(fd);
    printf("文件生成成功！\n");

    return 0;
}
