# Basic idea of entire project

The project is training for iot applications of the industry.
The data will be sent and visualized with nodered low code application.
Mqtt will be used to deliver data from iot machines to

## structure of folders

defs contains all needed dockerfiles.
other folders are to be used as mount bind

## super simple mqtt guide

turn on mosquitto container into 2 windows.
on one of the windows run command so subscribe, on other to publish

-   example commands to send mqtt messages
    to subscribe aka wait for mqtt messages:
-   mosquitto_sub -h iots_2025s-mosquitto-1 -t "iots_2025/TT10/kitchen"
    to publish aka send mqtt message:
-   mosquitto_pub -h mosquitto -t "iots_2025/TT10/kitchen" -m '{"address": "TT10", "loc":"kitchen","
    device":"iots_2025_node_red_conteiner", "temperature":32, "pressure":945 , "humidity":84}'

-   When nodered generates data, it has next contents:
-   const org = 'iots_2025';
-   const addr = 'Heinola';
-   const loc = 'Forest_SE2';
-   let device = 'Pico1';

So when you need to use MQTT to send data to telegraf, use next form:

-   "iots_2025/Heinola/Forest_SE2"

To see inside mosquitto container, does mqtt receive data from nodered, use:

-   mosquitto_sub -h localhost -t "iots_2025/Heinola/Forest_SE2"

## INFLUX Credentials

-   Username: lohikrme
-   Salasana: Koodaus1
-   Organisaatio: Lab University
-   Bucket name: iots_2025s_test_data
-   Token:
    <p>vS0HVFGmuus2bTK1HwvYBwBiBzTyCvCaSsmzJbBTk2IZJsRiacPEScUFfUxttU1UP0jPMIV3l7jOdyGjOkggUg==<p>

## Grafana

### First init

First time enter with username: admin password: admin
Set new password: Koodaus1

Then go to connections - select influxdb - select Query language flux because influxdb 2.0.

Set url: http://iots_2025s-influxdb2-1:8086

Security settings: Basic auth and With Credentialshttp://iots_2025s-influxdb2-1:8086

Basic Auth details:Username and password from Influxdb

Influx credentials: Other influx stuff from this readme above.

Note mqtt write code:

let data = msg.payload;
const org = 'iots_2025s';
const addr = 'TT10';
const loc = 'kitchen';

msg.payload =
'{"address":"' + data.address +
'","loc":"' + data.loc +
'","device":"' + data.device +
'","temperature":' + data.temperature.toFixed(2) +
', "pressure":' + data.pressure.toFixed(2) +
', "humidity":' + data.humidity.toFixed(2) + '}';

msg.topic = `${org}/${addr}/${loc}`;
return msg;
