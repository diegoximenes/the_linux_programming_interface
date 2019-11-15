#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <sys/time.h>

#define PROGRAM_NAME "alarm"

void usage(int status) {
    printf("Usage: %s SECONDS\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

void mysetitimer(int which,
                 const struct itimerval *value,
                 struct itimerval *ovalue) {
    if (setitimer(which, value, ovalue) == -1) {
        error("setitimer");
    }
}

unsigned myalarm(unsigned seconds) {
    struct itimerval new_timer_val, old_timer_val;

    new_timer_val.it_interval.tv_sec = 0;
    new_timer_val.it_interval.tv_usec = 0;
    new_timer_val.it_value.tv_sec = seconds;
    new_timer_val.it_value.tv_usec = 0;

    mysetitimer(ITIMER_REAL, &new_timer_val, &old_timer_val);

    return (old_timer_val.it_value.tv_sec +
            old_timer_val.it_value.tv_usec / 1000000);
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    // program variables
    int alarm_seconds = 0;

    // parse argv
    enum {
        GETOPT_HELP_CHAR = (CHAR_MIN - 2),
    };
    static struct option const long_options[] = {
        {"help", no_argument, NULL, GETOPT_HELP_CHAR},
    };
    while ((opt = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
        switch (opt) {
            case GETOPT_HELP_CHAR:
                usage(EXIT_SUCCESS);
                break;
            default:
                usage(EXIT_FAILURE);
                break;
        }
    }
    if (optind == argc - 1) {
        alarm_seconds = atoi(argv[optind++]);
    } else {
        usage(EXIT_FAILURE);
    }

    myalarm(alarm_seconds);

    while (1);

    exit(EXIT_SUCCESS);
}
