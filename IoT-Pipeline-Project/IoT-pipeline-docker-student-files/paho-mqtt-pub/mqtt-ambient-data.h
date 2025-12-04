// MQTT-ambient.h
// definitions about mqtt message

#ifndef MQTT_AMBIENT_H
#define MQTT_AMBIENT_H

#include <time.h>

#define MQTT_AMBIENT_JSON_MAX_LEN 1024

// TODO: change location and device to match other pipeline if needed
// find out if this mqtt message should be different than the one alrdy delivered
#define LOCATION "M19"
#define DEVICE "UFO-tech"

typedef struct ambient_data {
    char    *location;
    char    *device;
    float   temperature;
    float   humidity;
    float   pressure;
    float   co2;
    time_t  mtime;
} AmbientData_t;

extern AmbientData_t* MeasureAmbientData(AmbientData_t *pad, char *l, char *d);
extern AmbientData_t* SetAmbientData(AmbientData_t *pad, char *l, char *d, float t, float h, float p, float c, time_t mt);
extern char* StringifyAmbientData(AmbientData_t *pad, char *buf);

#endif