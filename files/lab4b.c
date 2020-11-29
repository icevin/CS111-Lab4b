// NAME: Kevin Tang
// EMAIL: kevintang2023@ucla.edu
// ID: 805419480

#include <getopt.h>
#include <math.h>
// #include <mraa.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int opt_period = 1;
int opt_scale  = 0;  // 0 = F, 1 = C
int opt_log    = 0;
FILE *opt_file   = 0;
int opt_debug  = 0;

int opt_report = 1;

#define INPUT_SIZE 1024

#ifdef BEAGLEBONE
mraa_aio_context grove_sensor;
mraa_gpio_context grove_button;
#endif

// Some help from https://www.tutorialspoint.com/c_standard_library/c_function_localtime.htm
struct tm * time_info;
struct timeval time_now;

void log_and_print(char* string) {
    if (opt_log != 0) {
        fprintf(opt_file, "%s\n", string);
        fflush(opt_file);
    }
    fprintf(stdout, "%s\n", string);
}

void button_shutdown() {
    char temp[128];
    time_info = localtime(&time_now.tv_sec);
    sprintf(temp, "%02d:%02d:%02d SHUTDOWN", time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
}

int main(int argc, char** argv) {
    // Process args
    static const struct option options[] =
        {
            {"period", required_argument, NULL, 1},
            {"scale", required_argument, NULL, 2},
            {"log", required_argument, NULL, 3},
            {"debug", no_argument, NULL, 4},
            {0, 0, 0, 0}};

    for (;;) {
        int opt_index = 0;
        int c        = getopt_long(argc, argv, "", options, &opt_index);

        if (c == -1)
            break;  

        switch (c) {
            case 1:
                opt_period = atoi(optarg);
                break;
            case 2:
                if (strcmp(optarg, "F") == 0) {
                    opt_scale = 0;
                } else if (strcmp(optarg, "S") == 0) {
                    opt_scale = 1;
                } else {
                    fprintf(stderr, "THIS ONE Try \"lab4b [--period=<seconds>] [--log=<log_filename>] [--scale=F/C] [--debug]\"\n\n");
                }
                break;
            case 3:
                opt_log  = 1;
                opt_file = fopen(optarg, "a+");
                if (opt_file == NULL) {
                    fprintf(stderr, "Error attempting to create/open log file. errno %d: %s\r\n", errno, strerror(errno));
                    exit(1);
                }
                break;

            case 4:
                opt_debug = 1;
                break;

            default:
                fprintf(stderr, "the case is: %d", c);
                fprintf(stderr, "Try \"lab4b [--period=<seconds>] [--log=<log_filename>] [--scale=F/C] [--debug]\"\n\n");
                exit(1);
                break;
        }
    }

    #ifdef BEAGLEBONE
    grove_sensor = mraa_aio_init(1);
    grove_button = mraa_gpio_init(60);
    mraa_gpio_dir(grove_button, MRAA_GPIO_IN);
    mraa_gpio_isr(grove_button, MRAA_GPIO_EDGE_RISING, &button_shutdown, NULL)
    #endif

    char buffer[INPUT_SIZE];

    struct pollfd poll_fd;
    poll_fd.fd     = STDIN_FILENO;
    poll_fd.events = POLLIN | POLLERR;

    int poll_rc = 0;
    int read_rc = 0;

    for (;;) {
        gettimeofday(&time_now, 0);

        poll_rc = poll(&poll_fd, 1, 0);
        if (poll_rc < 0) {
            fprintf(stderr, "Error polling. errno %d: %s\r\n", errno, strerror(errno));
            exit(1);
        }

        if (poll_fd.revents & POLLIN) {
            read_rc = read(poll_fd.fd, buffer, INPUT_SIZE);
            if (read_rc < 0) {
                fprintf(stderr, "Error reading. errno %d: %s\r\n", errno, strerror(errno));
                exit(1);
            }
        }

    }

    if (opt_log) {
        fclose(opt_file);
    }
}