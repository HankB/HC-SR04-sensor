#!/usr/bin/python3
#              .';:cc;.
#            .,',;lol::c.
#            ;';lddddlclo
#            lcloxxoddodxdool:,.
#            cxdddxdodxdkOkkkkkkkd:.
#          .ldxkkOOOOkkOO000Okkxkkkkx:.
#        .lddxkkOkOOO0OOO0000Okxxxxkkkk:
#       'ooddkkkxxkO0000KK00Okxdoodxkkkko
#      .ooodxkkxxxOO000kkkO0KOxolooxkkxxkl
#      lolodxkkxxkOx,.      .lkdolodkkxxxO.
#      doloodxkkkOk           ....   .,cxO;
#      ddoodddxkkkk:         ,oxxxkOdc'..o'
#      :kdddxxxxd,  ,lolccldxxxkkOOOkkkko,
#       lOkxkkk;  :xkkkkkkkkOOO000OOkkOOk.
#        ;00Ok' 'O000OO0000000000OOOO0Od.
#         .l0l.;OOO000000OOOOOO000000x,
#            .'OKKKK00000000000000kc.
#               .:ox0KKKKKKK0kdc,.
#                      ...
#
# Author: peppe8o
# Date: May 1th, 2021
# Version: 1.0

# Import required libraries
import RPi.GPIO as GPIO
import time

# --------------------------------------------------------------------
# PINS MAPPING AND SETUP
# --------------------------------------------------------------------

echoPIN = 24
triggerPIN = 23

GPIO.setmode(GPIO.BOARD)
GPIO.setup(echoPIN, GPIO.IN)
GPIO.setup(triggerPIN, GPIO.OUT)

# --------------------------------------------------------------------
# MAIN FUNCTIONS
# --------------------------------------------------------------------


def distance():
    new_reading = False
    counter = 0
    distance = 0
    duration = 0

    # send trigger
    GPIO.output(triggerPIN, 0)
    time.sleep(0.000002)
    GPIO.output(triggerPIN, 1)
    time.sleep(0.000010)
    GPIO.output(triggerPIN, 0)
    time.sleep(0.000002)

    # wait for echo reading
    while GPIO.input(echoPIN) == 0:
        pass
        counter += 1
        if counter == 5000:
            new_reading = True
            break

    if new_reading:
        return False
    startT = time.time()

    while GPIO.input(echoPIN) == 1:
        pass
    feedbackT = time.time()

    # calculating distance
    if feedbackT == startT:
        distance = "N/A"
    else:
        duration = feedbackT - startT
        soundSpeed = 34300  # cm/s
        distance = duration * soundSpeed / 2
        # distance = round(distance, 1)
    return distance

# --------------------------------------------------------------------
# MAIN LOOP
# --------------------------------------------------------------------

# Collect 5 readings and write out.


reading_count = 5

try:
    while True:
        readings = []
        for i in range(reading_count):
            readings.append(distance())
            time.sleep(0.1)
        print(readings)
except KeyboardInterrupt:
    GPIO.cleanup()
