"""Database Table Printer.

This script allows the user to print to the console an sqlite3 database, inventory_data.db.

Returns
-------
None

Params
------
None


"""

# Defining the runtime binary for python that should be used
#!/usr/bin/env python

# Importing sqllite3 library to interface with the sqlite3 database
import sqlite3

# Return connection obj to interact with the SQLite db
conn = sqlite3.connect("database.db") 

def main():
    # Verify successful connection creation
    cursor = conn.cursor()          # Return cursor obj to send sql statements to sqlite database

    # Get all table names from the database and store that to a variable
    cursor.execute("SELECT * FROM sqlite_master WHERE type='table'")
    tables = cursor.fetchall()      

    # Iterate through all the tables and get their names. There should be just 1 table.
    # Therefore, this executes only once and just sets the rows variable equal to the last list of rows (table)
    for table_name in tables:
        table_name = table_name[1]  # Extract the table name
        cursor.execute("SELECT * FROM {}".format(table_name))  # Fetch all rows from the table
        rows = cursor.fetchall()        # retrieve results from each table name

    # Creating some space before printing out the table and all its values
    print()
    print("======================================= NOW PRINTING OUT WHAT was stored to the databse ===============================================")

    # Print table name and values (there should be just 1 table...)
    print("Table:", table_name)
    i = 1 
    
    # Iterating through all the rows within the table and print out the row
    for row in rows:
        print("Row: " + str(i))
        print(row)
        i = i + 1

    conn.close()

if __name__ == "__main__":
    main()
