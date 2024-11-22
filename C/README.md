# measure_distance

## Status

* Current code reports time stamps and resulting distance.
* Work in this direction is proceeding in <https://github.com/HankB/event-driven-HC-SR04>
* In other words, "My work here is done." (But feel free to fork, report bugs and submit PRs.)

## References

[Sparkfin data sheet](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf)  
[ITdat Studio data sheet](https://www.electroschematics.com/wp-content/uploads/2013/07/HC-SR04-datasheet-version-2.pdf)  

## Motivation

Examples I found of code that uses the HC-SR04 sensor all busy-wait for the return signal, This introduces two issues.

* High CPU usage while waiting. This is probably not an issue with this application where the time between the trigger and echo signals is a fraction of a second, but for other applications where there is a need to wait for a longer interval for a SPIO inpuit to change, this could cause problems.
* Jitter in the measured result, particularly with single threaded cores such as on the Pi Zero where some other processing can delay recognition of the input change.

The way to avoid these is to use event handling built into the GPIOD driver <https://libgpiod.readthedocs.io/en/latest/group__line__event.html> (Note: It has been difficult to this doc between the version available in Debian and the latest release. Good luck.)

## Overview

1. Configure GPIO 11 as output (trigger) and GPIO 8 to monitor input changes (echo).
1. Drive the trigger high for 10 usec.
1. Monitor echo and record time when the input goes high and when it gots low.
1. Calculate distance according to the width of the echo pulse.

## TODO

See Status.

* Provide output analogous to the Python example along with statistics (mean, standard deviation) so results can be compared more directly.
* Fix problem where start and end events become unsynchronized.
* Flesh out the discussion a bit more.
