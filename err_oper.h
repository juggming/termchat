#include <stdarg.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#define MAXLINE_SIZE (int)sysconf(_SC_LINE_MAX)
/*
 *  function:       sterror(errno)?     exit_handler        syslog_class
 *  err_dump()      Y                   abort()             LOG_ERR
 *  err_msg()       N                   return              LOG_INFO
 *  err_quit()      N                   exit(1)             LOG_ERR
 *  err_ret()       Y                   return              LOG_INFO
 *  err_sys         Y                   exit(1)             LOG_ERR
 */

int daemon_proc;
static void err_doit(int, int, const char *, va_list );

int get_maxline_size(void)
{
    return (int)sysconf(_SC_LINE_MAX);
}


void err_ret(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, LOG_INFO, fmt, ap);
    va_end(ap);
    return;
}

void err_sys(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, LOG_ERR, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

void err_dump(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, LOG_ERR, fmt, ap);
    va_end(ap);
    abort();
    exit(EXIT_FAILURE);
}

void err_msg(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(0, LOG_INFO, fmt, ap);
    va_end(ap);
    return;
}

void err_quit(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(0, LOG_ERR, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

static void err_doit(int errnoflag, int level, const char *fmt, va_list ap)
{
    int     errno_save, n;
    char    errbuf[MAXLINE_SIZE];

    errno_save = errno;
#ifdef HAVE_VSNPRINTF
    vsnprintf(errbuf, MAXLINE_SIZE, fmt, ap);   /* safe */
#else
    vsprintf(errbuf, fmt, ap);
#endif
    n = strlen(errbuf);
    if(errnoflag)
        snprintf(errbuf + n, MAXLINE_SIZE - n , ": %s", strerror(errno_save));
    strcat(errbuf, "\n");

    if(daemon_proc) {
        syslog(level, errbuf);
    } else {
        fflush(stdout);
        fputs(errbuf, stderr);
        fflush(stderr);
    }
    return;
}
