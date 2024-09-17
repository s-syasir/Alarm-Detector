# mqtt_saver.py
# This file reads in the data coming in from the ESP-32 via MQTT
# writes it to a database that will be used by mqtt_frontend.py
# to be written into the frontend website that is used for debugging.

import paho.mqtt.client as mqtt
import datetime
import pytz
import sqlite3

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print ("Connected to MQTT broker")
    else:
        print ("Failed to connect, return code: ", rc)

def on_message(client, userdata, msg):
    # Decoding the incoming mqtt message and getting the current datettime in pacific
    message = msg.payload.decode()
    current_time = datetime.datetime.now(pytz.timezone('America/Los_angeles')).strftime('%Y-%m-%d %H:%M:%S')
    
    # Connecting to the database file and creating a cursor to write values in
    conn = sqlite3.connect('database.db')
    cursor = conn.cursor()

    # Write values into th file and save
    cursor.execute("INSERT INTO esp_data (datetime, lightval) VALUES (?, ?)", (current_time, message))
    conn.commit()
    conn.close()

    # <DEBUG>: printing out the data as it comes in
    print("Received message '" + message)
    print(current_time)


def main():
    mqtt_client = mqtt.Client()
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect('localhost', 1883, 60)
    mqtt_client.subscribe("flashing_lights")
    mqtt_client.loop_start()

    while True:
        pass

    conn.close()

if __name__ == '__main__':
    main()
