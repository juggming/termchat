#include "fast.h"
#include "err_oper.h"

#define TERM_CHAT_SERVER_PORT 7777
#define RECV_BUF_SIZE   4096
#define TERM_CHAT_BACKLOG 20


int get_open_max(void)
{
    return (int)sysconf(_SC_OPEN_MAX);
}

void do_handler_request(int filedes, void *buf, size_t bufsize);


int main(int argc, char *argv[])
{
    struct sockaddr_in servaddr, peeraddr;
    int i, omax, max_index, listenfd, connfd, ready_conn, jobfd;
    socklen_t socklen;
    char serv_buf[RECV_BUF_SIZE];

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_sys("socket");


    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(TERM_CHAT_SERVER_PORT);

    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        err_sys("bind");

    if(listen(listenfd, TERM_CHAT_BACKLOG) < 0)
        err_sys("listen");


    omax = get_open_max();
    struct pollfd client[omax];

    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;
    for(i = 1; i < omax; i++)
        client[i].fd = -1;
    max_index = 0;

    /*
     * Main loop body
     */
    while(true){
        if((ready_conn = poll(client, max_index + 1, -1)) < 0)
            err_sys("poll");

        if(client[0].revents & POLLRDNORM) {
            socklen = sizeof(peeraddr);
            if((connfd = accept(listenfd, (struct sockaddr *)&peeraddr, &socklen)) < 0)
                err_sys("accept");

            for(i = 1; i < omax; i++)
                if(client[i].fd < 0) {
                    client[i].fd = connfd;
                    break;
                }
            if(i == omax)
                err_quit("too many clients");

            client[i].events = POLLRDNORM;
            if(i > max_index)
                max_index = i;

            if(--ready_conn <= 0)
                continue;
        }

        // handling connections
        for(i = 1; i <= max_index; i++) {
            if((jobfd = client[i].fd) < 0)
                continue;
            if(client[i].revents & (POLLRDNORM | POLLERR))
                do_handler_request(jobfd, serv_buf, RECV_BUF_SIZE);

            if(--ready_conn <= 0)
                break;
        }
    }
}


void do_handler_request(int filedes, void *buf, size_t bufsize)
{

}
