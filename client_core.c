#include "fast.h"
#include "err_oper.h"
#include "user.h"

#define TC_SERVER_PORT      7777
#define TERM_CHAT_SERVER "127.0.0.1"

static int tc_client_init(void);


int main(int argc, char *argv[])
{
    char work_mode, prev;
    userInfo cur_user;
    int sockfd;

    sockfd = tc_client_init();

    client_command_parse(argc, argv, &work_mode, &cur_user);
    prev = work_mode;
    switch(work_mode) {
        case TC_MSG_SUP:
            break;
        case TC_MSG_SIN:
            break;
        default:
            break;
    }

    exit(EXIT_SUCCESS);
}

static int tc_client_init(void)
{
    int connfd;
    struct sockaddr_in servaddr;

    if((connfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_sys("socket");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(TC_SERVER_PORT);
    if(inet_aton(TERM_CHAT_SERVER, &servaddr.sin_addr) < 0)
        err_sys("inet_aton");

    if(connect(connfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        err_sys("connect");

    return connfd;
}
