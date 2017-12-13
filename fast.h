#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <poll.h>
#include <sys/times.h>


/*
 * function: 从filedes读取size长的字节至buf中
 *      参数：
 *          1. filedes:文件描述符
 *          2. buf
 *          3. size <= sizeof(buf)
 *      Return: -1 on failure, otherwise return read bytes.
 */

ssize_t nread(int filedes, void *buf, size_t size)
{
    ssize_t rbytes;
    size_t nleft;

    char *ptr = (char *)buf;
    nleft = size;
    while(nleft > 0) {
        if((rbytes = read(filedes, ptr, nleft)) < 0)
        {
            if(errno == EINTR)
                rbytes = 0;
            else
                return (-1);
        }
        else if(rbytes == 0)
            break;
        nleft -= rbytes;
        ptr += rbytes;
    }
    return (size - nleft);
}

ssize_t nwrite(int filedes, const void *buf, size_t size)
{
    size_t nleft;
    ssize_t wbytes;
    const char *ptr = (const char *)buf;

    nleft = size;
    while(nleft > 0) {
        if((wbytes = write(filedes, ptr, nleft)) < 0) {
            if(wbytes < 0 && errno == EINTR)
                wbytes = 0;
            else
                return (-1);
        }
        nleft -= wbytes;
        ptr += wbytes;
    }
    return size;
}

