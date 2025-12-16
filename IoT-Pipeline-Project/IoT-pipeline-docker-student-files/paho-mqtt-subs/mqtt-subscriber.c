// mqtt-subscriber.c
// uses secure asynchronous paho-mqtt library for receiving mqtt messages

// compile: gcc -o mqtt-subscriber mqtt-subscriber.c -I/usr/local/include -L/usr/local/lib -lpaho-mqtt3as -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <MQTTAsync.h>
#include "subscriber-config.h"

// global variables
static int subscribed = 0;
static int finished = 0;


// ---------------------------------------------------
// ******* OWN FUNCTIONS

// function used to print equivalent name of state of our statemachine
// because states are technically enum, numbers from 1 to n
// our array here is exact same order as original statemachine,
// and with number selects corresponding name, helping debugging
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
            "MESSAGE_STORED_IN_MYSQL",

            "DISCONNECTION_REQUESTED",
            "DISCONNECTED",
            "DESTROY_REQUESTED",
            "DESTROYED",
            "CONNECTION_LOST",
            "CONNECTION_FAILURE",
    };
    if (state < 0 || state >= sizeof(mqtt_state_names)/sizeof(mqtt_state_names[0])) {
        return "UNKNOWN_STATE";
    }

    return mqtt_state_names[state];
}

// function used to change the state of statemachine
static inline void SetMQTTState(MQTTAmbient_context_t *pMQTTAmbient_context, mqtt_state_t state) { 
    pMQTTAmbient_context->mqtt_state = state; 
    printf("mqtt_state: %s (%d)\n", GetMQTTStateName(state), state);
}


// when start the software, receive parameters
void GetCmdLineOptions(int argc, char * const argv[], MQTTAmbient_context_t *pMQTTAmbient_context) {
    int opt;

    // ./mqtt-subscriber -h host -t Topic -q QoS
    // ./mqtt-subscriber -h tcp://iots_2025s-mosquitto-1:1883 -t iots_2025/Lahti/Forest2
    while((opt = getopt(argc, argv, "h:t:c:q:")) != -1)  {  
        switch(opt)  {  
        case 'h':   // h = host address e.g mosquitto address
            pMQTTAmbient_context->host_addr = optarg;
            break;
        case 't':   // t = (mqtt) topic to subscribe
            pMQTTAmbient_context->topic = optarg;
            break;
        case 'c': // c = client id, e.g "MQTT-Subscriber-1"
            pMQTTAmbient_context->client_id = optarg;
            break;
        case 'q':   // q = quality of service 1 or 0
            int qos = atoi(optarg);
            if (qos < 0 || qos > 2) {
                fprintf(stderr, "Invalid QoS %d, using default %d\n", qos, QOS);
            } else {
                pMQTTAmbient_context->qos = qos;
            }
            break;
        case '?':   // something else, unknown
            printf("Unknown option: %c\n", optopt);
            break;
        default:
        }
    }
}

// -----------------------------------------------------
// ******* MQTT ARCHITECTURE CALLBACK FUNCTIONS:

// onDeliveryComplete()
// we do not use this function as subscriber, but mandatory to define
void onDeliveryComplete(void* context, MQTTAsync_token token) {}

// onConnLost()
// triggers on situations when connection is lost
// paho-mqtt automatically triggers this
void onConnLost(void *context, char *cause)
{
    MQTTAmbient_context_t *pMQTTAmbient_context = (MQTTAmbient_context_t *)context;
    int rc;

    SetMQTTState(pMQTTAmbient_context, CONNECTION_LOST);
    printf("\nConnection lost. Cause: %s. Reconnecting...\n", cause);

    pMQTTAmbient_context->conn_opts.keepAliveInterval = 20;
    pMQTTAmbient_context->conn_opts.cleansession = 1;

    SetMQTTState(pMQTTAmbient_context, CONNECTION_REQUESTED);
    if ((rc = MQTTAsync_connect(pMQTTAmbient_context->client, &(pMQTTAmbient_context->conn_opts))) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        finished = 1;
    }
}

// onSubscribeSuccess()
// this
void onSubscribeSuccess(void* context, MQTTAsync_successData* response)
{
    MQTTAmbient_context_t *ctx = context;
    SetMQTTState(ctx, SUBSCRIBED);
    subscribed = 1;
    printf("Subscribed successfully\n");
}

// onSubscribeFailure()
void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
    MQTTAmbient_context_t *ctx = context;
    printf("Subscribe failed, rc=%d\n", response ? response->code : 0);
    finished = 1;
}

// subscribeRequest()
// we trigger this function if onConnection() function is succesful
// this function tries to start a subscription
int subscribeRequest(MQTTAmbient_context_t *ctx)
{
    // responseOptionsInitializer is ultra important for subscriber
    // this predefined structure tells to paho-mqtt library,
    // what is the next callback function, if initial function success or fails
    // it also gives default values that protects running the program
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

    opts.onSuccess      = onSubscribeSuccess;
    opts.onFailure      = onSubscribeFailure;
    opts.context        = ctx;

    SetMQTTState(ctx, SUBSCRIBE_REQUESTED);

    return MQTTAsync_subscribe(
        ctx->client,
        ctx->topic,
        ctx->qos,
        &opts
    );
}


// onMessageArrived()
int onMessageArrived(void *context,
                     char *topicName,
                     int topicLen,
                     MQTTAsync_message *message)
{
    (void)topicLen;  // topicLen voi olla 0 tai käyttämätön

    MQTTAmbient_context_t *ctx = (MQTTAmbient_context_t *)context;

    if (ctx == NULL || message == NULL) {
        return 1;  // nothing to do
    }

    // Update state: message received
    SetMQTTState(ctx, MESSAGE_RECEIVED);

    // Print topic
    printf("\n--- MQTT MESSAGE RECEIVED ---\n");
    printf("Topic: %s\n", topicName);

    // Print payload safely (payload is NOT null-terminated)
    printf("Payload (%d bytes): ", message->payloadlen);
    fwrite(message->payload, 1, message->payloadlen, stdout);
    printf("\n");

    // Optional: print QoS and retained flag
    printf("QoS: %d, retained: %d\n",
           message->qos,
           message->retained);

    // TODO:
    // - parse payload (e.g. JSON)
    // - store data into MySQL
    // SetMQTTState(ctx, MESSAGE_STORED_IN_MYSQL);

    printf("-----------------------------\n");

    // Free resources (MANDATORY in Paho, or memory leak)
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);

    // message was succesfully processed
    return 1;   
}


// onConnect()
// this function is called when connection to broker is succesful
void onConnect(void* context, MQTTAsync_successData* response) {
    MQTTAmbient_context_t *ctx = (MQTTAmbient_context_t *)context;
    SetMQTTState(ctx, CONNECTED);
    // MOST IMPORTANT LINE FOR SUBSCRIBER
    // this line determines are we publisher (so we would have e.g SendMessage)
    // or are we subscriber
    // this time we are subscriber, so:
    if (subscribeRequest(ctx) != MQTTASYNC_SUCCESS) {
        printf("Failed to start subscribe\n");
        SetMQTTState(ctx, CONNECTION_FAILURE);
    }
}


// onConnectFailure()
// this function is called when connection to broker fails
void onConnectFailure(void* context, MQTTAsync_failureData* response) {
    MQTTAmbient_context_t *ctx = (MQTTAmbient_context_t *)context;
    printf("Failed to connect, rc = %d\n", response ? response->code : 0);
    SetMQTTState(ctx, CONNECTION_FAILURE);
}


// ---------------------------------------------------------------------
// ******* MAIN FUNCTION READS STATEMACHINE TO DETERMINE WHAT TO DO NEXT
int main(int argc, char *argv[]) {

    // return code variable
    int rc;

    MQTTAmbient_context_t MQTTAmbient_context = MQTTAmbient_context_initializer;

    GetCmdLineOptions(argc, argv, &MQTTAmbient_context);

    // state 1: starting
    printf("Starting MQTT-Subscriber. Host: %s. Topic: %s. QoS: %d...\n",
        MQTTAmbient_context.host_addr, 
        MQTTAmbient_context.topic, 
        MQTTAmbient_context.qos);
    
    SetMQTTState(&MQTTAmbient_context, STARTING);

    // state 2: create client, subscriber is a client to mosquitto broker
    if ((rc = MQTTAsync_create(
            &MQTTAmbient_context.client,
            MQTTAmbient_context.host_addr,
            MQTTAmbient_context.client_id,
            MQTTCLIENT_PERSISTENCE_NONE, 
            NULL)) != MQTTASYNC_SUCCESS)
        {
                printf("Failed to create client, return code %d\n", rc);
                return EXIT_FAILURE;
        }
    SetMQTTState(&MQTTAmbient_context, CLIENT_CREATED);

    // state 3: callbacks_set
    MQTTAsync_setCallbacks(
        MQTTAmbient_context.client, 
        &MQTTAmbient_context, 
        // onConnLost triggers if connection to broker stops after it alrdy was established
        onConnLost, 
        // onMessageArrived is SUPER IMPORTANT
        // if connection is success, it will call subscribeRequest() and onSubscribeSuccess()
        // but those do not directly determine, what to do when a message arrives
        // instead, it is determined here on the onMessageArrived function
        // we can decide ourselves the content of this function
        onMessageArrived, 
        // onDeliveryComplete is only used on publisher, but still mandatory to define
        onDeliveryComplete
    );
    SetMQTTState(&MQTTAmbient_context, CALLBACKS_SET);

    // state 4: request for connection
    MQTTAmbient_context.conn_opts.keepAliveInterval = 20;
    MQTTAmbient_context.conn_opts.cleansession = 1;
    MQTTAmbient_context.conn_opts.onSuccess = onConnect;
    MQTTAmbient_context.conn_opts.onFailure = onConnectFailure;
    // reference to the whole context is the most important single thing, that callback functions need
    // and the callback functions receive their context via setting conn_opts.context
    MQTTAmbient_context.conn_opts.context = &MQTTAmbient_context; 

    SetMQTTState(&MQTTAmbient_context, CONNECTION_REQUESTED);

    rc = MQTTAsync_connect(
            MQTTAmbient_context.client,
            &MQTTAmbient_context.conn_opts
        );

    if (rc != MQTTASYNC_SUCCESS) {
        printf("Failed to start connect, rc=%d\n", rc);
        return EXIT_FAILURE;
    }


    // ----------------------------------------------------
    // states 5... n
    // keep listening for messages
    // after connection has been established with MQTTAsync_connect(), 
    // the library handles listening and redirecting callbacks automatically
    while (!finished) {
        pause();
    }

} // main ends