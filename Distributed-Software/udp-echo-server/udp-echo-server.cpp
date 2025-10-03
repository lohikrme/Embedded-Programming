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

// function prototypes mean order of writing functions doesn't matter
void GetCmdLineOptions( int argc, char *argv[], Config_t *pconfig);
int main(int argc, char *argv[]);
void InterruptionHandler(int sig);
//------------------------------------------------------------------------

// GetCmdLineOptions() is a function, that reads given extra tags after calling the main software
// e.g ./udp-echo-server -p nnn would be summon of main software, and nnn would be data associated with -p tag
void GetCmdLineOptions( int argc, char *argv[], Config_t* pconfig) {
    int opt;
    // getopt searches for options with tags, such as "-p" and takes its value such as "nnn"
    while((opt=getopt(argc, argv, "p:")) != -1) {
        switch(opt) {
            case 'p':
                // mene osoittimen pconfig osoittamaan rakenteeseen ja käytä sen port-kenttää
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
    // save socket id into a variable, when socket is created, OS gives it a number
    int sockid;

    // address of server and client, sockaddr_in contains IP-address and port
    // usually sockaddr_in is used with AF_INET family, aka IPv4 internet addresses
    struct sockaddr_in serveraddr, clientaddr;

    // read at the start of invoking main function the parameters main is created with
    GetCmdLineOptions(argc, argv, &config);

    // now that we know what port we listen to (default or user input), we tell user we listen the port
    printf("udp-echo-server listening to port %d...\n Press CTRL+X to stop\n", config.port);

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

    // TODO code to use socket for something...

    // after done useful stuff with socket, close it
    close(sockid);
    return 0;
}


