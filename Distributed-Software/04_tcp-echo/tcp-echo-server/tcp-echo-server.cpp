// tcp-echp-server.cpp
// multithread example of tcp socket programming in C

// architecture:
// listening socket receives new connection requests, and create for them
// a new socket that handles the actual data transfer
// so e.g 10 clients, we have 10 child thread and 1 main thread to lsiten

// g++ -std=c++14 -pthread -g tcp-echo-server.cpp -o server

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
#include <pthread.h> // allow threading
// #include <atomic>

#define DEFAULT_SERVER_PORT 8080
#define MAXSIZE             1024

typedef struct _config {
    int port = DEFAULT_SERVER_PORT;
    int delay_in_secs = 0;
} Config_t;

// global vars
static int sockid = -1;

// global shared memory area (simulated, not real)
// use mutex lock to synchronize between child threads
static long total_message_counter = 0;
pthread_mutex_t lock;

// atomic is meant for things that prevent processor shutting down process until completed
// static std::atomic<long> msg_counter(0);

// function prototypes
static void GetCmdLineOptions(int argc, char *argv[], Config_t *pconfig);
int main(int argc, char *argv[]);
void * HandleClientConnection(void *socket_desc);
long TotalMessageCounter(long *plong, long add);
void InterruptionHandler(int sig);


// function implementations
static void GetCmdLineOptions(int argc, char *argv[], Config_t *pconfig) {
    static int opt;

    // ./server -p 12700 -d nn
    while((opt=getopt(argc, argv, "p:d:")) != -1) {
        switch(opt) {
        case 'p':
            pconfig->port = atoi(optarg);
            break;
        case 'd':
            pconfig->delay_in_secs = atoi(optarg);
            break;
        case '?':
            fprintf(stderr,"Unknown option %c\n", optopt);
            exit(EXIT_FAILURE);
            break;
        default:
            fprintf(stderr,"Usage: UDPEchoServer [-p port]\n");
        }
    }
}

int main(int argc, char *argv[]) {
    Config_t config;
    int len, n, new_socket, opt;
    int msg_cnt = 0;
    opt, new_socket = 1;
    
    struct sockaddr_in serveraddr, clientaddr;
    GetCmdLineOptions(argc, argv, &config);

    // init mutex, if !=0 it failed
    if(pthread_mutex_init(&lock, NULL) != 0) {
        perror("Mutex creation failed");
        exit(EXIT_FAILURE);
    }

    // Create LISTENING socket (stream)
    if((sockid=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // SETSOCKOPT() INPUT PARAMETERS
    // - sockid socket to act on
    // - level to act (generic/TCP/UDP/IPV4 etc)
    // - which option to set
    // - value of the option
    // - size of the option

    // notice that reuseaddr and reuseport are not concretic port numbers or ip addresses
    // but they are purely settings of the socket, how it behaves
    // so_reuseaddr allows restarting while some connection still in WAIT_STATE
    if (setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Socket option setting failed to set REUSE ADDRESS");
        exit(EXIT_FAILURE);
    }
    // while so_reuseport allows many threads to listen same port
    // needed to make load balancing in larger servers
    if (setsockopt(sockid, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Socket option setting failed to set REUSE PORT");
        exit(EXIT_FAILURE);
    }

    // memset resets a memory area, good for making sure no random data inside
    memset(&serveraddr, 0, sizeof(serveraddr));
    memset(&clientaddr, 0, sizeof(clientaddr));

    // FIll in server info
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    // htons transfers port into TCP/UDP compatible 32 bit number
    serveraddr.sin_port = htons(config.port);

    if (bind(sockid, (const sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        perror("socket bind failed");
        exit(EXIT_FAILURE);
    }

    // listening failed
    // params: socket id, length of queue how many connections allowed in queue
    if (listen(sockid, 3) < 0) {
        perror("socket bind failed");
        exit(EXIT_FAILURE);
    }

    printf("TCP Server listening on port %d... Press CNTL+C to stop\n", config.port);

    // start listening for Interruption signal and deliver those to InterruptionHandler
    signal(SIGINT, InterruptionHandler);

    // MAIN THREAD OF TCP SOCKET
    // WAITING FOR ARRIVING CONNECTIONS
    // IF CONNECTION ACCEPTED, CREATE A NEW THREAD FOR COMMUNICATION
    while(true) {
        // notice: if need save all client thread_ids, make another solution
        pthread_t thread_id;
        // sizeof can receive a variable with memory area, or a type such as sockaddr_in
        socklen_t client_address_len = sizeof(struct sockaddr_in);
        long global_message_counter;

        // ACCEPT CONNECTION REQUEST
        if ((new_socket=accept(sockid, (sockaddr *) &clientaddr, &client_address_len)) < 0) {
            perror("socket accept failed");
            exit(EXIT_FAILURE);
        }

        // CREATE A NEW THREAD FOR COMMUNICATION
        // &new_client_socket is the reference to client socket
        int *new_client_socket = (int *) malloc(sizeof(int));
        *new_client_socket = new_socket;
        if(pthread_create(&thread_id, NULL, HandleClientConnection, (void *) &new_client_socket) != 0) {
            perror("communication (worker) thread creation failed...");
            exit(EXIT_FAILURE);
        }
        // detach thread (no need to call join())
        // idea is if we shut down main thread, child threads might not shut down gracefully
        // but with detach, it makes child threads to clean up themselves after they completed
        pthread_detach(thread_id);

        // Here we use TotalMessageCounter to receive the amount of global messages so far sent
        global_message_counter = TotalMessageCounter(&total_message_counter, 0);
        printf("Connection from %s:%d handled to thread\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);
    }
    // close listening socket
    if(sockid != -1) {
        shutdown(sockid, SHUT_RDWR);
        close(sockid);
        // -1 makes sure it is closed
        sockid = -1;
    }
    return 0;
}

// FUNCTION USED BY CHILD THREAD TO HANDLE TCP CONNECTION
// basically this can be thought as things happening inside a new thread
void *HandleClientConnection(void *socket_desc) {
    // place of place where int * directs us
    // TODO: INVENT BETTER NAME
    int socket_ = *(int *) socket_desc;
    int n, loop_count = 0;
    // leave +1 size for \0
    char in_buf[MAXSIZE+1] = {0};
    char out_buf[MAXSIZE+30] = {0};

    // handle op until client closes connection
    // if n returns positive, there is data
    while((n=recv(socket_, in_buf, MAXSIZE, 0)) > 0) {
        loop_count++;
        // use self-made TotalMessageCounter() to add
        TotalMessageCounter(&total_message_counter, 1);
        in_buf[n] = '\0';
        sprintf(out_buf, "Echo [%d]: %s\n", loop_count, in_buf);
        // send response to client
        send(socket_, out_buf, strlen(out_buf), 0);
        // clean out_buf memory location so new stuff can be sent cleanly
        memset(out_buf, 0, MAXSIZE);
    }


    // if n returns 0, client wants to end socket stream
    if (n == 0) {
        printf("Client closed the connection... ending thread after %d replys\n", loop_count);
        // make sure no memory residuals left
        fflush(stdout);
    } 
    // n values under 0 are aerror
    else if (n < 0) {
        perror("socket thread recv failed");
    }
    // close connection to tcp socket
    close(socket_);
    // remember to clean up the socket description from memory
    // socket description was received as parameter, so it cleans up the new_client_sock from main program
    free(socket_desc);
    return 0;
}

// use mutex lock to increase global variable that
long TotalMessageCounter(long *plong, long add=1) {
    long value_after;
    // lock shared memory for other threads
    pthread_mutex_lock(&lock);
    *plong += add;
    value_after = *plong;
    // completed processing of memory area, unlock for other threads
    pthread_mutex_unlock(&lock);
    return value_after;
}

void InterruptionHandler(int sig) {
    char c;
    signal(sig, SIG_IGN);
    printf("\nDo you really want to quit? [y/n] ");
    c = getchar();
    if(toupper(c)=='Y') {
        if(sockid >= 0) {
            close(sockid);
            sockid = -1;
        }
        exit(0);
    } else {
        signal(SIGINT, InterruptionHandler);
    }
}