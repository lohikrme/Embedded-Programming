# project link to wokwi:
# https://wokwi.com/projects/444049405338768385

# Download the lcd and pico_12c libraries from:
# https://www.instructables.com/RPI-Pico-I2C-LCD-Control/
# https://github.com/T-622/RPI-PICO-I2C-LCD

# other useful information:
# https://docs.micropython.org/en/latest/library/machine.I2C.html
# https://docs.wokwi.com/parts/wokwi-lcd1602

# components:
# rasberry pi pico wifi
# LCD 16x2 (I2C)

import time
import machine
# pico_i2c_lcd.py is the file name, while I2cLcd is the class name
from pico_i2c_lcd import I2cLcd


time.sleep(0.1) # Wait for USB to become ready

# params: id of monitor, scl pin, data pin, freq (recommendation 100k hz for lcd monitor)
sda=machine.Pin(0)
scl=machine.Pin(1)
i2c=machine.I2C(0,sda=sda, scl=scl, freq=100000)
print(i2c.scan()) # prints 39, transform to hexadecimal = 2*16+7 = 0x27

# params (can be seen in T-622 repo's test file)
# params: name of i2c connection, i2c memory address, rows, columns
lcd = I2cLcd(i2c, 0x27, 2, 16)    

while True:
  lcd.putstr("It Works!")
  time.sleep(5)
  lcd.clear()
  lcd.putstr("It doesn't work!")
  time.sleep(5)
  lcd.clear()


print("Hello, Pi Pico W!")
