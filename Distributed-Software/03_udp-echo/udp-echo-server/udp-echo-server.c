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


#define DEFAULT_SERVER_PORT 8080
#define MAXSIZE 1000

// typedef allows defining own type
// delay in secs is used for artificial delay in response
typedef struct _config {
    int port;
    int delay_in_secs;
} Config_t;

// global variables
static int sockid = -1;

// function prototypes mean order of writing functions doesn't matter
void GetCmdLineOptions( int argc, char *argv[], Config_t *pconfig);
void InterruptionHandler(int sig);
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
        if((n = recvfrom(sockid, in_buf, MAXSIZE-1, MSG_WAITALL, (struct sockaddr*) &clientaddr, (socklen_t*)&len)) < 0) {
            perror("recvfrom failed");
            exit(EXIT_FAILURE);
        }
        // make sure string ends to the null
        in_buf[n] = '\0';
        msg_cnt++;
        printf("Client req (%d) received from %s using port %u: %s...", 
            msg_cnt, 
            // inet_ntoa transforms binary IP to human IP
            inet_ntoa(clientaddr.sin_addr),
            clientaddr.sin_port,
            in_buf
        );
        // fprintf(file_fp, "", ...)
        // printf() = fprintf(stdout, "", ...)
        // use sprintf to print straight into a buffer or file
        // 9 letters + digit so +20 maxsize for out_buf enough to prevent overflow

        // add additional delay to response
        if (config.delay_in_secs) {
            sleep(config.delay_in_secs);
        }
        // saves text into a string with sprintf into out_buf memory
        sprintf(out_buf, "Echo [%d]: %s", msg_cnt, in_buf);

        // next step is decide flag where message goes
        sendto(sockid, out_buf, strlen(out_buf), MSG_CONFIRM, (struct sockaddr*)&clientaddr, len);
        printf("reply sent\n");
    }

    // after done useful stuff with socket, close it
    if (sockid > 0) {
        close(sockid);
        sockid = -1;
    }
    
    return 0;
}


