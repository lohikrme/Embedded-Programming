// udp-echo-client.cpp
// g++ -std=c++14 udp-echo-client.cpp -o udp-echo-client

// u could also conditionally compile with -D tag 
// e.g g++ -std=c++14 -D LINUX udp-echo-client.cpp -o udp-echo-client
// but this software is not made to support it
// the condition we do is #if 0 or else

// FOr UDP echo client the parameters are
// -h = host ip address, -p = port, -m = message
// ./udp-echo-client -h ip -p 12700 -m "message"



#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <HalloweenWitch2.h>


#define DEFAULT_SERVER_PORT 8080
#define MAXSIZE 1000+32

// decide do u make software for linux or windows
// u can also give them in compiling with tag -D e.g -D LINUX
#define LINUX
// #define WINDOWS

// typedef allows defining own type
typedef struct _config {
    char *host = NULL;
    char *message = NULL;
    int port;
    int timeout = 10;
} Config_t;

// function prototypes mean order of writing functions doesn't matter
void GetCmdLineOptions( int argc, char *argv[], Config_t *pconfig);
int main(int argc, char *argv[]);
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
                pconfig-> message = optarg;
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
    Config_t config;
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

    // just send an array of characters (string) to server, server handles \0 etc
    sendto(
        sockid, 
        config.message, 
        strlen(config.message), 
        MSG_CONFIRM, (const sockaddr*) &serveraddr, 
        sizeof(serveraddr)
    );
    printf("Messageto %s:%d sent %s", config.host, config.port, config.message);

    if((n = recvfrom(sockid, in_buf, MAXSIZE-1, MSG_WAITALL, (sockaddr*) &serveraddr, (socklen_t *) &len)) < 0) {
        perror("recvfrom failed");
        exit(EXIT_FAILURE);
    }
    if (n == 0) {
        perror("Socket closed by server");
        exit(EXIT_FAILURE);
    }
    in_buf[n] = '\0';
    printf("Server returned: %s\n", in_buf);
    close(sockid);
    return 0;
}