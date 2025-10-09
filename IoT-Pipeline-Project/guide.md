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

## INFLUX Credentials

-   Username: lohikrme
-   Salasana: Koodaus1
-   Organisaatio: Lab University
-   Bucket name: iots_2025s_test_data
-   Token:
    <p>vS0HVFGmuus2bTK1HwvYBwBiBzTyCvCaSsmzJbBTk2IZJsRiacPEScUFfUxttU1UP0jPMIV3l7jOdyGjOkggUg==<p>
