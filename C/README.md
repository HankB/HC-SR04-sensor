# measure_distance

## References

[Sparkfin data sheet](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf)  
[ITdat Studio data sheet](https://www.electroschematics.com/wp-content/uploads/2013/07/HC-SR04-datasheet-version-2.pdf)  

## Motivation

Examples I found of code that uses the HC-SR04 sensor all busy-wait for the return signal, This introduces two issues.

* High CPU usage while waiting. This is probably not an issue with this application where the time between the trigger and echo signals is a fraction of a second, but for other applications where there is a need to wait for a longer interval for a SPIO inpuit to change, this could cause problems.
* Jitter in the measured result, particularly with single threadded cores such as on the Pi Zero where some other processing can delay recognition of the input change.

The way to avoid these is to use event handling built into the GPIOD driver <https://libgpiod.readthedocs.io/en/latest/group__line__event.html>

## Overview

1. Configure GPIO 8 as output (trigger) and GPIO 11 to monitor input changes (echo).
1. Drive the trigger high for 10 usec.
1. Monitor echo and record time when the input goes high and when it gots low.
1. Calculate distance according to the width of the echo pulse.

