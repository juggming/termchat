#include "fast.h"
#include "err_oper.h"
#include "user.h"
#include <sys/epoll.h>
#include <linux/limits.h>
#include <assert.h>
#include <sys/stat.h>
#include <db.h>

#define TC_USER_INFO_DB     "termchat.db"

#define TC_SERVER_PORT      7777
#define TC_LISTEN_BACKLOG   20
#define MAX_BUF_SIZE        4096
#define EPOLL_ARRAY_SIZE    20

#define TC_FILE_MODE    (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)


static int tc_core_init(uint16_t port);
static void userinfo_db_init(const char *dbname);
void set_sockopt_nonblocking(int sockfd);
void do_handler_request(int sockfd);

void handler_user_login(int sockfd);
void handler_user_register(int sockfd);
void handler_recv_file(int sockfd);
void handler_regular_message(int sockfd);

int main(int argc, char *argv[])
{
    struct epoll_event ev, event_array[EPOLL_ARRAY_SIZE];
    int listenfd, epfd;

    listenfd = tc_core_init(TC_SERVER_PORT);

    epfd = epoll_create1(0); // FD_CLOEXEC
    ev.data.fd = listenfd;
    ev.events = EPOLLIN | EPOLLET;

    if((epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev)) < 0)
        err_quit("epoll_ctl");

    // Main loop
    while(1) {
        int ready_conns, sockfd, index;
        struct sockaddr_in peeraddr;
        socklen_t socklen;
        ready_conns = epoll_wait(epfd, event_array, EPOLL_ARRAY_SIZE, -1);

        for(index = 0; index < ready_conns; index++) {
            if((event_array[index].events & EPOLLERR) ||
                    (event_array[index].events & EPOLLHUP)) {
                err_msg("epoll");
                close(event_array[index].data.fd);
                continue;
            }
            else if(event_array[index].data.fd == listenfd) {
                /*
                 * it means new connection arrives, add it to event_array[]
                 */

                socklen = sizeof(peeraddr);
                sockfd = accept(listenfd, (struct sockaddr *)&peeraddr, &socklen);
                if(sockfd < 0) {
                    if((errno == EAGAIN) || (errno == EWOULDBLOCK))
                        break;
                    else {
                        perror("accept");
                        break;
                    }
                }

                // show connection informations about peer client
                if((getpeername(sockfd, (struct sockaddr *)&peeraddr, &socklen)) < 0)
                    perror("getpeername");
                set_sockopt_nonblocking(sockfd);

                ev.data.fd = sockfd;
                ev.events = EPOLLIN |  EPOLLET;
                if((epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev)) < 0)
                    err_quit("epoll_ctl");
            }
            else {
                /* handler ready for reading and writing */
                if((event_array[index].events & EPOLLIN) || (event_array[index].events & EPOLLET))
                    do_handler_request(event_array[index].data.fd);
            }
        }
    }
}

static int tc_core_init(uint16_t port)
{
    struct sockaddr_in servaddr;
    int servfd;

    if((servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_quit("socket");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    servaddr.sin_family = AF_INET;

    set_sockopt_nonblocking(servfd);

    if(bind(servfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        err_quit("bind");

    if(listen(servfd, TC_LISTEN_BACKLOG) < 0)
        err_quit("listen");

    return servfd;
}

void set_sockopt_nonblocking(int sockfd)
{
    int sockopt;

    if((sockopt = fcntl(sockfd, F_GETFL, 0)) < 0)
        err_quit("fcntl");

    sockopt |= O_NONBLOCK;
    if(fcntl(sockfd, F_SETFL, sockopt) < 0)
        err_quit("fcntl");
}


void do_handler_request(int sockfd)
{
    char        msg_type;
    ssize_t     rbytes;

    rbytes = nread_nonblock(sockfd , &msg_type, sizeof(msg_type));

    switch(msg_type) {
        case TC_REG_MSG:
            handler_regular_message(sockfd);
            break;
        case TC_MSG_SIN:
            handler_user_login(sockfd);
            break;
        case TC_MSG_SUP:
            handler_user_register(sockfd);
            break;
        case TC_FILE_SYN:
            handler_recv_file(sockfd);
        case TC_MSG_ERR:
            // this should not happen for server
            break;
        case TC_FILE_ERR:
            break;
        default:
            // unsupported message
            break;
    }
}



void handler_user_login(int sockfd)
{

}

void handler_recv_file(int sockfd)
{
    char    recv_buf[MAX_BUF_SIZE];
    char    filename[NAME_MAX];
    char    filenameLen;
    struct sockaddr_in peeraddr;
    socklen_t socklen;
    int filedes;
    ssize_t rbytes;

    getpeername(sockfd, (struct sockaddr *)&peeraddr, &socklen);
    nread_nonblock(sockfd, &filenameLen, sizeof(filenameLen));
    assert(filenameLen < NAME_MAX);
    nread_nonblock(sockfd, &filename, filenameLen);
    filename[(int)filenameLen] = '\0'; // add a null terminate

    filedes = open(filename, O_CREAT | O_EXCL, TC_FILE_MODE);
    if(filedes < 0) {

    }

    while((rbytes = (nread_nonblock(sockfd, recv_buf, MAX_BUF_SIZE))) > 0) {
        nwrite_nonblock(filedes, recv_buf, rbytes);
    }
    close(filedes);
    fprintf(stdout, "Receive %s from %s:%d\n", filename,
            inet_ntoa(peeraddr.sin_addr), peeraddr.sin_port);
}
void handler_regular_message(int sockfd)
{

}


static void userinfo_db_init(const char *dbname)
{
    DB *ptrdb;
    u_int32_t flags;
    int ret;

    ret = db_create(&ptrdb,NULL, 0);
    if(ret < 0)
        ptrdb->err(ptrdb, ret, "%s", dbname);

    flags = DB_CREATE | DB_EXCL;
    ret = ptrdb->open(ptrdb, NULL, dbname, NULL, DB_BTREE, flags, 0);
    if(ret < 0)
        ptrdb->err(ptrdb, ret, "%s",dbname);
    ptrdb->close(ptrdb, 0);
}

void handler_user_register(int sockfd)
{
    DB      *ptrdb;
    u_int32_t   flags;
    int     ret;
    DBT     key, data;
    userInfo user;

    //fill up userInfo data
    nread_nonblock(sockfd, user.ui_name, MAX_NAME_LENGTH);
    nread_nonblock(sockfd, user.ui_passwd, MAX_PASSWD_LENGTH);

    user.ui_regtime = time(NULL);
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.data = user.ui_name;
    key.size = sizeof(MAX_NAME_LENGTH);

    data.data = &user;
    data.size = sizeof(user);

    ret = db_create(&ptrdb, NULL, 0);
    if(ret < 0)
        ptrdb->err(ptrdb, ret, "%s", TC_USER_INFO_DB);

    flags = DB_CREATE;
    ret = ptrdb->open(ptrdb, NULL, TC_USER_INFO_DB, NULL, DB_BTREE, flags, 0);
    if(ret < 0)
        ptrdb->err(ptrdb, ret, "%s", TC_USER_INFO_DB);
    ret = ptrdb->put(ptrdb, NULL, &key, &data, DB_NOOVERWRITE);
    if(ret < 0) {
        if(ret == DB_KEYEXIST)
            // you need to notify the client
            ptrdb->err(ptrdb, ret, "%s", TC_USER_INFO_DB);
        else
            ptrdb->err(ptrdb, ret, "%s", TC_USER_INFO_DB);
    }

    ptrdb->close(ptrdb, 0);
}
