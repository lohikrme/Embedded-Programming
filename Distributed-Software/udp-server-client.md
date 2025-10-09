# How to use udp server and client

### Docker containers

Both are supposed to run inside their own docker containers. U can turn on them both at once by making sure compose.yaml has the codes uncommented, and then by writing next commands:

-   docker compose down
-   docker compose up --build -d

The environmental variables make sure they have the correct mount bind folders. So inside both containers, there is a "c-projects" folder, that has straight access to windows folder with the names "udp-echo-server" and "udp-echo-client".

### Compiling server and client

The code is made to run on Linux. It means that it has some errors on Windows. That's why you should use either WSL or docker to compile it. So, go inside dockers with

-   'docker exec -it <container_name> /bin/bash'

And inside this command prompt view inside container, run next commands:

-Server: g++ -std=c++14 udp-echo-server.cpp -o udp-echo-server
-Client: g++ -std=c++14 udp-echo-client.cpp -o udp-echo-client

### Starting the server

The server is a code with endless while loop. so when it is started, it will stay on until stopped with CTRL+C. The client will meanwhile send a single message and turn off.

To turn on the server, it allows parameters:

-   -p = 'port to listen'
-   -d = 'delay before responding'.

Idea is that, we can artificially induce some delay, so we can try out delay-situations that could happen in a real distributed system. Perhaps client has a timeout after a few seconds or something.

The parameters are not mandatory for the server, so it can just be turned on, and it will then use default values (port 8080 and delay 0 seconds). But to use parameters, he is example:

-   ./udp-echo-server -p 12700 -d 5

### Using the client

To use the client, it can simply be turned on inside the command prompt (docker exec) view. So, it will deliver one message and receive the outcome, and after that, turn itself off. It is not a loop. Client can be given parameters:

-   -h = '(host's/server's) ip address to send messages'
-   -p = '(host's/server's) port to send messages'
-   -m = 'Content of the message, max 1000 bytes.
-   -t = 'Timeout and after time has passed assume udp packet is lost.
