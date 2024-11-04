#!/usr/local/bin/python

import RPi.GPIO as GPIO
import time
import datetime
import sqlite3
import paho.mqtt.client as mqtt

# photoresistor_detector.py
# This file reads in the data coming in from the photoresistor and
# makes decisions on calling on the Zigbee hub to activate the 
# bed shaker and the room light switch or light bulb as required.
# For debugging purposes, it also saves the photoresistor values
# into a database which will be loaded up into the basic 
# frontend, letting me know why things are breaking.

# Setting the mode of the GPIO to GPIO.board?
GPIO.setmode(GPIO.BOARD)

# Define the GPIO pin that is the circuit output
pin_to_circuit = 7

# rc_time() calls sends a quick
# pulse through the circuit output pin.
# This charges up the capacitor. Then,
# once the capacitor is charged up, the pulse is stopped
# Then that pin is changed to be an input pin. Thus,
# now the capacitor begins to discharge with it's only path
# being through the LDR.
# Based on the value of the resistance of the LDR,
# the time taken/the count required for the 
# capacitor to fully discharge is either very fast
# aka, there is a lot of light on the LDR, thus the resistance is low.
# Or the time taken/the count required for the capacitor to fully dicharge is
# very slow, thus little light on the LDR, thus resistance is higher.
def rc_time (pin_to_circuit):
    count = 0
  
    # Output a quick pulse on the circuit output
    GPIO.setup(pin_to_circuit, GPIO.OUT)
    GPIO.output(pin_to_circuit, GPIO.LOW)
    time.sleep(0.1)

    # Change the pin back to input
    GPIO.setup(pin_to_circuit, GPIO.IN)
  
    # Count until the pin goes high
    while (GPIO.input(pin_to_circuit) == GPIO.LOW):
        count += 1

    return count

def get_sleep_duration(current_time):
    # Get the current hour, minute, and second
    current_hour = current_time.hour
    current_minute = current_time.minute
    current_second = current_time.second

    # Debugging. Setting near midnight to see if behavior holds (it seems to)
    #current_hour = 23
    #current_minute = 55
    #current_second = 0

    # Calculating the target time / the next 10-min-interval
    target_hour = current_hour
    target_minute = (current_minute // 10 + 1) * 10
    target_second = 0

    if (current_minute >= 50):
        target_hour = target_hour + 1
        target_minute = 0

    # Calculate the sleep duration based on the current and target minutes and seconds
    curr_time_as_seconds = (current_hour * 3600) + (current_minute * 60) + (current_second)
    target_time_as_seconds = (target_hour * 3600) + (target_minute * 60)

    return (target_time_as_seconds - curr_time_as_seconds)

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print ("Connected to MQTT broker")
    else:
        print ("Failed to connect, return code: ", rc)

def on_message(client, userdata, msg):
    # Decoding the incoming mqtt message and getting the current datettime in pacific
    message = msg.payload.decode()

    # <DEBUG>: printing out the data as it comes in
    print("Received message '" + message)
    print(current_time)

def send_out_mqtt_message(mqtt_client):
    mqtt_client.username_pw_set(secrets.username, secrets.password)
    mqtt_client.connect(secrets.ip_addr, 1883, 60)
    mqtt_client.publish("alarmTrigger", "ON")

def main():
    #Catch when script is interrupted, cleanup correctly
    mqtt_client = mqtt.Client()
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    try:
        # Main loop
        while True:
            # Get the photoresistor value to be used to activate things later.
            photoresistor_resistance_sorta = rc_time(pin_to_circuit)
            # Debug. Printing out values
            print("PHOTO RESISTOR VALUE")
            print(photoresistor_resistance_sorta)
            #print("TIME TO NEXT 10 min interval, in seconds")
            #print(get_sleep_duration(datetime.datetime.now()))

            # Connect to the sqlite3 database to save the photoresistor value?
            current_time = datetime.datetime.now()
            conn = sqlite3.connect('database.db')
            cursor = conn.cursor()
            cursor.execute("INSERT INTO esp_data (datetime, lightval) VALUES (?, ?)", (current_time, photoresistor_resistance_sorta))

            print("CLOSING CONNECTION")

            # Close out the database connection
            conn.commit()
            conn.close()

            time.sleep(get_sleep_duration(datetime.datetime.now()))

            send_out_mqtt_message(mqtt_client)




    except KeyboardInterrupt:
        pass
    finally:
        GPIO.cleanup()

if __name__ == '__main__':
    main()
