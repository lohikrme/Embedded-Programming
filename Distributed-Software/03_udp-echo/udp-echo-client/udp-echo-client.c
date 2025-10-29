// udp-echo-client.cpp
// g++ -std=c++14 udp-echo-client.cpp -o udp-echo-client

// u could also conditionally compile with -D tag 
// e.g g++ -std=c++14 -D LINUX udp-echo-client.cpp -o udp-echo-client
// but this software is not made to support it
// the condition we do is #if 0 or else

// FOr UDP echo client the parameters are
// -h = host ip address, -p = port, -d = data (message or file path)
// ./udp-echo-client -h ip -p 12700 -d "message"

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
#include <time.h>
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
    char *append;
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
    while((opt=getopt(argc, argv, "h:p:d:t:a:")) != -1) {
        switch(opt) {
            case 'h':
                pconfig->host = optarg;
                break;
            case 'p':
                // mene osoittimen pconfig osoittamaan rakenteeseen ja käytä sen port-kenttää
                pconfig->port = atoi(optarg);
                break;
            case 'd':
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
                // deep copy a string with strdup - remember in main clean the memory because deep copy
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
            case 'a':
                // make sure it is either "new" or "append",
                // otherwise replace all other given options with "append"
                if (optarg && (strcmp(optarg, "new") == 0 || strcmp(optarg, "append") == 0)) {
                    pconfig->append = optarg;
                } else {
                    fprintf(stderr, "Invalid append mode '%s', defaulting to 'append'\n", optarg);
                    pconfig->append = "append";
                }
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
    config.append = "append";

    // save socket id into a variable, when socket is created, OS gives it a number
    int sockid, len, n;
    sockid = len = n = 0;
    struct sockaddr_in serveraddr;
    char in_buf[MAXSIZE];

    // read at the start of invoking main function the parameters main is created with
    // notice that getcmdlineoptions gets config as straight reference to original values
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

    // timeval struct contains time_t seconds and time_usec milloseconds
    struct timeval tv;
    tv.tv_sec = config.timeout;
    tv.tv_usec = 0;

    if (setsockopt(sockid, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error in setting the timeout for recvfrom!");
    }

    // 1. If message is too large, enforce a soft limit to avoid surprises
    const size_t MAX_ALLOWED = 100000; // safety cap for message processing
    size_t total_len = config.message ? strlen(config.message) : 0;
    if (total_len > MAX_ALLOWED) {
        fprintf(stderr, "Message too large (%zu bytes), limiting to %zu bytes\n", total_len, MAX_ALLOWED);
        total_len = MAX_ALLOWED;
    }

    // 2. Prepare to measure time (start before first send)
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // 3. If file mode, send in simple chunks; otherwise send single message
    if (config.is_file) {
        const size_t CHUNK_SIZE = 500; // tweak as needed (keep safe for UDP)
        size_t offset = 0;
        int part = 0;

        // A. Loop until we've sent total_len bytes
        // notice that if we are e.g 300 left and chunk size 500, we cannot send 500 bytes data
        // or the last 200 are random content from pc ram, 
        // thats why have this compact if else chunk_size or remaining
        while (offset < total_len) {
            size_t remaining = total_len - offset;
            size_t this_len = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;

            // Copy this chunk into a temporary NUL-terminated buffer
            // memcpy is called 'copy memory block' function
            // basically from message-string to tmp-string it copiesdata
            // it takes chunk_size or remaining amount data depending which one smaller
            char tmp[this_len + 1];
            memcpy(tmp, config.message + offset, this_len);
            tmp[this_len] = '\0';

            // Escape JSON special characters for this chunk using self-made function
            char *escaped_chunk = EscapeJSONString(tmp);
            if (!escaped_chunk) {
                fprintf(stderr, "EscapeJSONString failed\n");
                close(sockid);
                return 1;
            }

            // Build JSON for this chunk
            // notice that, if append config is "new" from client starting parameters
            // the first time u send first packed, use "new", but after that always "append"
            char json_buf[MAXSIZE];
            int njson = snprintf(json_buf, sizeof(json_buf),
                "{ \"client_id\": \"%s\", \"type\": \"file\", \"filename\": \"%s\", \"content\": \"%s\", \"append\": \"%s\" }",
                CLIENT_ID, config.filename ? config.filename : "unnamed", escaped_chunk, config.append);

            free(escaped_chunk); // free escaped memory immediately

            // if append mode was "new", replace append mode to be "append" from now on
            if (config.append && strcmp(config.append, "new") == 0) {
                config.append = "append";
            }

            if (njson < 0 || (size_t)njson >= sizeof(json_buf)) {
                fprintf(stderr, "JSON buffer overflow for part %d\n", part);
                close(sockid);
                return 1;
            }

            // Send this chunk
            ssize_t sent = sendto(sockid, json_buf, (size_t)njson, 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
            if (sent < 0) {
                perror("sendto failed");
                close(sockid);
                return 1;
            }

            printf("Sent part %d (%zu bytes)\n", part + 1, this_len);

            offset += this_len;
            part++;

            // small optional sleep to reduce burstiness
            usleep(5 * 1000); // 5 ms
        }

        // After sending all parts, wait once for final server reply
        socklen_t addrlen = sizeof(serveraddr);
        ssize_t rn = recvfrom(sockid, in_buf, MAXSIZE - 1, 0, (struct sockaddr*)&serveraddr, &addrlen);
        if (rn < 0) {
            perror("recvfrom failed");
            close(sockid);
            // still measure end time below before returning
        } else {
            in_buf[rn] = '\0';
            printf("\n\n\nServer returned: %s\n", in_buf);
        }
    } else {
        // Single-message path (escape whole message and send once)
        char *escaped = EscapeJSONString(config.message);
        if (!escaped) {
            fprintf(stderr, "EscapeJSONString failed\n");
            close(sockid);
            return 1;
        }

        char json_buf[MAXSIZE];
        int njson = snprintf(json_buf, sizeof(json_buf),
            "{ \"client_id\": \"%s\", \"type\": \"message\", \"content\": \"%s\" }",
            CLIENT_ID, escaped);
        free(escaped);

        if (njson < 0 || (size_t)njson >= sizeof(json_buf)) {
            fprintf(stderr, "JSON buffer overflow\n");
            close(sockid);
            return 1;
        }

        ssize_t sent = sendto(sockid, json_buf, (size_t)njson, 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
        if (sent < 0) {
            perror("sendto failed");
            close(sockid);
            return 1;
        }
        printf("Message: '%s' has been sent to %s:%d\n", config.message, config.host, config.port);

        // wait for single reply
        socklen_t addrlen = sizeof(serveraddr);
        ssize_t rn = recvfrom(sockid, in_buf, MAXSIZE - 1, 0, (struct sockaddr*)&serveraddr, &addrlen);
        if (rn < 0) {
            perror("recvfrom failed");
            close(sockid);
            // still measure end time below
        } else {
            in_buf[rn] = '\0';
            printf("\n\n\nServer returned: %s\n", in_buf);
        }
    }

    // 4. Close socket and stop timer
    close(sockid);
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // 5. Compute elapsed time
    long seconds = end_time.tv_sec - start_time.tv_sec;
    long nanoseconds = end_time.tv_nsec - start_time.tv_nsec;
    double elapsed = seconds + nanoseconds / 1e9;

    // 6. Print elapsed
    printf("⏱️ Transfer duration: %.3f seconds\n", elapsed);

    // clean memory area of config and return main succesfully
    if (config.filename) free(config.filename);
    return 0;

} // MAIN ENDS

// use this function to make sure, most common special letters do not corrupt the json message that will be sent
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
