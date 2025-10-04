# task 1: make a led that turns on pressing a button

# notice that components need GROUNDING
# for electricity to travel through them
# there is data pins and ground pins used

# when button is pressed, PULL_UP resistor of its datapin
# will reduce the volt from 3.3 to 0 volts
# because wires connect and electricity goes from datapin to ground
# thats why when button is pressed, button.value() == 0

# https://wokwi.com/projects/443507567002714113

import time
from machine import Pin
time.sleep(0.1) # Wait for USB to become ready

led = Pin(16, Pin.OUT);
button = Pin(13, Pin.IN, Pin.PULL_UP)

while True:
    if button.value() == 0:
        led.value(1)
    else:
        led.value(0)

print("Hello, Pi Pico W!")

