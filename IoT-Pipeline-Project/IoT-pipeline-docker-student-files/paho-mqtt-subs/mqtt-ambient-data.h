// mqtt-ambient-data.h

// prevent double including same header "if not defined"
#ifndef MQTT_AMBIENT_H
#define MQTT_AMBIENT_H

#include <time.h>

#define MQTT_AMBIENT_JSON_MAX_LEN 1024

// ambient data struct, pad is a pointer towards struct like this
// AmbientData_t should contain minimumsame fields as publisher's
// subscriber uses AmbientData_t to parse coming messages
typedef struct ambient_data {
    char    *address;
    char    *location;
    char    *device;
    float   temperature;
    float   humidity;
    float   pressure;
    float   co2;
    time_t  mtime;
} AmbientData_t;

#endif