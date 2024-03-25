/* See LICENSE file for copyright and license details. */

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <machine/apmvar.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include "config.h"

/* global variables */
bool verbose = false;

void debug(const char *fmt, ...) {
    if(!verbose) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "batteryd: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
}

const char* config_path() {

    char* home = getenv("HOME");
    const size_t buflen = strlen(home) + strlen(LOCAL_CONFIG) + 1;
    char* buffer = alloca(buflen);

    /* get the calling's user directory.  */
    if (home == NULL) {
        err(__LINE__, "%s", "HOME variable not defined");
    }

    /* build the local path.  */
    memset(buffer, 0, buflen);
    strlcpy(buffer, home, buflen);
    strlcat(buffer, LOCAL_CONFIG, buflen);
    return strdup(buffer);
}

/* state handlers.  */
static void on_change(const char* cp, const char* target) {
    size_t buflen = strlen(cp) + strlen(target) + 2;
    char* fp      = alloca(buflen);

    debug("state is %s\n", target);

    /* concat path.  */
    memset(fp, 0, buflen);
    strlcpy(fp, cp, buflen);
    strlcat(fp, "/", buflen);
    strlcat(fp, target, buflen);

    /* execute the script.  */
    switch (fork()) {
        case -1: {
                     err(__LINE__, "fork");
                 }
        case 0: {
                    char* argv[] = { fp, NULL };
                    setsid();
                    execv(fp, argv);
                    err(__LINE__, "%s", fp);
                }
        default: {
                     int dumb;
                     wait(&dumb);
                     break;
                 }
    }
}

/* state processor */
static void process(const char* cp, struct apm_power_info* pinfo) {
    static u_char state = APM_BATT_UNKNOWN;

    /* check if the state has changed.  */
    if (state == pinfo->battery_state) {
        return;
    }

    state = pinfo->battery_state;

    /* process the new state.  */
    switch (pinfo->battery_state) {
        case APM_BATT_CRITICAL: {
            on_change(cp, "critical");
            break;
        }
        case APM_BATT_LOW: {
            on_change(cp, "low");
            break;
        }
        case APM_BATT_HIGH: {
            on_change(cp, "high");
            break;
        }
        case APM_BATT_CHARGING: {
            on_change(cp, "charging");
            break;
        }
        default: {
            break;
        }
    }
}

void parse_options(int argc, char** argv) {
    int ch;
    while ((ch = getopt(argc, argv, "v")) != -1) {
        switch (ch) {
            case 'v':
                verbose = true;
                break;
            default:
                fprintf(stderr, "usage: batteryd [-v]\n");
                exit(__LINE__);
        }
    }
}

int main(int argc, char** argv) {

    parse_options(argc, argv);

    /* get config path. */
    const char* cp = config_path();
    debug("%s\n", cp);

    /* open apm */
    int fd = open("/dev/apm", O_RDONLY);
    if (fd < 0) {
        err(__LINE__, "open");
    }

    /* polling loop.  */
    while (1) {
        struct apm_power_info pinfo;
        if (ioctl(fd, APM_IOC_GETPOWER, &pinfo) < 0) {
            err(__LINE__, "ioctl");
        }
        process(cp, &pinfo);
        sleep(1);
    }

    close(fd);
    free((void*)cp);
    return 0;
}
