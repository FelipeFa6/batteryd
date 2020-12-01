#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <machine/apmvar.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

/*
 * Macros.
 */

static bool daemonize = true;

#define dbg(__fmt, ...) \
  daemonize ?: fprintf(stderr, "batteryd: " __fmt "\n", __VA_ARGS__)

/*
 * Signal handler.
 */

static bool keep_running = true;

static void
handler(int sig)
{
  keep_running = false;
}

/*
 * Get configuration path.
 */

#define GLOBAL_CONFIG "/etc/batteryd"
#define LOCAL_CONFIG "/.config/batteryd"

const char*
config_path()
{
  /*
   * Return a global path when running as root.
   */
  if (getuid() == 1) {
    return strdup(GLOBAL_CONFIG);
  }
  /*
   * Get the calling's user directory.
   */
  char* home = getenv("HOME");
  if (home == NULL) {
    err(__LINE__, "%s", "HOME variable not defined");
  }
  /*
   * Build the local path.
   */
  const size_t buflen = strlen(home) + strlen(LOCAL_CONFIG) + 1;
  char* buffer = alloca(buflen);
  memset(buffer, 0, buflen);
  strcat(buffer, home);
  strcat(buffer, LOCAL_CONFIG);
  return strdup(buffer);
}

/*
 * State handlers.
 */

static void
on_change(const char* cp, const char* target)
{
  size_t buflen = strlen(cp) + strlen(target) + 2;
  char* fp = alloca(buflen);
  dbg("state is %s", target);
  /*
   * Create the full path.
   */
  memset(fp, 0, buflen);
  strcat(fp, cp);
  strcat(fp, "/");
  strcat(fp, target);
  /*
   * Execute the script.
   */
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

/*
 * State processor.
 */

static void
process(const char* cp, struct apm_power_info* pinfo)
{
  static u_char state = APM_BATT_UNKNOWN;
  /*
   * Check if the state has changed.
   */
  if (state == pinfo->battery_state) {
    return;
  }
  state = pinfo->battery_state;
  /*
   * Process the new state.
   */
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
    default: {
      break;
    }
  }
}

/*
 * Argument parsing.
 */

void
parse_options(int argc, char** argv)
{
  int ch;
  while ((ch = getopt(argc, argv, "d")) != -1) {
    switch (ch) {
      case 'd':
        daemonize = false;
        break;
      default:
        fprintf(stderr, "usage: batteryd [-d]\n");
        exit(__LINE__);
    }
  }
}

/*
 * Main loop.
 */

int
main(int argc, char** argv)
{
  /*
   * Parse the options.
   */
  parse_options(argc, argv);
  /*
   * Check if we should daemonize.
   */
  if (daemonize) {
    int nochroot = getuid() != 1;
    setsid();
    daemon(nochroot, 0 /* noclose */);
  }
  /*
   * Grab the config path.
   */
  const char* cp = config_path();
  dbg("%s", cp);
  /*
   * Open the APM device.
   */
  int fd = open("/dev/apm", O_RDONLY);
  if (fd < 0) {
    err(__LINE__, "open");
  }
  /*
   * Register ^C.
   */
  signal(SIGINT, handler);
  /*
   * Polling loop.
   */
  while (keep_running) {
    struct apm_power_info pinfo;
    if (ioctl(fd, APM_IOC_GETPOWER, &pinfo) < 0) {
      err(__LINE__, "ioctl");
    }
    process(cp, &pinfo);
    sleep(1);
  }
  /*
   * Clean-up and return.
   */
  close(fd);
  free((void*)cp);
  return 0;
}
