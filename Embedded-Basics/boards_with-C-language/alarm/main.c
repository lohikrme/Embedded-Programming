// main.c
// 25th november 2025

// https://wokwi.com/projects/448526176948842497

// libraries & dependencies
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// GPIO pins
#define SYSTEM_OK_LED GPIO_NUM_40
#define SEVERE_WARNING_LED  GPIO_NUM_42
#define MODERATE_WARNING_LED GPIO_NUM_38
#define BUZZER GPIO_NUM_45
#define SEVERE_WARNING_BUTTON GPIO_NUM_3
#define SEVERE_CLEAR_BUTTON GPIO_NUM_4
#define MODERATE_WARNING_BUTTON GPIO_NUM_5
#define MODERATE_CLEAR_BUTTON GPIO_NUM_6

// global variables, use volatile so pc wont optimize these variables
// volatile means it prepares for hardware to change it randomly
volatile int severe_warning_is_on = 0;
volatile int moderate_warning_is_on = 0;

// button behaviour: turn_on or clear alert
typedef enum { 
  BTN_ACTION_SET = 0,       // turn alert on
  BTN_ACTION_CLEAR = 1      // turn alert off
} btn_action_t;

typedef struct {
    gpio_num_t button_gpio;     // button pin gpio, e.g "SEVERE_WARNING_BUTTON"
    volatile int *warning_flag; // points to global warning variables
    btn_action_t action;        // BTN_ACTION_SET or BTN_ACTION_CLEAR
    const char *name;           // metainfo name of button, e.g print name of button clicked
} button_task_parameters_t;


//---------------------------------------------------
// BUTTON TASK HANDLES BOTH SETTING AND CLEARING BOTH WARNINGS
static void button_task(void *arg) {

  // STEP 1: EXTRACT PARAMETERS AND INITIATE VARIABLES

  // 1a) Cast pvParameters delivered by x_create_task, and if empty, return
  button_task_parameters_t *p = (button_task_parameters_t *)arg;
  if (p == NULL) {
    vTaskDelete(NULL);
    return;
  }
  // 1b) Save params into variables, use arrow to access stuff inside p
  gpio_num_t button = p->button_gpio;
  volatile int *warning_flag = p->warning_flag;
  btn_action_t action = p->action;
  const char *name = p->name;

  // 1c) initiate basic variables, because pullup mode, starts with "high voltage"
  int last_state = 1;

  // STEP 2: USE STATE TO DETERMINE IF BUTTON HAS BEEN PRESSED AND ACT BASED ON PARAMS
  while(1) {
    // state 0 = pressed, 1 = not pressed
    int state = gpio_get_level(button); 
    if (state == 0 && last_state == 1) {
      // debounce
      vTaskDelay(pdMS_TO_TICKS(30));
      if (gpio_get_level(button) == 0) {
        // act based on params

        // if button is SET mode, make its warning flag = 1
        if (action == BTN_ACTION_SET) {
          *warning_flag = 1;
        }
        // else if button is CLEAR mode, make its warning flag = 0
        else if (action == BTN_ACTION_CLEAR) {
          *warning_flag = 0;
        }

        // prevent multiple registration
        vTaskDelay(pdMS_TO_TICKS(300));
      }
    }
    last_state = state;
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}


//------------------------------------------------------
// SEVERE WARNING TASK - RED LED BLINKS AND WARNING SOUND
// ps. ESP_IDF boards apparently dont play sound well in wokwi
static void severe_warning_task(void *arg) {
    while (1) {
      // WHEN SEVERE_WARNING IS ON, TURN ON LED AND SOUND
      if (severe_warning_is_on) {
        gpio_set_level(SEVERE_WARNING_LED, 1);
        gpio_set_level(BUZZER, 1);
        // use pdMS_TO_TICKS through the project
        // it calculates how many freertos os ticks for x milliseconds
        vTaskDelay(pdMS_TO_TICKS(400)); 
        gpio_set_level(SEVERE_WARNING_LED, 0);
        gpio_set_level(BUZZER, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(SEVERE_WARNING_LED, 1);
        gpio_set_level(BUZZER, 1);
        vTaskDelay(pdMS_TO_TICKS(400));
        gpio_set_level(SEVERE_WARNING_LED, 0);
        gpio_set_level(BUZZER, 0);
        // Pause before next double-blink
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    // WHEN SEVERE_WARNING IS OFF, TURN OFF LED AND BUZZER AND WAIT
    else {
      gpio_set_level(SEVERE_WARNING_LED, 0);
      gpio_set_level(BUZZER, 0);
      vTaskDelay(pdMS_TO_TICKS(100));
    }
    }
}

// MODERATE WARNING TASK - BLUE LED BLINKS
static void moderate_warning_task(void *arg) {
  while (1) {
    if (moderate_warning_is_on) {
      gpio_set_level(MODERATE_WARNING_LED, 1);
      vTaskDelay(pdMS_TO_TICKS(800));
      gpio_set_level(MODERATE_WARNING_LED, 0);
      vTaskDelay(pdMS_TO_TICKS(1200));
    }
    else {
      gpio_set_level(MODERATE_WARNING_LED, 0);
      vTaskDelay(pdMS_TO_TICKS(500));
    }
  }
}

// SYSTEM OK SIGNAL - GREEN LED SLOWLY BLINKS
static void system_ok_signal(void *arg) {
  while (1) {
    // everything is okey, blink green led
    if (!severe_warning_is_on && !moderate_warning_is_on) {
        gpio_set_level(SYSTEM_OK_LED, 1);
        vTaskDelay(pdMS_TO_TICKS(1500));
        gpio_set_level(SYSTEM_OK_LED, 0);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
    // alarms replace green led
    else {
        gpio_set_level(SYSTEM_OK_LED, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
  }
}



//---------------------------------------------------
// MAIN FUNCTION
// handles all xTaskCreates
void app_main(void) {
    printf("ORIGINAL SECURITY ALARM BEGINS!...\n");

    // STEP 1: INITIATE ALL GPIO PINS (DIRECTION, MODE)

    // 1a) severe warning led
    // gpio_mode_output means board push voltage into led
    gpio_set_direction(SEVERE_WARNING_LED, GPIO_MODE_OUTPUT);
    
    // 1b) moderate warning led
    gpio_set_direction(MODERATE_WARNING_LED, GPIO_MODE_OUTPUT);

    // 1c) system ok led
    gpio_set_direction(SYSTEM_OK_LED, GPIO_MODE_OUTPUT);

    // 1d) buzzer
    gpio_set_direction(BUZZER, GPIO_MODE_OUTPUT);

    // 1e) severe warning turn on button
    gpio_set_direction(SEVERE_WARNING_BUTTON, GPIO_MODE_INPUT);
    // gpio_pullup_only prevents random data inputs by e.g distortions
    gpio_set_pull_mode(SEVERE_WARNING_BUTTON, GPIO_PULLUP_ONLY);

    // 1f) severe clear button
    gpio_set_direction(SEVERE_CLEAR_BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(SEVERE_CLEAR_BUTTON, GPIO_PULLUP_ONLY);

    // 1g) moderate warning turn on button
    gpio_set_direction(MODERATE_WARNING_BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(MODERATE_WARNING_BUTTON, GPIO_PULLUP_ONLY);

    // 1h) moderate clear button
    gpio_set_direction(MODERATE_CLEAR_BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(MODERATE_CLEAR_BUTTON, GPIO_PULLUP_ONLY);

    // STEP 2: BUTTON PARAMETERS
    // parametrized button_task parameteres
    // use static, so they wont "stale" with time
    // other option would be malloc to have an explicit reference to params
    static button_task_parameters_t severe_on  = { 
      .button_gpio = SEVERE_WARNING_BUTTON, 
      .warning_flag = &severe_warning_is_on, 
      .action = BTN_ACTION_SET, 
      .name = "severe_on" 
    };
    static button_task_parameters_t severe_off = { 
      .button_gpio = SEVERE_CLEAR_BUTTON, 
      .warning_flag = &severe_warning_is_on, 
      .action = BTN_ACTION_CLEAR, 
      .name = "severe_off" 
    };
    static button_task_parameters_t moderate_on = { 
      .button_gpio = MODERATE_WARNING_BUTTON, 
      .warning_flag = &moderate_warning_is_on, 
      .action = BTN_ACTION_SET, 
      .name = "moderate_on" 
    };
    static button_task_parameters_t moderate_off = { 
      .button_gpio = MODERATE_CLEAR_BUTTON, 
      .warning_flag = &moderate_warning_is_on, 
      .action = BTN_ACTION_CLEAR, 
      .name = "moderate_off" 
    };


    // STEP 3: CREATE X_TASKS WITH FreeRTOS
    // https://www.freertos.org/Documentation/02-Kernel/04-API-references/01-Task-creation/01-xTaskCreate

    // 3a) system ok task
    xTaskCreate(
      system_ok_signal,         // function that is turned on
      "system_ok_signal",       // name of task
      2048,                     // stack size
      NULL,                     // parameters not needed
      1,                        // priority where more is higher
      NULL                      // Out pointer to task handle
    ); 

    // 3b) warning tasks
    xTaskCreate(
      severe_warning_task,      // function that is turned on
      "severe_warning_task",    // name of task
      2048,                     // stack size
      NULL,                     // parameters not needed
      7,                        // priority where more is higher
      NULL                      // Out pointer to task handle
    ); 

    xTaskCreate(
      moderate_warning_task,    // function that is turned on
      "moderate_warning_task",  // name of task
      2048,                     // stack size
      NULL,                     // parameters not needed
      4,                        // priority where more is higher
      NULL                      // Out pointer to task handle
    );

    // 3c) button tasks (notice that one button task with different parameters handles all)
    // buttons are moderate to high priority, because we do not want to miss the warning trigger
    // supposedly real system would measure with sensor and send warning

    // severe warning on red button
    xTaskCreate(
      button_task,                  // function that is turned on
      "severe_warning_set_task",    // name of task
      2048,                         // stack size
      &severe_on,                   // reference to severe_on params
      8,                            // priority where more is higher
      NULL                          // Out pointer to task handle
    ); 

    // severe warning off grey button
    xTaskCreate(
      button_task,                  // function that is turned on
      "severe_warning_clear_task",  // name of task
      2048,                         // stack size
      &severe_off,                  // reference to severe_off params
      6,                            // priority where more is higher
      NULL                          // Out pointer to task handle
    ); 

    // severe warning on red button
    xTaskCreate(
      button_task,                  // function that is turned on
      "moderate_warning_set_task",  // name of task
      2048,                         // stack size
      &moderate_on,                 // reference to moderate_on params
      5,                            // priority where more is higher
      NULL                          // Out pointer to task handle
    ); 

    // severe warning off grey button
    xTaskCreate(
      button_task,                    // function that is turned on
      "moderate_warning_clear_task",  // name of task
      2048,                           // stack size
      &moderate_off,                  // reference to moderate_on params
      4,                              // priority where more is higher
      NULL                            // Out pointer to task handle
    ); 

    
}
