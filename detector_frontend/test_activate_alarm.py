# test_activate_alarm.py
# Send out an ON message to the home assistant OS.
# Home Assistant can handle the logic of false ONs, etc as required.

import paho.mqtt.client as mqtt
import secrets

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


def main():
    mqtt_client = mqtt.Client()
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message    

    mqtt_client.username_pw_set(secrets.username, secrets.password)
    mqtt_client.connect(secrets.ip_addr, 1883, 60)
    mqtt_client.publish("alarmTrigger", "ON")
    mqtt_client.loop_start()

    while True:
        pass

    conn.close()

if __name__ == '__main__':
    main()
