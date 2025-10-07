# link to wokwi project:
# https://wokwi.com/projects/444049566279506945

# needed components:
# rasberry pi pico wifi
# DHT 22 sensor (temperature, moisture)

# Koodi Wi-fi yhteyden testaamiseen:

# Libraries
#------------------------------
# For Wi-Fi connectivity
import network       
# For delays and timing
import time          
# For making HTTP requests
import urequests     
# For interfacing with DHT sensors
import dht           
# For controlling GPIO pins
from machine import Pin  



# Wi-Fi credentials
#------------------------------
# SSID of the Wi-Fi network
ssid = 'Wokwi-GUEST'     
# Password (empty for open networks like Wokwi-GUEST)
password = ''            



# ThingSpeak API configuration
#------------------------------
# My ThingSpeak Write API Key
THINGSPEAK_API_KEY = 'O62AI7VFMMTZETPY'  
# ThingSpeak endpoint
THINGSPEAK_URL = 'https://api.thingspeak.com/update'  



# Set up Wi-Fi in station mode
#------------------------------
# Create a WLAN object in station mode, 
# the device connects to a Wi-Fi network as a client. 
wlan = network.WLAN(network.STA_IF)  
# Activate the Wi-Fi interface
wlan.active(True)                    
# Connect to the specified Wi-Fi network
wlan.connect(ssid, password)         



# Wait until connected
#------------------------------
print("Connecting to Wi-Fi...", end="")
while not wlan.isconnected():
    # Print dots while waiting
    print(".", end="")               
    # Wait half a second before retrying
    time.sleep(0.5)                  



# Once connected, print confirmation and IP address
#------------------------------
print("\nConnected!")
# Display the assigned IP address
print("IP address:", wlan.ifconfig()[0])  



# Initialize the DHT22 sensor on GPIO pin 15
#------------------------------
sensor = dht.DHT22(Pin(15))



# Function to send temp/humid data to ThingSpeak
#------------------------------
def send_to_thingspeak(temp, humid):
    if temp is None or humid is None:
        print("Temperature and/or Humidity data missing and cannot be sent.")
        return
    print(temp, humid)
    try:
        # Send HTTP POST request to ThingSpeak with data
        response = urequests.post(
            THINGSPEAK_URL,
            data='api_key={}&field1={}&field2={}'
                .format(THINGSPEAK_API_KEY, temp, humid),
            headers={'Content-Type': 'application/x-www-form-urlencoded'}
        )

        print("ThingSpeak response:", response.text)  # Print server response
        response.close()  # Close the connection
    except Exception as e:
        print("Failed to send data:", e)  # Handle any errors



# Main loop: read sensor and send data every 15 seconds
#------------------------------
while True:
    try:
        # Trigger sensor measurement
        sensor.measure()                      
        # Read temperature and humidity
        temperature = sensor.temperature()    
        humidity = sensor.humidity()
        # Display temperature
        print("Temperature:", temperature, "°C")  
        print("Humidity:", humidity, "%")
        # Send data to ThingSpeak
        send_to_thingspeak(temperature, humidity)       
    except Exception as e:
        print("Error reading sensor or sending data:", e)  # Handle errors

    time.sleep(15)  # Wait 15 seconds before next reading
