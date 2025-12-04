// mqtt-ambient-publisher.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <MQTTAsync.h>
#include "mqtt-ambient-config.h"
#include "mqtt-ambient-data.h"

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
        "MESSAGE_PUBLISH_REQUESTED",
        "MESSAGE_PUBLISHED",
        "MESSAGE_PUBLISH_ERROR",
        "MESSAGE_DELIVERY_COMPLETE",
        "DISCONNECTION_REQUESTED",
        "DISCONNECTED",
        "DESTROY_REQUESTED",
        "DESTROYED",
        "CONNECTION_LOST",
        "CONNECTION_FAILURE"
    };

    return mqtt_state_names[state];
}

static inline void SetMQTTState(MQTTAmbient_context_t *pMQTTAmbient_context, mqtt_state_t state) { 
    pMQTTAmbient_context->mqtt_state = state; 
    printf("mqtt_state: %s (%d)\n", GetMQTTStateName(state), state);
}

static inline const char *GetMQTTAmbientOpModeName(MQTTAmbient_op_mode_t op_mode) { 
    static const char *mqtt_op_mode_names[] = {
    "SINGLE_SHOT",
    "COUNT_LIMITED",
    "FOREVER",
    };

    return mqtt_op_mode_names[op_mode];
}


// TODO: find out how letters of this cmd line options work
void GetCmdLineOptions(int argc, char * argv[], MQTTAmbient_context_t *pMQTTAmbient_context) {
    int opt;

    // ./mqtt-ambient-publisher -h host -c ClientID -t Topic -q QoS -d nn -[S|Lxx|F]
    // ./mqtt-ambient-publisher -h tcp://ds-2024-mosquitto-1:1883 -c MQTTAmbientPub -t Lahti/Ambient -[S|Lxx|F]
    while((opt = getopt(argc, argv, "h:c:t:q:SL:Fd:")) != -1)  {  
        switch(opt)  {  
        case 'h':   // h = host address
            pMQTTAmbient_context->host_addr = optarg;
            break;
        case 'c':   // c = client id
            pMQTTAmbient_context->client_id = optarg;
            break;
        case 't':   // t = (mqtt) topic
            pMQTTAmbient_context->topic = optarg;
            break;
        case 'q':   // q = quality of service 1 or 0
            pMQTTAmbient_context->qos = atoi(optarg);
            break;
        case 'S':   // S = Mode SINGLESHOT
            pMQTTAmbient_context->op_mode = SINGLE_SHOT;
            break;
        case 'L':   // L = Message LIMIT
            pMQTTAmbient_context->op_mode = COUNT_LIMITED;
            pMQTTAmbient_context->publish_count_limit = atoi(optarg);
            break;
        case 'F':   // F = Mode FOREVER
            pMQTTAmbient_context->op_mode = FOREVER;
            pMQTTAmbient_context->publish_count_limit = -1;
            break;
        case 'd':   // d = delay of publishing message
            pMQTTAmbient_context->publish_delay = atoi(optarg);
            break;
        case '?':   // something else, unknown
            printf("Unknown option: %c\n", optopt);
            break;
        default:
        }
    }
}

void onConnLost(void *context, char *cause)
{
    MQTTAmbient_context_t *pMQTTAmbient_context = (MQTTAmbient_context_t *)context;
    int rc;

    SetMQTTState(pMQTTAmbient_context, CONNECTION_LOST);
    printf("\nConnection lost. Cause: %s. Reconnectig...\n", cause);

    pMQTTAmbient_context->conn_opts.keepAliveInterval = 20;
    pMQTTAmbient_context->conn_opts.cleansession = 1;
    // TODO: check if other fields should be cleared (onSuccess ect)
    // conn_opts = MQTTAsync_connectOptions_initializer;
    // conn_opts.keepAliveInterval = 20;
    // conn_opts.cleansession = 1;

    SetMQTTState(pMQTTAmbient_context, CONNECTION_REQUESTED);
    if ((rc = MQTTAsync_connect(pMQTTAmbient_context->client, &(pMQTTAmbient_context->conn_opts))) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        finished = 1;
    }
}

int onMessageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    printf("Message with topic %s arrived length is %d.\n%s", topicName, topicLen, (char *)message->payload);
    return 1;
}

// Note onDeliveryComplete will not fire if onMessageArrived is not armed
void onDeliveryComplete(void* context, MQTTAsync_token dt) {
    MQTTAmbient_context_t *pMQTTAmbient_context = (MQTTAmbient_context_t *)context;

    delivered_token = dt;

    // This is NOT fired in qos 0
    // NOTE! publish_count is inceremnted here only in qos 1 or 2
    SetMQTTState(pMQTTAmbient_context, MESSAGE_DELIVERY_COMPLETE);
    pMQTTAmbient_context->publish_count++;
}

// if qos is 0, we will never know if it was completed
// if wos is 1 or 2, we will get MESSAGE_DELIVERY_COMPLETE
void onSendSuccess(void* context, MQTTAsync_successData* response) {
    MQTTAmbient_context_t *pMQTTAmbient_context = (MQTTAmbient_context_t *)context;

    printf("Message with token %d sent successfully.\n", response->token);

    // Set mqtt_state only in qos 0 (qos 1 and 2 uses onDeliveryComplete)
    if(pMQTTAmbient_context->qos == 0) {
        SetMQTTState(pMQTTAmbient_context, MESSAGE_PUBLISHED);
        pMQTTAmbient_context->publish_count++;
    }
}

void onSendFailure(void* context, MQTTAsync_failureData* response) {
    MQTTAmbient_context_t *pMQTTAmbient_context = (MQTTAmbient_context_t *)context;
    SetMQTTState(pMQTTAmbient_context, MESSAGE_PUBLISH_ERROR);
    printf("Failed to send message, return code %d\n", response ? response->code : 0);
}

void SendMessage(void *context) {
    int rc;
    // generate data into variable
    AmbientData_t ad;
    // prepare to create a json string
    char buf[MQTT_AMBIENT_JSON_MAX_LEN];

    MQTTAmbient_context_t *pMQTTAmbient_context = (MQTTAmbient_context_t *)context;

    // Prepare the message for publishing
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

    printf("SendMessage called with %s (%d)\n", 
        GetMQTTStateName(pMQTTAmbient_context->mqtt_state), pMQTTAmbient_context->mqtt_state);

    // Configure the response options for publishing
    opts.onSuccess = onSendSuccess;
    opts.onFailure = onSendFailure;
    opts.context = pMQTTAmbient_context;

    // Set the message content
    MeasureAmbientData(&ad, LOCATION, DEVICE);
    pubmsg.payload = StringifyAmbientData(&ad, buf);
    pubmsg.payloadlen = (int)strlen(pubmsg.payload);

    pubmsg.qos = pMQTTAmbient_context->qos;
    pubmsg.retained = 0; // persistency 0

    // Publish the message asynchronously
    SetMQTTState(pMQTTAmbient_context, MESSAGE_PUBLISH_REQUESTED);

    if ((rc = MQTTAsync_sendMessage(pMQTTAmbient_context->client, pMQTTAmbient_context->topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start publish, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    return;
}

void onConnect(void* context, MQTTAsync_successData* response) {
    MQTTAmbient_context_t *pMQTTAmbient_context = (MQTTAmbient_context_t *)context;
    SetMQTTState(pMQTTAmbient_context, CONNECTED);
    SendMessage(context);
}

void onConnectFailure(void* context, MQTTAsync_failureData* response) {
    MQTTAmbient_context_t *pMQTTAmbient_context = (MQTTAmbient_context_t *)context;
    printf("Failed to connect, rc = %d\n", response ? response->code : 0);
    SetMQTTState(pMQTTAmbient_context, CONNECTION_FAILURE);
    exit(EXIT_FAILURE);
}

int CheckToDisconnect(MQTTAmbient_context_t *pMQTTAmbient_context) {
    // disconnect if single_shot mode or count_limited mode and publish count exceed the limit
    if(pMQTTAmbient_context->op_mode == SINGLE_SHOT || 
        pMQTTAmbient_context->op_mode == COUNT_LIMITED && 
        (pMQTTAmbient_context->publish_count >= pMQTTAmbient_context->publish_count_limit)) {
        return 1;
    }
    return 0;
}

void onDisconnect(void* context, MQTTAsync_successData* response)
{
    MQTTAmbient_context_t *pMQTTAmbient_context = (MQTTAmbient_context_t *)context;
    // set state to disconnected
    SetMQTTState(pMQTTAmbient_context, DISCONNECTED);
    // make finished flag true
    // main function while stops working when finished true
    finished = 1;
    printf("Successful disconnection\n");
}

void DisconnectRequest(MQTTAmbient_context_t *pMQTTAmbient_context) {
    MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
    int rc; //return code

    opts.onSuccess = onDisconnect;
    opts.context   = pMQTTAmbient_context;

    SetMQTTState(pMQTTAmbient_context, DISCONNECTION_REQUESTED);

    if ((rc = MQTTAsync_disconnect(pMQTTAmbient_context->client, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start disconnect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    MQTTAmbient_context_t MQTTAmbient_context = MQTTAmbient_context_initializer;
    int one_sec_delay_counter = 0;
    int rc;

    GetCmdLineOptions(argc, argv, &MQTTAmbient_context);

    printf("Starting MQTT ambient publish operation in mode %s (limit=%d, delay=%d) \n"
        "to host %s with clientId %s. Topic is %s and QoS is %d...\n",
        GetMQTTAmbientOpModeName(MQTTAmbient_context.op_mode), 
        MQTTAmbient_context.publish_count_limit,
        MQTTAmbient_context.publish_delay,
        MQTTAmbient_context.host_addr, MQTTAmbient_context.client_id, 
        MQTTAmbient_context.topic, MQTTAmbient_context.qos);

    SetMQTTState(&MQTTAmbient_context, STARTING);

    // ----------------------------------------------------
    // State 1: Create client
    // find params from mqtt-ambient-config.h
    MQTTAsync_create(
        &MQTTAmbient_context.client, 
        MQTTAmbient_context.host_addr,
        MQTTAmbient_context.client_id,
        MQTTCLIENT_PERSISTENCE_NONE,
        NULL
    );
    SetMQTTState(&MQTTAmbient_context, CLIENT_CREATED);

    // ----------------------------------------------------
    // State 2: Set Callbacks
    // params: client, deliver context with every callback, 
    //          return three addresses (connection lost, message arrived, or delivery completed)
    MQTTAsync_setCallbacks(
        MQTTAmbient_context.client, 
        &MQTTAmbient_context, 
        onConnLost, 
        onMessageArrived, 
        onDeliveryComplete 
    );

    SetMQTTState(&MQTTAmbient_context, CALLBACKS_SET);

    // ----------------------------------------------------
    // state 3: connection requested
    MQTTAmbient_context.conn_opts.keepAliveInterval = 20;
    MQTTAmbient_context.conn_opts.cleansession = 1;
    MQTTAmbient_context.conn_opts.onSuccess = onConnect;
    MQTTAmbient_context.conn_opts.onFailure = onConnectFailure;
    // store updated context to original context
    MQTTAmbient_context.conn_opts.context = &MQTTAmbient_context;

    SetMQTTState(&MQTTAmbient_context, CONNECTION_REQUESTED);
    if ((rc = MQTTAsync_connect(MQTTAmbient_context.client, &MQTTAmbient_context.conn_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        return EXIT_FAILURE;
    }

    // ----------------------------------------------------
    // states 4-n, from connect to publish to delivery to errors

    // loop once a second
    while (!finished) {
        usleep(1000000);
        one_sec_delay_counter++;

        // check if publish has been completed
        if(MQTTAmbient_context.mqtt_state == MESSAGE_PUBLISH_REQUESTED) {
            printf("--waiting for publish to complete \n");
            continue;
        }

        // check if we want to disconnect
        if(CheckToDisconnect(&MQTTAmbient_context)) {
            DisconnectRequest(&MQTTAmbient_context);
            break;
        }
        // we do not want to disconnect yet
        else {
            // if we send messages for example every 30 seconds
            // we can define delay in command options with -d tag
            // every second we add 1 to delay counter
            // so when we reach e.g 30 seconds, that is the hallmark to send next message
            // also reset delay counter every time a message has been sent
            if(one_sec_delay_counter >= MQTTAmbient_context.publish_delay) {
                // if qos is 0, message must be published, not care if delivered or not
                if(MQTTAmbient_context.qos == 0 && (MQTTAmbient_context.mqtt_state == MESSAGE_PUBLISHED)) {
                    SendMessage(&MQTTAmbient_context);
                    one_sec_delay_counter = 0;
                }
                // if qos is 1 or 2, publish is not enough, but must be delivery complete state
                else if (MQTTAmbient_context.qos == 0 && (MQTTAmbient_context.mqtt_state == MESSAGE_PUBLISHED)) {
                    SendMessage(&MQTTAmbient_context);
                    one_sec_delay_counter = 0;
                }
            }
        }
    }
    // because software ends to DisconnectRequest function
    // that function asks from pahomqtt library the disconnect
    // DisconnectRequest calls helped function onDisconnect()
    // and then that function when finished sets up flag 'DISCONNECTED'
    // so now we wait for that to happen
    while(MQTTAmbient_context.mqtt_state != DISCONNECTED) {
        usleep(500000); // half second cyckle
    }

    // clean up
    SetMQTTState(&MQTTAmbient_context, DESTROY_REQUESTED);
    MQTTAsync_destroy(&MQTTAmbient_context.client);
    SetMQTTState(&MQTTAmbient_context, DESTROYED);
    printf("Exiting...\n");
    return 0;

}