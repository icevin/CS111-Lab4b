// NAME: Kevin Tang
// EMAIL: kevintang2023@ucla.edu
// ID: 805419480

#include <getopt.h>
#include <math.h>
// #include <mraa.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int opt_period = 1;
int opt_scale  = 0;  // 0 = F, 1 = C
int opt_log    = 0;
int opt_file = 0;
int opt_debug = 0;




int main(int argc, char** argv) {

    // Process args
    static const struct option options[] =
        {
            {"period", required_argument, NULL, 'p'},
            {"scale", required_argument, NULL, 's'},
            {"log", required_argument, NULL, 'l'},
            {"debug", no_argument, NULL, 'd'},
            {0, 0, 0, 0}};

    for (;;) {
        int opt_index = 0;
        char c        = getopt_long(argc, argv, "", options, &opt_index);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 'p':
                opt_period = atoi(optarg);
                break;
            case 's':
                if (strcmp(optarg, "F") == 0)
                    opt_scale = 0;
                else if (strcmp(optarg, "S") == 0)
                    opt_scale = 1;
                else
                    fprintf(stderr, "Try \"lab4b [--period=<seconds>] [--log=<log_filename>] [--scale=F/C] [--debug]\"\n\n");

                break;
            case 'l':
                opt_log = 1;
                opt_file = open(optarg, O_CREAT | O_RDWR | O_TRUNC | O_APPEND, 0666);
                if (opt_file < 0) {
                    fprintf(stderr, "Error attempting to create/open log file. errno %d: %s\r\n", errno, strerror(errno));
                    exit(1);
                }
                break;

            case 'd':
                opt_debug = 1;
                break;
            default:
                fprintf(stderr, "Try \"lab4b [--period=<seconds>] [--log=<log_filename>] [--scale=F/C] [--debug]\"\n\n");
                exit(1);
                break;
        }
    }



    if(opt_log) {
        close(opt_file);
    }
}