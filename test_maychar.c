#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/mychar"

int main()
{
    int fd;
    char write_buf[] = "Hello, this is a test message from user space!";
    char read_buf[1024];
    ssize_t ret;

    //打开设备
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("打开设备失败");
        return -1;
    }
    printf("[测试] 设备打开成功\n");

    //写数据
    ret = write(fd, write_buf, strlen(write_buf));
    if (ret < 0) {
        perror("写入设备失败");
        close(fd);
        return -1;
    }
    printf("[测试] 写入成功，写了 %zd 字节\n", ret);
    printf("[测试] 写入内容：\"%s\"\n", write_buf);

    //关闭再重新打开，重置读取位置
    close(fd);
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("重新打开设备失败");
        return -1;
    }

    //读数据
    memset(read_buf, 0, sizeof(read_buf));
    ret = read(fd, read_buf, sizeof(read_buf));
    if (ret < 0) {
        perror("读取设备失败");
        close(fd);
        return -1;
    }
    printf("[测试] 读取成功，读了 %zd 字节\n", ret);
    printf("[测试] 读取内容：\"%s\"\n", read_buf);

    //验证结果
    if (strcmp(write_buf, read_buf) == 0) {
        printf("[测试] 验证通过！写入和读取的内容一致。\n");
    }
    else {
        printf("[测试] 验证失败！写入和读取的内容不一致。\n");
    }

    //关闭设备
    close(fd);
    printf("[测试] 设备关闭，测试结束\n");

    return 0;
}
