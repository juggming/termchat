#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "termchat.h"

#define MAX_NAME_LENGTH 32
#define MAX_PASSWD_LENGTH 32

#define min(a, b) ((a) < (b) ? (a): (b))

typedef struct userInfo {
    char ui_name[MAX_NAME_LENGTH];
    char ui_passwd[MAX_PASSWD_LENGTH];
    time_t ui_regtime; // record user register time
} userInfo;

struct option long_options[] = {
    {"new", no_argument, NULL, 'n'},
    {"login", no_argument, NULL, 'l'},
    {"user", required_argument, NULL, 'u'},
    {"password", required_argument, NULL, 'p'},
    {"help", no_argument, NULL, 0},
    {NULL, 0, NULL , 0},
};

const char *short_options = "nlu:p:h";

void print_termchat_usage(void)
{
    fprintf(stdout, "termchat <-nl> [-u username] [-p password]\n");
}
void client_command_parse(int args, char **pptrarg, char *mode, userInfo *pinfo)
{
    int option_index = 0;
    int c;
    size_t len;

    do {
        c = getopt_long(args, pptrarg, short_options, long_options, &option_index);
        switch(c) {
            case 'n':
                // signed up a new user
                *mode = TC_MSG_SUP;
                break;
            case 'l':
                // connect to the server
                *mode = TC_MSG_SIN;
                break;
            case 'u':
                len = strlen(optarg);
                if(len >= MAX_NAME_LENGTH) {
                    fprintf(stderr, "user name must be less than %d characters\n", MAX_NAME_LENGTH);
                    exit(EXIT_FAILURE);
                }
                strncpy(pinfo->ui_name, optarg, len);
                break;
            case 'p':
                len = strlen(optarg);
                if(len >= MAX_PASSWD_LENGTH) {
                    fprintf(stderr, "user's password length must be less than %d characters\n", MAX_PASSWD_LENGTH);
                    exit(EXIT_FAILURE);
                }
                strncpy(pinfo->ui_passwd, optarg, len);
                break;
            case 'h':
                print_termchat_usage();
                break;
            case '?':
                print_termchat_usage();
                break;
        }
    } while(c != -1);
}
