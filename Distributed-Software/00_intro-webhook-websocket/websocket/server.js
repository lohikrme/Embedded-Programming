const websocket = require('ws');
const port = 8000;

const wss = new websocket.Server({ port: port });

// add word '-Chinese Parrot confirms' to end of message and send back
// websockets know if client is on or off because two way connection
// if client opens connection, this triggers
wss.on('connection', (socket) => {
    console.log('Client connected');
    // if client sends message, this triggers
    socket.on('message', (message) => {
        console.log(`Received "${message.toString()}"`);
        socket.send(`${message}\n-Chinese Parrot confirms`);
    });
    // if client turns off connection, this triggers
    socket.on('close', () => {
        console.log('Client disconnected');
    });
});
