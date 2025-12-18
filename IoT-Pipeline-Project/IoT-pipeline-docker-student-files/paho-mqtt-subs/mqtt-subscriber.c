// mqtt-subscriber.c
// uses secure asynchronous paho-mqtt library for receiving mqtt messages

// compile: gcc -o mqtt-subscriber mqtt-subscriber.c -I/usr/local/include -L/usr/local/lib -lpaho-mqtt3as -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <MQTTAsync.h>
#include "subscriber-config.h"
#include <cjson/cJSON.h> // supports json files in pure C
#include "mqtt-ambient-data.h"
#include <mysql/mysql.h>


// global variables
static int subscribed = 0;
static int finished = 0;
static MYSQL *conn;



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
    // ./mqtt-subscriber -h tcp://iots_2025s-mosquitto-1:1883 -t iots_2025/+/+/#
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

// safe_strdup_item()
// check if item exists, is a cJSON string and has a valuestring
// if all conditions full, create a copy of a string with strdup, else NULL
// the goal is AmbientData_t remains even after cJSON_Delete()
static char* safe_strdup_item(const cJSON *item) {
    return (item && cJSON_IsString(item) && item->valuestring)
           ? strdup(item->valuestring)
           : NULL;
}

// ParseAmbientData()
// parse arriving message payload into AmbientData_t struct using cjson library
// remember to create a new AmbientData_t struct to store data
AmbientData_t* parseAmbientData(const char *payload, int len, AmbientData_t *out) {
    if (!payload || !out) {
        return NULL;
    }

    // make sure length of arriving data is okey
    // the info of suitable length is inside mqtt-ambient-data.h
    if (len <= 0 || len > MQTT_AMBIENT_JSON_MAX_LEN) {
        return NULL;
    }

    // deep copy data into buffer string
    char *buffer = (char*)malloc((size_t)len + 1);
    if (!buffer) {
        fprintf(stderr, "malloc failed\n");
        return NULL;
    }
    memcpy(buffer, payload, (size_t)len);
    buffer[len] = '\0';

    // parse json from the buf string and delete buf
    cJSON *json = cJSON_Parse(buffer);
    free(buffer);

    // if parsing fails, print error
    if (!json) {
        fprintf(stderr, "JSON parse error: %s\n", cJSON_GetErrorPtr());
        return NULL;
    }

    // check out different fields according to AmbientData_t
    cJSON *addr = cJSON_GetObjectItemCaseSensitive(json, "address");
    cJSON *loc  = cJSON_GetObjectItemCaseSensitive(json, "location");
    cJSON *dev  = cJSON_GetObjectItemCaseSensitive(json, "device");
    cJSON *temp = cJSON_GetObjectItemCaseSensitive(json, "temperature");
    cJSON *hum  = cJSON_GetObjectItemCaseSensitive(json, "humidity");
    cJSON *pres = cJSON_GetObjectItemCaseSensitive(json, "pressure");
    cJSON *co2  = cJSON_GetObjectItemCaseSensitive(json, "co2");
    cJSON *mt   = cJSON_GetObjectItemCaseSensitive(json, "mtime");

    // strings into own memory (dupataan, jotta pysyvät elossa cJSON_Delete:n jälkeen)
    out->address  = safe_strdup_item(addr);
    out->location = safe_strdup_item(loc);
    out->device   = safe_strdup_item(dev);

    // verify numerical values, replace with 0.0 if value not numerical
    out->temperature = (temp && cJSON_IsNumber(temp)) ? (float)temp->valuedouble : 0.0f;
    out->pressure    = (pres && cJSON_IsNumber(pres)) ? (float)pres->valuedouble : 0.0f;
    out->humidity    = (hum  && cJSON_IsNumber(hum))  ? (float)hum->valuedouble  : 0.0f;
    out->co2         = (co2  && cJSON_IsNumber(co2))  ? (float)co2->valuedouble  : 0.0f;

    // verify time (käytetään havaintoaikaa, ei vastaanottoa)
    if (mt && cJSON_IsNumber(mt)) {
        out->mtime = (time_t)mt->valuedouble;
    } else {
        out->mtime = (time_t)0; // jos puuttuu
    }

    // clear memory
    cJSON_Delete(json);
    // return AmbientData_t with datas added
    return out;
}

// storeInMysql()
// this function handles storing data into mysql database
// reason is that, the database has metric_key - value pairs, so when we receive e.g 3 variables as data
// we store 3 rows to database, so 3 queries needed for 1 mqtt message containing full dht22 sensor data
void storeInMysql(const AmbientData_t *ad) {
    // initiatilize querySentences
    char querySentenceTemperature[MQTT_AMBIENT_JSON_MAX_LEN + 30];
    char querySentenceHumidity[MQTT_AMBIENT_JSON_MAX_LEN + 30];
    char querySentenceAirpressure[MQTT_AMBIENT_JSON_MAX_LEN + 30];

    // save suitable strings inside querySentences
    snprintf(querySentenceTemperature, sizeof(querySentenceTemperature), 
        "INSERT INTO iot_data (address, location, device, metric_key, value) " 
        "VALUES ('%s', '%s', '%s', 'Temperature', %.2f)", 
        ad->address ? ad->address : "", 
        ad->location ? ad->location : "", 
        ad->device ? ad->device : "", 
        ad->temperature); 

    snprintf(querySentenceHumidity, sizeof(querySentenceHumidity), 
        "INSERT INTO iot_data (address, location, device, metric_key, value) " 
        "VALUES ('%s', '%s', '%s', 'Humidity', %.2f)", 
        ad->address ? ad->address : "", 
        ad->location ? ad->location : "", 
        ad->device ? ad->device : "", 
        ad->humidity); 

    snprintf(querySentenceAirpressure, sizeof(querySentenceAirpressure), 
        "INSERT INTO iot_data (address, location, device, metric_key, value) " 
        "VALUES ('%s', '%s', '%s', 'Airpressure', %.2f)", 
        ad->address ? ad->address : "", 
        ad->location ? ad->location : "", 
        ad->device ? ad->device : "", 
        ad->pressure); 
        
    // run all querySentences to save temp, humid and airpressure
    if (mysql_query(conn, querySentenceTemperature) != 0) { 
        fprintf(stderr, "querySentenceTemperature Failure: %s\n", mysql_error(conn)); 
    } 
    if (mysql_query(conn, querySentenceHumidity) != 0) { 
        fprintf(stderr, "querySentenceHumidity Failure: %s\n", mysql_error(conn)); 
    } 
    if (mysql_query(conn, querySentenceAirpressure) != 0) { 
        fprintf(stderr, "querySentenceAirpressure Failure: %s\n", mysql_error(conn)); 
    } 
}

// onMessageArrived()
// this is the function that is automatically called by  paho-mqtt when a message arrives
// content is my own - printing data, parsing it, and storing into mysql
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

    // parse ambient data into ad struct, so it is easier to access
    AmbientData_t ad = {0}; // alustetaan kaikki kentät nollaan/NULL:iin
    if (!parseAmbientData((const char*)message->payload, message->payloadlen, &ad)) {
        fprintf(stderr, "Failed to parse ambient data\n");
    } else {
        // Tässä vaiheessa ad sisältää duplatut stringit ja numeriarvot
        printf("Parsed: address=%s, location=%s, device=%s, "
               "temp=%.2f, pressure=%.2f, humidity=%.2f, co2=%.2f, mtime=%lu\n",
               ad.address ? ad.address : "(null)",
               ad.location ? ad.location : "(null)",
               ad.device ? ad.device : "(null)",
               ad.temperature, ad.pressure, ad.humidity, ad.co2,
               (unsigned long)ad.mtime);

        // Save ambient data into mysql database
        storeInMysql(&ad);
        
        // update the mqtt state
        SetMQTTState(ctx, MESSAGE_STORED_IN_MYSQL);
    }

    // Free variables stored with strdub to avoid memory leaks
    free(ad.address);
    free(ad.location);
    free(ad.device);

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

    // init mysql database connection at the start of main
    if ((conn = mysql_init(NULL)) == NULL)                                                             
    {                                                                                                  
        fprintf(stderr, "Could not init DB\n");                                                 
        return EXIT_FAILURE;                                                                             
    }  
    // params: mysql init, service name in docker network, username, password, db_name, portnumber
    if (mysql_real_connect(conn, "mysql", "user", "Koodaus1", "iots_2025", 3306, NULL, 0) == NULL)             
    {                                                                                                  
        fprintf(stderr, "DB Connection Error\n");                                                        
        return EXIT_FAILURE;                                                                             
    }  

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