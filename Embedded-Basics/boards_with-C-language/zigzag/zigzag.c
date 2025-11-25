// zigzag.c
// the idea is it draws zigzag on 16x2 lcd monitor, with 'o' and ' '

// to find wokwi project to try out the application go to:
//https://wokwi.com/projects/448052402178417665

// components:
// LCD1602 and Pi Pico!

// library needed for the lcd monitor to show content
// documentation: https://docs.arduino.cc/libraries/liquidcrystal/
#include <LiquidCrystal.h>


// initialize the lcd monitor
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

// setup the components first in main
void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
}

// determine which row, which column, 
// also determine is the mode clean (' ') or not clean ('o') the monitor
int rowIndex = 0;
int columnIndex = 0;
int clean = 0;

// start an endless loop
void loop() {
  // wake up the loop with 1 ms delay
  delay(1); 

  // set cursor on lcd monitor, every loop will go
  // (0,0) (1,1) (2,0) (3,1)... (14,0) (15,1)... (0,0) (1,1) (2,0) (3,1)... (14,0) (15,1)...
  lcd.setCursor(columnIndex, rowIndex);

  // select letter to draw based on boolean 'clean' (0 or 1)
  if (clean) {
    lcd.print(" ");
  }
  else {
    lcd.print("o");
  }

  // swap row every time from first to second or from second to first row
  if (rowIndex == 0) {
    rowIndex = 1;
  }
  else {
    rowIndex = 0;
  }

  // columns are from 0 to 15... after 15 is reached, reset column index and swap clean mode
  if (columnIndex == 15) {
    clean = !clean;
    columnIndex = 0;
  }
  else {
    columnIndex ++;
  }

  // delay between drawing a letter is 0.5 seconds
  delay(500);

}