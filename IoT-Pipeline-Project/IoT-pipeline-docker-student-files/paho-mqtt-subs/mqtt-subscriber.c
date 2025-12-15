// mqtt-subscriber.c

// compile: gcc -o mqtt-subscriber mqtt-subscriber.c -I/usr/local/include -L/usr/local/lib -lpaho-mqtt3as -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <MQTTAsync.h>
#include "subscriber-config.h"

// global data
static volatile MQTTAsync_token delivered_token;
static int finished = 0;

// functions
static const char *GetMQTTStateName(mqtt_state_t state) {
    static const char *mqtt_state_names[] = {
            "STARTING", 
            "CLIENT_CREATED", 
            "CALLBACKS_SET", 
            "CONNECTION_REQUESTED",
            "CONNECTED",
            "SUBSCRIBE_REQUESTED",
            "SUBSCRIBED",
            "MESSAGE_RECEIVED",

            "DISCONNECTION_REQUESTED",
            "DISCONNECTED",
            "DESTROY_REQUESTED",
            "DESTROYED",
            "CONNECTION_LOST",
            "CONNECTION_FAILURE",
    };

    return mqtt_state_names[state];
}