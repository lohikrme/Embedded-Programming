// subscriber-config.h

#include <MQTTAsync.h>

// Define default values for connection
#define ADDRESS     "tcp://iots_2025s-mosquitto-1:1883"     // Broker address
#define CLIENTID    "MQTT-Subscriber-1"                     // Client ID
#define TOPIC       "LAB/DS2025s/Ambient"                   // Topic to subscribe
#define QOS         1                                       // QoS level

// state machine architecture is used to handle which state of code is going
typedef enum mqtt_state {
    STARTING, 
    CLIENT_CREATED, 
    CALLBACKS_SET, 
    CONNECTION_REQUESTED,
    CONNECTED,
    SUBSCRIBE_REQUESTED,
    SUBSCRIBED,
    MESSAGE_RECEIVED,

    DISCONNECTION_REQUESTED,
    DISCONNECTED,
    DESTROY_REQUESTED,
    DESTROYED,
    CONNECTION_LOST,
    CONNECTION_FAILURE,
} mqtt_state_t;


typedef struct _MQTTAmbient_context {
    char *host_addr;
    char *client_id;
    char *topic;
    int qos; // 0 1 or 2      

    mqtt_state_t mqtt_state;    

    MQTTAsync client;
    MQTTAsync_connectOptions conn_opts;
} MQTTAmbient_context_t;

// saves default values to MQTTAmbient_context
#define MQTTAmbient_context_initializer { .host_addr=ADDRESS, .client_id=CLIENTID, .topic=TOPIC, .qos=QOS, .mqtt_state=STARTING, .conn_opts=MQTTAsync_connectOptions_initializer }



