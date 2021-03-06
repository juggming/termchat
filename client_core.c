#include "fast.h"
#include "err_oper.h"
#include "user.h"

#define TC_SERVER_PORT      7777
#define TERM_CHAT_SERVER "127.0.0.1"
#define TC_ERR_BUF_SIZE     80
#define MAX_BUF_SIZE        4096

static int tc_client_init(void);
static int do_user_login(int sockfd, const userInfo *uinfo);
static int do_user_register(int sockfd, const userInfo *uinfo);
static int do_send_file(int connfd, const char *filename);
static void do_send_message(int connfd, void *msg, size_t msgsize);
static int parse_operation_replay(int sockfd, const char mode);

static void switch_working_mode(int sig);

static char work_mode;


int main(int argc, char *argv[])
{
    char prev_mode;
    userInfo cur_user;
    int sockfd;

    sockfd = tc_client_init();

    client_command_parse(argc, argv, &work_mode, &cur_user);
    prev_mode = work_mode;
    switch(work_mode) {
        case TC_MSG_SUP:
            do_user_register(sockfd, &cur_user);
            if(parse_operation_replay(sockfd, TC_MSG_SUP) < 0) {
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            break;
        case TC_MSG_SIN:
            do_user_login(sockfd, &cur_user);
            if(parse_operation_replay(sockfd, TC_MSG_SIN) < 0) {
                perror("do_user_login");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            break;
        default:
            // do nothing here
            break;
    }
    signal(SIGINT, switch_working_mode);
    while(1) {
        int linemax = get_maxline_size();
        char msg_buf[linemax];

        if(work_mode & TC_REG_MSG) {
            fprintf(stdout, "\n<message mode>: ");
            while(fgets(msg_buf, linemax, stdin) != NULL)
                nwrite(sockfd, msg_buf, linemax);
        }
        else if(work_mode & TC_FILE_SYN) {
            fgets(msg_buf, linemax, stdin);
            do_send_file(sockfd, msg_buf);
        }
    }

    close(sockfd);
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


static int do_user_register(int sockfd, const userInfo *uinfo)
{
    char *reg_buf;
    char recv_buf[TC_ERR_BUF_SIZE];

    reg_buf = (char *)alloca(sizeof(userInfo) + 1);
    if(reg_buf == NULL)
        err_sys("alloca");
    *reg_buf = TC_MSG_SUP;
    memcpy(reg_buf+1, uinfo, sizeof(userInfo));
    nwrite(sockfd, reg_buf, sizeof(uinfo) + 1);
    nread(sockfd, recv_buf, TC_ERR_BUF_SIZE);

    if(recv_buf[0] == TC_OPER_OK)
        return 0;
    else {
        fprintf(stdout, "do_user_register() failed: %s\n", recv_buf);
        return -1;
    }
}


static int do_user_login(int sockfd, const userInfo *uinfo)
{
    char *reg_buf;
    char recv_buf[TC_ERR_BUF_SIZE];

    reg_buf = (char *)alloca(sizeof(userInfo) + 1);
    if(reg_buf == NULL)
        err_sys("alloca");
    *reg_buf = TC_MSG_SIN;
    memcpy(reg_buf+1, uinfo, sizeof(userInfo));
    nwrite(sockfd, reg_buf, sizeof(uinfo) + 1);
    nread(sockfd, recv_buf, TC_ERR_BUF_SIZE);

    if(recv_buf[0] == TC_OPER_OK)
        return 0;
    else {
        fprintf(stdout, "do_user_register() failed: %s\n", recv_buf);
        return -1;
    }
}


static int do_send_file(int connfd, const char *filename)
{
    size_t filenameLen = strlen(filename);
    char send_buf[MAX_BUF_SIZE], ch;
    int filedes;
    ssize_t rbytes;

    ch = TC_FILE_SYN;
    write(connfd, &ch, 1);
    write(connfd, &filenameLen, 1);
    write(connfd, filename,filenameLen);


    if((filedes = open(filename, O_RDONLY)) < 0) {
        fprintf(stderr, "open %s failed: %s\n", filename, strerror(errno));
        return -1;
    }
    while((rbytes = nread(filedes, send_buf, MAX_BUF_SIZE)) > 0) {
        nwrite(connfd, send_buf, rbytes);
    }
    close(filedes);
    return (parse_operation_replay(connfd, TC_FILE_SYN));
}

static int parse_operation_replay(int sockfd, const char mode)
{
    if(mode == TC_REG_MSG)
        return 0;
    else {
        char *ack_buf = (char*)alloca(TC_ERR_BUF_SIZE);
        if(ack_buf == NULL) {
            fprintf(stderr, "alloca: %s\n", strerror(errno));
            return -1;
        }
        read(sockfd, ack_buf, 1); // read the first byte
        switch(*ack_buf) {
            case TC_OPER_OK:
                // do nothing
                return 0;
                break;
            case TC_MSG_ERR:
            case TC_FILE_ERR:
                nread(sockfd, ack_buf, TC_ERR_BUF_SIZE);
                fprintf(stderr, "%s\n", ack_buf);
                break;
            default:
                fprintf(stderr, "Invalid operation replay\n");
                break;
        }
        return -1;
    }
}


static void do_send_message(int connfd, void *msg, size_t msgsize)
{
    char first_byte = TC_REG_MSG;
    write(connfd, &first_byte, 1);
    nwrite(connfd, msg, msgsize);
}


static void switch_working_mode(int sig)
{
    if(sig == SIGINT) {
        if(work_mode & TC_REG_MSG)
            work_mode = TC_FILE_SYN;
        else
            work_mode = TC_REG_MSG;
    }
}

