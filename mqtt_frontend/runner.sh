#!/bin/bash

# The main runner for the esp_data frontend.

# FIRST: Run populate.py if database.db doesnt exist already. It should, already? But just in case.
if [ ! -f "/home/yasir/Scripts/Alarm-Detector/mqtt_frontend/database.db" ]; then
  python3 ./db_tools/populate.py &
fi

# SECOND: run mqtt_saver.py to read in the MQTT data thats coming in and save to the database.
# NOTE: this runs in parallel due to the & operator that's attached to the end of the command
python3 mqtt_saver.py &

# THIRD: run mqtt_frontend.py to create a frontend for displaying the esp_data with the datetime timestamp for when
# it came in
# NOTE: this runs in parallel due to the & operator that's attached to the end of the command
python3 mqtt_frontend.py &

# OPTIONAL: print out the values in the database to debug and see where things are going wrong, etc.
#python3 /home/yasir/Scripts/Alarm-Detector/mqtt_frontend/db_tools/print_db.py
