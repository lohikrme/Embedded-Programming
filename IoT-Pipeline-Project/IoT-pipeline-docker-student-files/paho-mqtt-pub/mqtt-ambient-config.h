// mqtt-ambient-config.h

#include <MQTTAsync.h>

// Define default values for connection
#define BROKER_ADDRESS     "tcp://iots_2025s-mosquitto-1:1883"     // Broker address
#define CLIENTID    "MQTTAmbientPub"                        // Client ID
#define PAYLOAD     "Testing..."                            // Client ID
#define TOPIC       "LAB/DS2025s/Ambient"                   // Topic to publish to
#define QOS         1                                       // QoS level
#define TIMEOUT     10000L                                  // Timeout for the publish operation

typedef enum _MQTTAmbient_op_mode {
    SINGLE_SHOT=0,  // if use this, one message and destroy
    COUNT_LIMITED,  // e.g set send 15 messages and then stop
    FOREVER,        // running for now until stopped
} MQTTAmbient_op_mode_t;

// state machine architecture is used to handle which state of code is going
typedef enum mqtt_state {
    STARTING, 
    CLIENT_CREATED, 
    CALLBACKS_SET, 
    CONNECTION_REQUESTED,
    CONNECTED,
    MESSAGE_PUBLISH_REQUESTED,
    MESSAGE_PUBLISHED,
    MESSAGE_PUBLISH_ERROR,
    MESSAGE_DELIVERY_COMPLETE,

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
    int qos;                    
    int publish_count;          // defines amount of messages to me send, COUNT_LIMITED mode
    int publish_count_limit;    // if -1, it wont check any limits so it works FOREVER mode
    int publish_delay;          

    mqtt_state_t mqtt_state;    
    MQTTAmbient_op_mode_t op_mode;

    MQTTAsync client;
    MQTTAsync_connectOptions conn_opts;
} MQTTAmbient_context_t;

// saves default values to MQTTAmbient_context
// saves default values to MQTTAmbient_context
#define MQTTAmbient_context_initializer { .host_addr=BROKER_ADDRESS, .client_id=CLIENTID, .topic=TOPIC, .qos=QOS, .publish_count=0, .publish_count_limit=1, .publish_delay=1, .mqtt_state=STARTING, .op_mode=SINGLE_SHOT, .conn_opts=MQTTAsync_connectOptions_initializer }
