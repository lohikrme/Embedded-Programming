/*udp-echo-server.cpp
$ g++ -std=c++14 udp-echo-server.cpp -o udp-echo-server
*/

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

// typedef allows defining own type
#define DEFAULT_SERVER_PORT 8080
typedef struct _config {
    int port;
} Config_t;

// function prototypes
void GetCmdLineOptions( int argc, char *argv[], Config_t *pconfig);
int main(int argc, char *argv[]);
void IntHandler(int sig);
//------------------------------------------------------------------------

// function implementations
void GetCmdLineOptions( int argc, char *argv[], Config_t *pconfig) {
    int opt;
    // ./udp-echo-server -p nnn
    while((opt=getopt(argc, argv, "p:")) != -1) {
        switch(opt) {
            case 'p':
                pconfig->port = atoi(optarg);
                break;
            case '?':
            fprintf(stderr, "Unknown option %c\n", optopt);
                break;
            default:
                fprintf(stderr, "Usage: udp-echo-server [-p port\n]");
        }
    }
}


int main(int argc, char *argv[]) {
    Config_t config;
    int sockid;

    struct sockaddr_in serveraddr, clientaddr;

    GetCmdLineOptions(argc, argv, &config);

    printf("udp-echo-server listening to port %d...\n Press CTRL+X to stop\n", config.port);

    signal(SIGINT, IntHandler);

    if ((sockid=socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&serveraddr, 0, sizeof(serveraddr));
    memset(&serveraddr, 0, sizeof(clientaddr));
    close(sockid);
    return 0;
}


// make this sub function work even coding after main
// sigint is a control + c handler?
// sig_ign = ignore control handler who otherwise would close application
void IntHandler(int sig) {
    char c;
    signal(sig, SIG_IGN);
    printf("\nDo you really want to quit [y/n]?");
    c = getchar();
    if(toupper(c)=='Y') {
        exit(0);
    } else {
        signal(SIGINT, IntHandler);
    }
}