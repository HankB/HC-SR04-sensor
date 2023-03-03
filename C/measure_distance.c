/*
* Explore various GPIO line event related APIs

Build
    gcc -o measure_distance -Wall measure_distance.c -l gpiod
*/

#include <stdio.h>
#include <gpiod.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>

static const char *consumer = "HC-SR04";

static void unwind(const char * tag,
                const char *err_msg,
                   struct gpiod_chip *chip,
                   struct gpiod_line *trigger,
                   struct gpiod_line *echo)
{
    fprintf(stderr, "%s:%s\n", tag, err_msg);
    if (echo != NULL)
        gpiod_line_release(echo);
    if (trigger != NULL)
        gpiod_line_release(trigger);
    if (chip != NULL)
        gpiod_chip_close(chip);
}

static float calc_distance(const struct timespec *start, const struct timespec *end)
{
    time_t      delta_ns;
    if(end->tv_sec == start->tv_sec )
        delta_ns = end->tv_nsec - start->tv_nsec;
    else
        delta_ns = end->tv_nsec - start->tv_nsec + 1000000000;

    return (float)delta_ns/(148*1000);

}
int main(int argc, char **argv)
{
    // need to open the chip first
    struct gpiod_chip *chip = gpiod_chip_open("/dev/gpiochip0");
    if (chip == NULL)
    {
        unwind("gpiod_chip_open()", strerror(errno), NULL, NULL, NULL);
        exit(-1);
    }

    // acquire & configure GPIO 11 (trigger) GPIO 8 (echo)
    struct gpiod_line *trigger;
    trigger = gpiod_chip_get_line(chip, 11);
    const struct gpiod_line_request_config write_config =
        {consumer,
         GPIOD_LINE_REQUEST_DIRECTION_OUTPUT,
         0};

    int rc = gpiod_line_request(trigger, &write_config, 0);
    if (rc < 0)
    {
        unwind("gpiod_line_request(trigger)", strerror(errno), chip, trigger, NULL);
        exit(-1);
    }

    // prepare to wait on event for GPIO 11 (echo)
    struct gpiod_line *echo;
    echo = gpiod_chip_get_line(chip, 8);
    if (echo < 0)
    {
        unwind("gpiod_chip_get_line(echo)", strerror(errno), chip, trigger, NULL);
        exit(-1);
    }

    rc = gpiod_line_request_both_edges_events(echo, consumer);
    if (rc < 0)
    {
        unwind("gpiod_line_request_both_edges_events(echo)", strerror(errno), chip, trigger, echo);
        exit(-1);
    }

    while (true)
    {
        const struct timespec echo_timeout = {5, 0};           // 5s, 0ns
        const struct timespec trigger_length = {0, 10 * 1000}; // 0s, 10 msec
        struct timespec trigger_length_rem = {0, 0};

        rc = gpiod_line_set_value(trigger, 1);
        if (rc < 0)
        {
            unwind("gpiod_line_set_value(trigger,1)", strerror(errno), chip, trigger, echo);
            exit(-1);
        }
        nanosleep(&trigger_length, &trigger_length_rem);
        rc = gpiod_line_set_value(trigger, 0);
        if (rc < 0)
        {
            unwind("gpiod_line_set_value(trigger,0)", strerror(errno), chip, trigger, echo);
            exit(-1);
        }

        // gpiod_line_event_wait() seems not required unless a timeout is needed
        rc = gpiod_line_event_wait(echo, &echo_timeout);
        if (rc < 0)
        {
            unwind("gpiod_line_event_wait(echo)", strerror(errno), chip, trigger, echo);
            exit(-1);
        }
        else if (rc == 0)
        {
            unwind("gpiod_line_event_wait(echo)", "timeout waiting for HC-SR04", chip, trigger, echo);
            exit(-1);
        }

        struct gpiod_line_event start_pulse;
        rc = gpiod_line_event_read(echo, &start_pulse);
        if (rc < 0)
        {
            unwind("gpiod_line_event_read(echo)/start", strerror(errno), chip, trigger, echo);
            exit(-1);
        }
        else
        {
            printf("start %d at %ld s, %ld ns\n", start_pulse.event_type, start_pulse.ts.tv_sec, start_pulse.ts.tv_nsec);
        }

        struct gpiod_line_event end_pulse;
        rc = gpiod_line_event_read(echo, &end_pulse);
        if (rc < 0)
        {
            unwind("gpiod_line_event_read(echo)/end", strerror(errno), chip, trigger, echo);
            exit(-1);
        }
        else
        {
            printf("end %d at %ld s, %ld ns\n", end_pulse.event_type, end_pulse.ts.tv_sec, end_pulse.ts.tv_nsec);
        }

        float dist = calc_distance(&start_pulse.ts, &end_pulse.ts);
        printf("distance: %f\n\n", dist);
        sleep(1); // Delay before next measurement
    }

    // cleanup
    gpiod_line_release(trigger);
    gpiod_line_release(echo);
    gpiod_chip_close(chip);
}
