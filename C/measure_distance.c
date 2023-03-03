/*
* Explore various GPIO line event related APIs

Build
    gcc -o measure_distance -Wall measure_distance.c -l gpiod
*/

#include <stdio.h>
#include <gpiod.h>
#include <unistd.h>
#include <time.h>

static const char *consumer = "HC-SR04";

int main(int argc, char **argv)
{
    // need to open the chip first
    struct gpiod_chip *chip = gpiod_chip_open("/dev/gpiochip0");
    if (chip == NULL)
    {
        perror("gpiod_chip_open()");
        return 1;
    }

    // acquire & configure GPIO 8 (trigger) GPIO 11 (echo)
    struct gpiod_line *trigger;
    trigger = gpiod_chip_get_line(chip, 8);
    const struct gpiod_line_request_config write_config =
        {consumer,
         GPIOD_LINE_REQUEST_DIRECTION_OUTPUT,
         0};

    int rc = gpiod_line_request(trigger,
                                &write_config,
                                0);
    if (rc < 0)
    {
        perror("gpiod_line_request(trigger)");
        gpiod_line_release(trigger);
        gpiod_chip_close(chip);
    }

    // prepare to wait on event for GPIO 11 (echo)
    struct gpiod_line *echo;
    echo = gpiod_chip_get_line(chip, 11);
    if (echo < 0)
    {
        perror("gpiod_chip_get_line(echo)");
        gpiod_line_release(trigger);
        gpiod_chip_close(chip);
        return -1;
    }

    rc = gpiod_line_request_both_edges_events(echo, consumer);
    if (rc < 0)
    {
        perror("gpiod_line_request_both_edges_events(echo)");
        gpiod_line_release(trigger);
        gpiod_line_release(echo);
        gpiod_chip_close(chip);
        return -1;
    }

    while (true)
    {
        const struct timespec echo_timeout = {5, 0}; // 5s, 0ns
        const struct timespec trigger_length = {0, 10*1000}; // 0s, 10 msec
        struct timespec trigger_length_rem = {0,0};

        rc = gpiod_line_set_value(trigger, 1);
        if (rc < 0)
        {
            perror("gpiod_line_set_value(trigger)");
            gpiod_line_release(trigger);
            gpiod_line_release(echo);
            gpiod_chip_close(chip);
        }        
        nanosleep(&trigger_length, &trigger_length_rem);
        rc = gpiod_line_set_value(trigger, 0);
        if (rc < 0)
        {
            perror("gpiod_line_set_value(trigger)");
            gpiod_line_release(trigger);
            gpiod_line_release(echo);
            gpiod_chip_close(chip);
        }        

        // gpiod_line_event_wait() seems not required unless a timeout is needed
        rc = gpiod_line_event_wait(echo, &echo_timeout);
        if (rc < 0)
        {
            perror("gpiod_line_event_wait(echo)");
            gpiod_line_release(trigger);
            gpiod_line_release(echo);
            gpiod_chip_close(chip);
            return -1;
        }
        else if (rc == 0)
        {
            fprintf(stderr, "timeout waiting for HC-SR04\n");
            gpiod_line_release(trigger);
            gpiod_line_release(echo);
            gpiod_chip_close(chip);
            return -1;
        }

        struct gpiod_line_event start_pulse;
        rc = gpiod_line_event_read(echo, &start_pulse);
        if (rc < 0)
        {
            perror("gpiod_line_event_read(echo, &event)");
            gpiod_line_release(trigger);
            gpiod_line_release(echo);
            gpiod_chip_close(chip);
            return -1;
        }
        else
        {
            printf("start %d at %ld s, %ld ns\n", start_pulse.event_type, start_pulse.ts.tv_sec, start_pulse.ts.tv_nsec);
        }

        struct gpiod_line_event end_pulse;
        rc = gpiod_line_event_read(echo, &end_pulse);
        if (rc < 0)
        {
            perror("gpiod_line_event_read(echo, &event)");
            gpiod_line_release(trigger);
            gpiod_line_release(echo);
            gpiod_chip_close(chip);
            return -1;
        }
        else
        {
            printf("end %d at %ld s, %ld ns\n", end_pulse.event_type, end_pulse.ts.tv_sec, end_pulse.ts.tv_nsec);
        }

        sleep(1); //Delay before next measurement
    }

    // cleanup
    gpiod_line_release(echo);
    gpiod_chip_close(chip);
    // GPIO 8 should return to default here (input w/ pullup)
}
