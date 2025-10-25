# About the course

Distributed software course tries to teach how udp and tcp sockets and mqtt protocol work. Idea is to stay slightly on the realm of embedded programming, so more like process communicating to another process on linux etc, rather than something done purely between web servers. The language will be C++.

The tasks during the course split to next topics:

1. Makefile assignment, to get basic understanding about compiling larger C++ project is done professionally.
2. UDP Echo assignment, to teach how udp socket works. UDP sockets are not stream but packet based.
3. TCP Echo assignment, to teach how tcp socket works. TCP sockets are stream based, so first connection but be verified.
4. TODO: add when know

## Task 1: Makefile assignment

### Docker containers

Create one docker container from dockerfiles / docker-compose.yaml, with profile "makefile". It is based on "server" dockerfile, though in reality it does not need all the packets server container has, server file installs for example the make software to compile C++.

### Main Points

The code is very simple itself. The main point of assignment is the makefile, that shows example how to professionally (in linux environment) compile C++ code from multiple files.

## Task 2: UDP Echo

### Docker containers

Both are supposed to run inside their own docker containers. U can turn on them both at once by making sure compose.yaml has the codes uncommented, and then by writing next commands:

-   docker compose down
-   docker compose up --build -d

The environmental variables make sure they have the correct mount bind folders. So inside both containers, there is a "c-projects" folder, that has straight access to windows folder with the names "udp-echo-server" and "udp-echo-client", to access the source code of host pc.

### Compiling udp server and udp client

The code is made to run on Linux. It means that it has some errors on Windows. That's why you compile it inside the container. So, go inside dockers with (one for server, one for client):

-   'docker exec -it <container_name> /bin/bash'

And run next commands:

-Server: g++ -std=c++14 udp-echo-server.cpp -o udp-echo-server
-Client: g++ -std=c++14 udp-echo-client.cpp -o udp-echo-client

### Starting the udp server

The server is a code with endless while loop. so when it is started, it will stay on until stopped with CTRL+C. The client will meanwhile send a single message and turn off.

To turn on the server, it allows parameters:

-   -p = 'port to listen'
-   -d = 'delay before responding'.

Idea is that, we can artificially induce some delay, so we can try out delay-situations that could happen in a real distributed system. Perhaps client has a timeout after a few seconds or something.

The parameters are not mandatory for the server, so it can just be turned on, and it will then use default values (port 8080 and delay 0 seconds). But to use parameters, he is example:

-   ./udp-echo-server -p 12700 -d 5

### Using the udp client

To use the client, it can simply be turned on inside the command prompt (docker exec) view. So, it will deliver one message and receive the outcome, and after that, turn itself off. It is not a loop. Client can be given parameters:

-   -h = '(host's/server's) ip address to send messages'
-   -p = '(host's/server's) port to send messages'
-   -m = 'Content of the message, max 1000 bytes.
-   -t = 'Timeout and after time has passed assume udp packet is lost.
