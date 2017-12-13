#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#define MAX_NAME_LENGTH 32
#define MAX_PASSWD_LENGTH 32

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

const char *short_options = "nl:u:ph";

void client_command_parse(int args, char **pptrarg)
{
    int option_index = 0;
    int c;

    c = getopt_long(args, pptrarg, short_options, long_options, &option_index);
    if(c < 0) {
        perror("getopt_long");
        exit(EXIT_FAILURE);
    }
    switch(c) {
        case 'n':
            // signed up a new user

            break;
        case 'l':
            // connect to the server

            break;
        case 'u':

            break;
        case 'p':

            break;
        case 'h':
        default:
            //print useage
            break;
    }

}
