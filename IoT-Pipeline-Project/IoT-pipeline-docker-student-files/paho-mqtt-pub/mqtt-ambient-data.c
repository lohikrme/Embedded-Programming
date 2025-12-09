// mqtt-ambient-data.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mqtt-ambient-data.h"

#define TEMP_RANGE              15
#define TEMP_RANGE_START        20
#define TEMP_FILTER_COFF        6.0
#define HUMIDITY_RANGE          30
#define HUMIDITY_RANGE_START    70
#define HUMIDITY_FILTER_COFF    6.0
#define PRESSURE_RANGE          100
#define PRESSURE_RANGE_START    900
#define PRESSURE_FILTER_COFF    6.0
#define CO2_RANGE               600
#define CO2_RANGE_START         400
#define CO2_FILTER_COFF         6.0

// for filtering virtual measurement values store last measurement value
// then every time u generate a new value, it can be combination of old and new
static float temperature_nm1 = TEMP_RANGE_START;
static float humidity_nm1 = HUMIDITY_RANGE_START;
static float pressure_nm1 = PRESSURE_RANGE_START;
static float co2_nm1 = CO2_RANGE_START;

// functions
// Random between 0..1
float Rand() {
    return (float)rand()/(float)RAND_MAX;
}

AmbientData_t *MeasureAmbientData(AmbientData_t *pad, char *l, char *d) {
    // char *d = DEVICE;
    // char *l = LOCATION;

    // create measurement data
    float t = Rand() * TEMP_RANGE + TEMP_RANGE_START;
    float h = Rand() * HUMIDITY_RANGE + HUMIDITY_RANGE_START;
    float p = Rand() * PRESSURE_RANGE + PRESSURE_RANGE_START;
    float co2 = Rand() * CO2_RANGE + CO2_RANGE_START;
    time_t mt;

    // printf("t: %f, h: %f, p: %f, c: %f\n", t, h, p, co2);

    // filter, e.g base value 25C, new 30C... (25 * 6 + 30) / (6+1)
    t   = (temperature_nm1 * TEMP_FILTER_COFF + t)/(TEMP_FILTER_COFF+1);
    h   = (humidity_nm1 * HUMIDITY_FILTER_COFF + h)/(HUMIDITY_FILTER_COFF+1);
    p   = (pressure_nm1 * PRESSURE_FILTER_COFF + p)/(PRESSURE_FILTER_COFF+1);
    co2 = (co2_nm1 * CO2_FILTER_COFF + co2)/(CO2_FILTER_COFF+1);

    // printf("t: %f, h: %f, p: %f, c: %f\n", t, h, p, co2);

    // update the old base value, new value becomes old base value
    temperature_nm1 = t;
    humidity_nm1 = h;
    pressure_nm1 = p;
    co2_nm1 = co2;
    mt = time(NULL);
    // pad is a pointer to ambient data struct
    return SetAmbientData(pad, l, d, t, h, p, co2, mt);
}

// fill the "ambientdata struct" variables with the new data, 
// pad is the pointer to ambient data struct
AmbientData_t *SetAmbientData(AmbientData_t *pad, char *l, char *d, float t, float h, float p, float c, time_t mt) {
    pad->location = l;
    pad->device = d;
    pad->temperature = t;
    pad->humidity = h;
    pad->pressure = p;
    pad->co2 = c;
    pad->mtime = mt;
    return pad;
} 

char *StringifyAmbientData(AmbientData_t *pad, char *buf) {
    printf("location: %s.   device: %s.   temperature: %f.    humidity: %f.    pressure: %f.    co2: %f.    mtime: %lu.  ", 
    pad->location,
    pad->device,
    pad->temperature,
    pad->humidity,
    pad->pressure,
    pad->co2,
    pad->mtime);
    sprintf(buf, "{\"location\": \"%s\", \"device\": \"%s\", \"temperature\": %f, \"humidity\": %f, \"pressure\": %f, \"co2\": %f, \"mtime\": %lu }",
    pad->location,
    pad->device,
    pad->temperature,
    pad->humidity,
    pad->pressure,
    pad->co2,
    pad->mtime);
    return buf;
}
