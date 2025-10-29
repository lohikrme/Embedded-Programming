// udp-echo-client.cpp
// g++ -std=c++14 udp-echo-client.cpp -o udp-echo-client

// u could also conditionally compile with -D tag 
// e.g g++ -std=c++14 -D LINUX udp-echo-client.cpp -o udp-echo-client
// but this software is not made to support it
// the condition we do is #if 0 or else

// FOr UDP echo client the parameters are
// -h = host ip address, -p = port, -m = message
// ./udp-echo-client -h ip -p 12700 -m "message"

// IMPORTANT: optarg is a global variable used in C command prompt softwares


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
//#include <HalloweenWitch2.h>


#define DEFAULT_SERVER_PORT 8080
#define MAXSIZE 1000+32
#define CLIENT_ID "client_halloweenWitch"

// decide do u make software for linux or windows
// u can also give them in compiling with tag -D e.g -D LINUX
#define LINUX
// #define WINDOWS

// typedef allows defining own type
typedef struct _config {
    char *host;
    char *message;
    char *filename;
    int port;
    int timeout;
    int is_file;
} Config_t;

// function prototypes mean order of writing functions doesn't matter
void GetCmdLineOptions( int argc, char *argv[], Config_t *pconfig);
int main(int argc, char *argv[]);
char* EscapeJSONString(const char* input);
//------------------------------------------------------------------------

// GetCmdLineOptions() is a function, that reads given extra tags after calling the main software
// e.g ./udp-echo-server -p nnn would be summon of main software, and nnn would be data associated with -p tag

void GetCmdLineOptions( int argc, char *argv[], Config_t* pconfig) {
    int opt;
    // getopt searches for options with tags, such as "-p" and takes its value such as "nnn"
    // -t tag = timeout
    while((opt=getopt(argc, argv, "h:p:m:t:")) != -1) {
        switch(opt) {
            case 'h':
                pconfig->host = optarg;
                break;
            case 'p':
                // mene osoittimen pconfig osoittamaan rakenteeseen ja käytä sen port-kenttää
                pconfig->port = atoi(optarg);
                break;
            case 'm':
                //----------------------------
                // FILE DATA
                if (optarg[0] == '@') {
                    // skip '@' so filepath is functional
                    const char* filepath = optarg + 1;
                    FILE *fp = fopen(filepath, "r");
                    // file is not found
                    if (fp == NULL) {
                        perror("Failed to open file");
                        exit(EXIT_FAILURE);
                    }
                    // find out the size of file, and then go to beginning of file
                    fseek(fp, 0, SEEK_END);
                    long filesize = ftell(fp);
                    rewind(fp);

                // allocate memory for content of file
                pconfig->message = malloc(filesize + 1);
                if (!pconfig->message) {
                    perror("malloc to store file content failed");
                    fclose(fp);
                    exit(EXIT_FAILURE);
                }
                
                // read whole file content into pconfig.message:
                size_t read = fread(pconfig->message, 1, filesize, fp);
                // add \0 to end of file to computers know string ends
                pconfig->message[read] = '\0';
                fclose(fp);
                // deep copy a string with strdup
                const char* filename = filepath + 7;
                pconfig->filename = strdup(filename);
                pconfig->is_file = 1;

                }
                //----------------------------
                // PLAIN MESSAGE DATA
                else {
                    pconfig-> message = optarg;
                    pconfig->filename = NULL;
                    pconfig->is_file = 0;
                }
                break;
            case 't':
                pconfig -> timeout = atoi(optarg);
                break;
            case '?':
            fprintf(stderr, "Unknown option %c\n", optopt);
                exit(EXIT_FAILURE);
                break;
            default:
                fprintf(stderr, "Usage: udp-echo-client -h host -p port -m message\n");
        }
    }
}


int main(int argc, char *argv[]) {
    // make a variable called config that is type of Config_t
    // also make default values
    Config_t config;
    config.host = "127.20.0.2";
    config.message = "Hello world!";
    config.port = 12000;
    config.timeout = 10;

    // save socket id into a variable, when socket is created, OS gives it a number
    int sockid, len, n;
    sockid = len = n = 0;
    struct sockaddr_in serveraddr;
    char in_buf[MAXSIZE];

    // read at the start of invoking main function the parameters main is created with
    GetCmdLineOptions(argc, argv, &config);

    if(config.message == NULL) {
        perror("No message to send");
        exit(EXIT_FAILURE);
    }

    if ((sockid=socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        // if problem, print error
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serveraddr, 0, sizeof(serveraddr));

// conditional compiling
#if 0
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(config.port);
    serveraddr.sin_addr.s_addr = inet_addr(config.host);    
#else 
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(config.port);

    struct hostent *h;
    struct in_addr* *addrList;
    char host_ip[60];

    if( (h=gethostbyname(config.host)) == NULL) {
        printf("gethostbyname error... using original\n");
        // copy from config ip to host_ip
        strcpy(host_ip, config.host);
    } 
    else {
        addrList = (struct in_addr**) h->h_addr_list;
        // inet_ntoa() converts IP address from binary to human form
        strcpy(host_ip, inet_ntoa(*addrList[0]));
    }
    if (inet_pton(AF_INET, host_ip, &serveraddr.sin_addr.s_addr) <= 0) {
        perror("Invalid IP-address");
        exit(EXIT_FAILURE);
    }
    printf("host %s -> host-ip %s\n", config.host, host_ip);
#endif

    struct timeval tv;
    tv.tv_sec = config.timeout;
    tv.tv_usec = 0;

    if (setsockopt(sockid, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error in setting the timeout for recvfrom!");
    }

    // 1. Build a json message
    char json_buf[MAXSIZE];
    char* escaped = EscapeJSONString(config.message);

    if (config.is_file) {
        snprintf(json_buf, sizeof(json_buf),
            "{ \"client_id\": \"%s\", \"type\": \"file\", \"filename\": \"%s\", \"content\": \"%s\", \"append\": true }",
            CLIENT_ID ,config.filename, escaped);
    } else {
        snprintf(json_buf, sizeof(json_buf),
            "{ \"client_id\": \"%s\", \"type\": \"message\", \"content\": \"%s\" }",
            CLIENT_ID, escaped);
    }

    free(escaped); // vapautetaan muisti

    // 2. send json server, no need to add 0/ here
    printf("Lähetettävä JSON:\n%s\n", json_buf);
    sendto(
        sockid, 
        json_buf,
        strlen(json_buf), 
        MSG_CONFIRM, 
        (struct sockaddr*) &serveraddr, 
        sizeof(serveraddr)
    );
    printf("Message: \n\n\n'%s' \n\n\n has been sent to %s:%d\n", config.message, config.host, config.port);

    if((n = recvfrom(sockid, in_buf, MAXSIZE-1, MSG_WAITALL, (struct sockaddr*) &serveraddr, (socklen_t *) &len)) < 0) {
        perror("recvfrom failed");
        exit(EXIT_FAILURE);
    }
    if (n == 0) {
        perror("Socket closed by server");
        exit(EXIT_FAILURE);
    }
    in_buf[n] = '\0';
    printf("\n\n\nServer returned: %s\n", in_buf);
    close(sockid);
    return 0;
}

char* EscapeJSONString(const char* input) {
    size_t len = strlen(input);
    char* output = malloc(len * 2 + 1); // varaa reilusti tilaa
    char* p = output;

    for (size_t i = 0; i < len; i++) {
        switch (input[i]) {
            case '\"': *p++ = '\\'; *p++ = '\"'; break;
            case '\\': *p++ = '\\'; *p++ = '\\'; break;
            case '\n': *p++ = '\\'; *p++ = 'n'; break;
            case '\r': *p++ = '\\'; *p++ = 'r'; break;
            case '\t': *p++ = '\\'; *p++ = 't'; break;
            default: *p++ = input[i]; break;
        }
    }
    *p = '\0';
    return output;
}
