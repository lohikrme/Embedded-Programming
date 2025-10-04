# make traffic lights
# project link to wokwi: https://wokwi.com/projects/443876684733991937

import time
from machine import Pin
time.sleep(0.1) # W

led_red = Pin(1, Pin.OUT)
led_yellow = Pin(8, Pin.OUT)
led_green = Pin(13, Pin.OUT)


while True:
    led_red.value(1)
    time.sleep(5)
    led_red.value(0)
    led_yellow.value(1)
    time.sleep(5)
    led_yellow.value(0)
    led_green.value(1)
    time.sleep(5)
    led_green.value(0)
print("Hello, Pi Pico W!")