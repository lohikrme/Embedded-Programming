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
void SaveToFile(const char* client_id, const char* filename, const char* content, int append);
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
    // make a variable called config that is type of Config_t
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


void HandleIncoming(const char* in_buf, struct sockaddr_in* clientaddr) {
    cJSON* root = cJSON_Parse(in_buf);
    if (!root) {
        printf("Faulty JSON file\n");
        snprintf(responseMessage, sizeof(responseMessage), "Faulty JSON file");
        return;
    }

    cJSON* cj_type = cJSON_GetObjectItem(root, "type");
    cJSON* cj_content = cJSON_GetObjectItem(root, "content");
    cJSON* cj_client_id = cJSON_GetObjectItem(root, "client_id");

    if (!cj_type || !cj_content || !cj_client_id ||
        !cJSON_IsString(cj_type) || !cJSON_IsString(cj_content) || !cJSON_IsString(cj_client_id)) {
        printf("Missing or faulty fields in JSON\n");
        snprintf(responseMessage, sizeof(responseMessage), "Missing or faulty fields in JSON");
        cJSON_Delete(root);
        return;
    }

    const char* type_str = cj_type->valuestring;
    const char* content_str = cj_content->valuestring;
    const char* client_id_str = cj_client_id->valuestring;

    if (strcmp(type_str, "message") == 0) {
        FILE* fp = fopen("log.txt", "a");
        if (fp) {
            time_t now = time(NULL);
            struct tm tm_now;
            localtime_r(&now, &tm_now);
            char timestamp[64];
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm_now);

            fprintf(fp, "[%s] [%s] %s\n", timestamp, client_id_str, content_str);
            fclose(fp);

            printf("Message saved into log.txt file\n");
            snprintf(responseMessage, sizeof(responseMessage), "Message saved into log.txt file");
        } else {
            perror("Failed writing a log file");
            snprintf(responseMessage, sizeof(responseMessage), "Failed writing a log file");
        }
    } else if (strcmp(type_str, "file") == 0) {
        cJSON* cj_filename = cJSON_GetObjectItem(root, "filename");
        cJSON* cj_append = cJSON_GetObjectItem(root, "append");

        if (!cj_filename || !cJSON_IsString(cj_filename)) {
            printf("Missing or faulty file name!\n");
            snprintf(responseMessage, sizeof(responseMessage), "Missing or faulty file name!");
            cJSON_Delete(root);
            return;
        }

        int append_mode = 1;
        if (cj_append) {
            if (cJSON_IsBool(cj_append)) {
                append_mode = cJSON_IsTrue(cj_append) ? 1 : 0;
            } else if (cJSON_IsNumber(cj_append)) {
                append_mode = (cj_append->valueint != 0) ? 1 : 0;
            }
        }

        SaveToFile(client_id_str, cj_filename->valuestring, content_str, append_mode);
    } else {
        printf("Unknown message type: %s\n", type_str);
        snprintf(responseMessage, sizeof(responseMessage), "Unknown message type");
    }

    cJSON_Delete(root);
}



void SaveToFile(const char* client_id, const char* filename, const char* content, int append) {
    // Create root folder 'data' if need
    mkdir("data", 0777);

    // Create an own folder for each client, where client is known by ip_port as their id
    char folder[256];
    snprintf(folder, sizeof(folder), "data/%s", client_id);
    mkdir(folder, 0777);

    // Create a filepath
    char filepath[300];
    snprintf(filepath, sizeof(filepath), "%s/%s", folder, filename);

    // Select is mode is append or clean and write new
    const char* mode = append ? "a" : "w";

    // write into the file
    FILE* fp = fopen(filepath, mode);
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
