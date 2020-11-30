// NAME: Kevin Tang
// EMAIL: kevintang2023@ucla.edu
// ID: 805419480

#include <getopt.h>
#include <math.h>

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

#ifndef PC
#include <mraa.h>
#include <mraa/aio.h>
mraa_aio_context grove_sensor;
mraa_gpio_context grove_button;
#endif

// argument options
int opt_period = 1;
int opt_scale  = 0;  // 0 = F, 1 = C
int opt_log    = 0;
FILE *opt_file   = 0;
int opt_debug  = 0;

// stdin Options
int opt_report = 1;

#define INPUT_SIZE 1023

// Some help from https://www.tutorialspoint.com/c_standard_library/c_function_localtime.htm
struct tm * time_info;
struct timeval time_now;
time_t last_report;
time_t current;

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
    log_and_print(temp);
    if (opt_log)
        fclose(opt_file);

    #ifndef PC
    mraa_aio_close(grove_sensor);
    mraa_gpio_close(grove_button);
    #endif
    exit(0);
}

double parse_reading(int reading) {
    // https://wiki.seeedstudio.com/Grove-Temperature_Sensor_V1.2/
    const double B = 4275.;
    const double R0 = 100000.;
    double t = 1023.0 / (double) reading - 1.0;
    t = R0 * t;
    double log_result = log(t / R0);
    double temperature = 1.0/(log_result/B+1/298.15)-273.15; // Convert to temperature via datasheet
    
    if (opt_scale == 0) // F
    {
        return (temperature * 9. / 5.) + 32;
    } else {
        return temperature;
    }
    
}

// option_string is a \0 terminated string w/ one of the options
void parse_option(char* option_string) {
    if (opt_log != 0) {
        fprintf(opt_file, "%s\n", option_string);
        fflush(opt_file);
    }
    if (strcmp(option_string, "SCALE=F") == 0) {
        opt_scale = 0;
    } else if (strcmp(option_string, "SCALE=C") == 0) {
        opt_scale = 1;
    } else if (strcmp(option_string, "STOP") == 0) {
        opt_report = 0;
    } else if (strcmp(option_string, "START") == 0) {
        opt_report = 1;
    } else if (strcmp(option_string, "OFF") == 0) {
        button_shutdown();
    } else {
        char* found_period = strstr(option_string, "PERIOD=");
        if (found_period != NULL)
        {
            // move found_period to beginning of arg
            // technically not needed for atoi(), but why not
            found_period += 7;
            opt_period = atoi(found_period);
            return;
        }
        // We don't technically need to look for LOG <message> since all non-valid commands are automatically outputted anyways
        // We just find it and do nothing
        char* found_log = strstr(option_string, "LOG ");
        if (found_log != NULL)
        {
            // move found_log to beginning of arg
            found_period += 4;
        }
    }
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
                } else if (strcmp(optarg, "C") == 0) {
                    opt_scale = 1;
                } else {
                    fprintf(stderr, "Invalid scale argument, try \"lab4b [--period=<seconds>] [--log=<log_filename>] [--scale=F/C] [--debug]\"\n\n");
                    exit(1);
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
                fprintf(stderr, "Try \"lab4b [--period=<seconds>] [--log=<log_filename>] [--scale=F/C] [--debug]\"\n\n");
                exit(1);
                break;
        }
    }

    #ifndef PC
    grove_sensor = mraa_aio_init(1);
    grove_button = mraa_gpio_init(60);
    
    if (grove_sensor == NULL || grove_button == NULL)
    {
        fprintf(stderr, "Error: failed to initialize sensors. errno %d: %s\r\n", errno, strerror(errno));
        exit(1);
    }
    
    mraa_gpio_dir(grove_button, MRAA_GPIO_IN);
    mraa_gpio_isr(grove_button, MRAA_GPIO_EDGE_RISING, &button_shutdown, NULL);
    #endif

    char buffer[INPUT_SIZE + 1];

    struct pollfd poll_fd;
    poll_fd.fd     = STDIN_FILENO;
    poll_fd.events = POLLIN | POLLERR;

    int poll_rc = 0;
    int read_rc = 0;

    time(&last_report);
    time(&current);

    for (;;) {
        time(&current);
        gettimeofday(&time_now, 0);

        if (opt_report == 1 && difftime(current, last_report) >= (double) opt_period) {
            time_info = localtime(&time_now.tv_sec);
            #ifndef PC
            int sensor_reading = mraa_aio_read(grove_sensor);
            #else
            int sensor_reading = 100;
            #endif
            float temperature = parse_reading(sensor_reading);
            char temp[128];
            sprintf(temp, "%02d:%02d:%02d %.1f", time_info->tm_hour, time_info->tm_min, time_info->tm_sec, temperature);
            log_and_print(temp);
            time(&last_report);
        }

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
            
            // lex input
            buffer[read_rc] = '\0';

            char* head = &buffer[0];

            // Ignore whitespace
            while(*head != '\0') {
                while(*head == ' ' || *head == '\t')
                    head++;

                char* tail = strstr(head, "\n");
                
                int size = tail - head;
                char* arg_buffer = (char*) malloc((size + 1) * sizeof(char));
                strncpy(arg_buffer, head, size);
                arg_buffer[size] = '\0';
                parse_option(arg_buffer);
                free(arg_buffer);
                head = tail + 1;
            }

            

        }

    }

    if (opt_log) {
        fclose(opt_file);
    }

    #ifndef PC
    mraa_aio_close(grove_sensor);
    mraa_gpio_close(grove_button);
    #endif
}