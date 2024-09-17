#!/usr/bin/env python

# Importing the sqlite3 library for working with sqlite databases
import sqlite3
import os

def main():
    # Connect to the SQLite database

    conn = sqlite3.connect('../database.db')
    c = conn.cursor()

    # Check if the table already exists
    c.execute('''SELECT count(name) FROM sqlite_master WHERE type='table' AND name='esp_data' ''')
    if c.fetchone()[0] == 0:
        # Create the table if it doesn't exist
        c.execute('''CREATE TABLE esp_data
                     (datetime TEXT, lightval TEXT)''')

    # Initialize the table with data associated with columns above
    # Data is configured as:
    # datetime, bagtag, epc tag, last scanned location, location on or off the plane
    data = [
        ('0','0')
        # Add more data entries as needed
    ]
    #print("is this running?")
    c.executemany('INSERT INTO esp_data VALUES (?, ?)', data)

    conn.commit()
    conn.close()

# Function to delete the database file
def delete_database():
    database_file = '../database.db'
    if os.path.exists(database_file):
        os.remove(database_file)
        print("Database deleted successfully.")
    else:
        print("Database does not exist.")

# Function to keep the database
def keep_database():
    print("Database kept.")

def user_input():
    # Prompt the user for input
    user_input = input("Do you want to delete the database before making it?(Y/N): ")

    # Process the user's choice
    if user_input.lower() == 'y':
        delete_database()
    elif user_input.lower() == 'n':
        keep_database()
    else:
        print("Invalid input. Please enter Y or N.")


if __name__ == "__main__":
    delete_database()
    main()

