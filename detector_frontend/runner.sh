#!/bin/bash

# The main runner for the photoresistor detectorfrontend.

# FIRST: Run populate.py if database.db doesnt exist already. It should, already? But just in case.
if [ ! -f "database.db" ]; then
  python3 ./db_tools/populate.py &
fi

# SECOND: run photoresistor_detector.py to read in the photoresistor data thats coming in and save to the database.
# NOTE: this runs in parallel due to the & operator that's attached to the end of the command
sudo python3 photoresistor_detector.py &

# THIRD: run mqtt_frontend.py to create a frontend for displaying the esp_data with the datetime timestamp for when
# it came in
# NOTE: this runs in parallel due to the & operator that's attached to the end of the command
sudo python3 mqtt_frontend.py &

# OPTIONAL: print out the values in the database to debug and see where things are going wrong, etc.
#python3 /home/yasir/Scripts/Alarm-Detector/mqtt_frontend/db_tools/print_db.py
