/*udp-echo-server.cpp
$ g++ -std=c++14 udp-echo-server.cpp -o udp-echo-server
*/

// start server in linux with command (p = port, d = delay)
// ./udp-echo-server -p 12700 -d 5

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h> // socket.h relies on this
#include <sys/socket.h> // contain struct sockaddr % sockid = socket(AF_INET, SOCK_DGRAM, 0)
#include <arpa/inet.h> // contain inet_pton, inet_ntoa, etc
#include <netinet/in.h> // contain sockaddr_in and IP-addresses
#include <cjson/cJSON.h> // supports json files in pure C
#include <sys/stat.h> // mkdir
#include <time.h>


#define DEFAULT_SERVER_PORT 8080
#define MAXSIZE 1000
#define RESPONSE_MESSAGE_SIZE 512

// typedef allows defining own type
// delay in secs is used for artificial delay in response
typedef struct _config {
    int port;
    int delay_in_secs;
} Config_t;

// global variables
static int sockid = -1;
char responseMessage[RESPONSE_MESSAGE_SIZE] = "UDP Server has received data. ";

// function prototypes mean order of writing functions doesn't matter
void SaveToFile(const char* client_id, const char* filename, const char* content, const char* filemode);
void GetCmdLineOptions( int argc, char *argv[], Config_t *pconfig);
void InterruptionHandler(int sig);
void HandleIncoming(const char* in_buf, struct sockaddr_in* clientaddr);
int main(int argc, char *argv[]);

//------------------------------------------------------------------------

// GetCmdLineOptions() is a function, that reads given extra tags after calling the main software
// e.g ./udp-echo-server -p nnn would be summon of main software, and nnn would be data associated with -p tag
void GetCmdLineOptions( int argc, char *argv[], Config_t* pconfig) {
    int opt;
    // getopt searches for options with tags, such as "-p" and takes its value such as "nnn"
    while((opt=getopt(argc, argv, "p:d:")) != -1) {
        switch(opt) {
            case 'p':
                // mene osoittimen pconfig osoittamaan rakenteeseen ja käytä sen port-kenttää
                pconfig->port = atoi(optarg);
                break;
            case 'd':
                pconfig -> delay_in_secs = atoi(optarg);
                break;
            case '?':
            fprintf(stderr, "Unknown option %c\n", optopt);
                exit(EXIT_FAILURE);
                break;
            default:
                fprintf(stderr, "Usage: udp-echo-server [-p port\n]");
        }
    }
}

// InterruptionHandler() is a signal handler function
// when user press ctrl + c, os (linux, windows) sends SIGINT (interrupt signal) to the process
// so, this InterruptionHandler does not actually receive letter 'c', but a SIGINT signal
void InterruptionHandler(int sig) {
    char letter;
    // ignore new sigint signals after first one under process
    // this commands tells straight to OS to do nothing when sig-type signal arrives again
    signal(sig, SIG_IGN);
    // ask user if they rly want to wait (if user writes y, confirms)
    printf("\nDo you really want to quit [y/n]?");
    letter = getchar();
    if(toupper(letter)=='Y') {
        // todo: close socket
        if (sockid >= 0) {
            close(sockid);
            sockid = -1;
        }
        exit(0);
    } 
    // if user writes something else than "y"
    // interpret it as not wanting to end program
    else {
        // this command tells straight to OS to deliver sigint signals to InterruptionHandler function
        signal(SIGINT, InterruptionHandler);
    }
}


int main(int argc, char *argv[]) {
    // initialize a variable called config that is type of Config_t
    Config_t config;
    config.port=12700;
    config.delay_in_secs=1;
    // save socket id into a variable, when socket is created, OS gives it a number
    int len, n, msg_cnt;
    len = n = msg_cnt = 0;
    char in_buf[MAXSIZE+4];
    char out_buf[MAXSIZE+32];

    // address of server and client, sockaddr_in contains IP-address and port
    // usually sockaddr_in is used with AF_INET family, aka IPv4 internet addresses
    struct sockaddr_in serveraddr, clientaddr;

    // read at the start of invoking main function the parameters main is created with
    GetCmdLineOptions(argc, argv, &config);

    // now that we know what port we listen to (default or user input), we tell user we listen the port
    printf("udp-echo-server listening to port %d...\n Press CTRL+C to stop\n", config.port);

    // start listening for Interruption signal and deliver those to InterruptionHandler
    signal(SIGINT, InterruptionHandler);

    // create an UDP socket
    // for UDP = SOCK_DGRAM
    // for TCP = SOCK_STREAM
    // AF_INET means IPv4 address
    // 0 means lets choose protocol automatically (typically udp or tcp)
    // then socket returns an integer that acts as id of socket
    if ((sockid=socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        // if problem, print error
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    // memset receives pointer/reference to memory area and size of it
    // number can be anything between 0 and 255, aka one byte (2^8)
    // one byte at time it gives suitable value, in our case every byte is 0000 0000
    memset(&serveraddr, 0, sizeof(serveraddr));
    memset(&clientaddr, 0, sizeof(clientaddr));

    // Fill in server info
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(config.port);

    // make serveraddr_in to serveraddr type before bind
    if(bind(sockid, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) < 0) {
        perror("Socket bind failed!");
        exit(EXIT_FAILURE);
    }

    len = sizeof(clientaddr);
    // while loop keeps the server running
    while(1) {
        // use in_buf array as memory location to receive data
        if((n = recvfrom(sockid, in_buf, MAXSIZE-1, 0, (struct sockaddr*) &clientaddr, (socklen_t*)&len)) < 0) {
            perror("recvfrom failed");
            exit(EXIT_FAILURE);
        }
        // make sure string ends to the null
        in_buf[n] = '\0';
        msg_cnt++;
        printf("Client req (%d) received data from %s:%u\n", 
            msg_cnt, 
            inet_ntoa(clientaddr.sin_addr),
            clientaddr.sin_port
        );

        // Handle data
        HandleIncoming(in_buf, &clientaddr);


        // add additional delay to response
        if (config.delay_in_secs) {
            sleep(config.delay_in_secs);
        }
        // saves text into a string with snprintf into out_buf memory
        snprintf(out_buf, sizeof(out_buf), "%sPacket number: [%d]", responseMessage, msg_cnt);

        // next step is decide flag where message goes
        sendto(
            sockid, 
            out_buf, 
            strlen(out_buf), 
            MSG_CONFIRM, 
            (struct sockaddr*)&clientaddr, 
            len
        );
        printf("reply sent\n");
    }

    // after done useful stuff with socket, close it
    if (sockid > 0) {
        close(sockid);
        sockid = -1;
    }
    
    return 0;
}

// function that receives incoming messages from client
// in_buf is the memory area where we store data receives
// sockaddr_in is a struct that contains port number and ipv4 address
void HandleIncoming(const char* in_buf, struct sockaddr_in* clientaddr) {
    // CJSON* root is the way how cjson library parses arriving json data
    cJSON* root = cJSON_Parse(in_buf);
    // make sure json was parse-able
    if (!root) {
        printf("Faulty JSON file\n");
        snprintf(responseMessage, sizeof(responseMessage), "Faulty JSON file");
        return;
    }

    // take rows from the json
    cJSON* cj_type = cJSON_GetObjectItem(root, "type");
    cJSON* cj_content = cJSON_GetObjectItem(root, "content");
    cJSON* cj_client_id = cJSON_GetObjectItem(root, "client_id");

    // make sure all needed rows were inside the json
    if (!cj_type || !cj_content || !cj_client_id ||
        !cJSON_IsString(cj_type) || !cJSON_IsString(cj_content) || !cJSON_IsString(cj_client_id)) {
        printf("Missing or faulty fields in JSON\n");
        snprintf(responseMessage, sizeof(responseMessage), "Missing or faulty fields in JSON");
        // delete root of json to avoid memory leak
        cJSON_Delete(root);
        return;
    }

    // take data from json to string (char*) type
    const char* type_str = cj_type->valuestring;
    const char* content_str = cj_content->valuestring;
    const char* client_id_str = cj_client_id->valuestring;

    // strcmp function from <string.h> compares if 2 words are equal or not
    // so if type_str === "message", then we get 0
    // and that means we do not parse a file but a message
    // therefore. we open server's local log.txt file, and save it there
    if (strcmp(type_str, "message") == 0) {
        FILE* fp = fopen("log.txt", "a");
        if (fp) {
            time_t now = time(NULL);
            struct tm tm_now;
            // localtime_r function takes current time, and uses  provided storage buffer
            localtime_r(&now, &tm_now);
            // timestamp is actually array of chars aka string of C language
            char timestamp[64];
            // strftime converts datetime into string with format specified in params
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm_now);
            // standard c function to write formatted output to file or stdout
            fprintf(fp, "[%s] [%s] %s\n", timestamp, client_id_str, content_str);
            // close file after writing
            fclose(fp);

            printf("Message saved into log.txt file\n");
            // snprintf is the function, that safely moves a string into array of characters
            snprintf(responseMessage, sizeof(responseMessage), "Message saved into log.txt file");
        } else {
            perror("Failed writing a log file");
            snprintf(responseMessage, sizeof(responseMessage), "Failed writing a log file");
        }
    } 
    // so we are using again strcmp to compare if type given by json is 'file'
    else if (strcmp(type_str, "file") == 0) {
        // if it is, we will receive filename and mode of append, and use the info to create a file
        cJSON* cj_filename = cJSON_GetObjectItem(root, "filename");
        cJSON* cj_append = cJSON_GetObjectItem(root, "append");

        // make sure filename is parseable
        if (!cj_filename || !cJSON_IsString(cj_filename)) {
            printf("Missing or faulty file name!\n");
            // use again snprintf to safely store string into an array of chars
            snprintf(responseMessage, sizeof(responseMessage), "Missing or faulty file name!");
            // delete root of json to avoid memory leak
            cJSON_Delete(root);
            return;
        }
        // decide file append mode basedo n JSON value "append" = 'a' & "new" = 'w' 
        // default is "a" if not given
        const char* file_mode = "a"; /
        if (cj_append) {
            if (cJSON_IsString(cj_append)) {
                const char* mode_str = cj_append->valuestring;
                if (strcmp(mode_str, "append") == 0) {
                    file_mode = "a";
                } else if (strcmp(mode_str, "new") == 0) {
                    file_mode = "w";
                } else {
                    fprintf(stderr, "Unknown mode string '%s', defaulting to append\n", mode_str);
                }
            } else if (cJSON_IsBool(cj_append)) {
                file_mode = cJSON_IsTrue(cj_append) ? "a" : "w";
            } else if (cJSON_IsNumber(cj_append)) {
                file_mode = (cj_append->valueint != 0) ? "a" : "w";
            }
        }
        // save file using self-made SaveToFile function
        // remember that file_mode is straight a or w
        SaveToFile(client_id_str, cj_filename->valuestring, content_str, file_mode);
    } 
    // if something else than previous options happen, info client (with responseMessage) that unknown message type, not processed
    else {
        printf("Unknown message type: %s\n", type_str);
        snprintf(responseMessage, sizeof(responseMessage), "Unknown message type");
    }
    // delete root of json to avoid memory leak
    cJSON_Delete(root);
}


// save data given by client to a filepath that is individual based on client id (name)
void SaveToFile(const char* client_id, const char* filename, const char* content, const char* filemode) {
    // Create root folder 'data' if need
    mkdir("data", 0777);

    // Create an own folder for each client, where client is known by ip_port as their id
    char folder[256];
    snprintf(folder, sizeof(folder), "data/%s", client_id);
    mkdir(folder, 0777);

    // Create a filepath
    char filepath[300];
    snprintf(filepath, sizeof(filepath), "%s/%s", folder, filename);

    // write into the file
    FILE* fp = fopen(filepath, filemode);
    if (fp) {
        fwrite(content, 1, strlen(content), fp);
        fclose(fp);
        printf("Saved into the file: %s\n", filepath);
        // store new responseMessage
        snprintf(responseMessage, sizeof(responseMessage), "Saved into the file: %s\n", filepath);
    } else {
        perror("Writing file failed...\n");
    }
}
