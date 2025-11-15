// tcp-echo-client.cpp

// g++ -std=c++14 -pthread -g tcp-echo-client.cpp -o client

// ./client -h ds-2025-student-tcp_server-1 -p 8000 -m "Hello TCP Server"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cassert>

#define DEFAULT_SERVER_PORT 8080
#define MAXLEN             (1024)

typedef struct _config {
    char *host = NULL;
    char *message = NULL;
    int port = DEFAULT_SERVER_PORT;
    int timeout = 10;
    int loop_cnt = 1;
} Config_t;

// function prototypes
void GetCmdLineOptions(int argc, char *argv[], Config_t *pconfig);
int main(int argc, char *argv[]);
// void INTHandler(int sig);

// function implementations
void GetCmdLineOptions(int argc, char *argv[], Config_t *pconfig) {
    int opt;

    // ./client -h ip -p 12700 -t nn -m "message" -c nn
    while((opt=getopt(argc, argv, "h:p:m:t:c:")) != -1) {
        switch(opt) {
        case 'h':
            pconfig->host = optarg;
            break;
        case 'm':
            pconfig->message = optarg;
            break;
        case 'p':
            pconfig->port = atoi(optarg);
            break;
        case 't':
            pconfig->timeout = atoi(optarg);
            break;
        case 'c':
            pconfig->loop_cnt = atoi(optarg);
            break;
        case '?':
            fprintf(stderr,"Unknown option %c\n", optopt);
            exit(EXIT_FAILURE);
            break;
        default:
            fprintf(stderr,"Usage: UDPEchoClient -h host -p port -m message\n");
        }
    }
}


int main(int argc, char *argv[]) {
    Config_t config;
    int sockid, client_fd;
    int slen, n;
    struct sockaddr_in serveraddr;
    char in_buf[MAXLEN];

    GetCmdLineOptions(argc, argv, &config);

    if(config.message == NULL) {
      perror("No message to send");
      exit(EXIT_FAILURE);
    }

    if((sockid=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&serveraddr, 0, sizeof(serveraddr));

#if 0
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(config.port);
    serveraddr.sin_addr.s_addr = inet_addr(config.host);
#else
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(config.port);

    struct hostent *h;
    struct in_addr **addrList;
    char host_ip[60];


    if((h=gethostbyname(config.host)) == NULL) {
        printf("gethostbyname error...using original\n");
        strcpy(host_ip, config.host);
    } else {
        addrList = (struct in_addr**)h->h_addr_list;
        strcpy(host_ip, inet_ntoa(*addrList[0]));
    }
    if(inet_pton(AF_INET, host_ip, &serveraddr.sin_addr) <= 0 ) {
      perror("Invalid address");
      exit(EXIT_FAILURE);
    }
    printf("host %s -> host-ip %s\n", config.host, host_ip);
#endif

#if 0
    struct timeval tv;
    tv.tv_sec = config.timeout;
    tv.tv_usec = 0;

    if(setsockopt(sockid, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
      perror("Error in setting timeout for recvfrom");
    }
#endif

    // make connection
    if ((client_fd = connect(sockid, (const sockaddr*)&serveraddr, sizeof(serveraddr))) < 0) {
        perror("Connection refused");
        exit(EXIT_FAILURE);
    }

    const int msg_len = strlen(config.message);
    for (int i= 0; i < config.loop_cnt; i++) {
        // send data now that connection is established
        // params: receiver socket id, data, flags special options 0, byte count transmitted
        slen = send(sockid, config.message, msg_len, 0);

        // todo: check return value, if less than 0 failed
        // with assert give condition that is always true
#if 0
        // option A:
        // if condition is untrue, it will terminate software and notify which assert caused it
        assert(slen == msg_len);
#endif
#if 1
        // option B:
        if(slen != msg_len) {
            printf("slen %d != msg_len %d\n", slen, msg_len);
        }
#endif

        // try to receive response
        if((n = recv(sockid, in_buf, MAXLEN, 0)) < 0) {
            perror("Receive failed");
            exit(EXIT_FAILURE);
        };
        if (n > 0) {
            in_buf[n] = '\0';
            if ((i%1000) == 999) {
                printf("Server replied (%d): %s", n, in_buf);
            }
        } else {
                printf("recv returned 0 => other end closed the connection");
            break;
        }
        
    }
    // close tcp connection
    close(sockid);
    return 0;
}