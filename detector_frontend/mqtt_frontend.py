from flask import Flask, render_template
import sqlite3

# Initialize Flask app
app = Flask(__name__)

# Connect to the database (or create it if it doesn't exist)
def get_db_connection():
  '''Connect to database.db (create if not existing)
  PARAM:    NONE (database.db)
  RETURNS:  conn - () 
  '''
  conn = sqlite3.connect('database.db')
  conn.row_factory = sqlite3.Row
  return conn

# Function to fetch all data from a table
def fetch_data_from_db():
  '''
  PARAM:
  RETURNS: data (list) - 
  '''   
  conn = get_db_connection()
  cursor = conn.cursor()
  cursor.execute('SELECT * FROM esp_data')  # Replace with your table name
  data = cursor.fetchall()
  conn.close()
  return data

# Route for the main page
@app.route('/')
def index():
  data = fetch_data_from_db()
  # Reversing data for better look
  print(type(data))
  data.reverse()
  return render_template('index.html', data=data)

if __name__ == '__main__':
  app.run(host='0.0.0.0', debug=True, port=2121)
