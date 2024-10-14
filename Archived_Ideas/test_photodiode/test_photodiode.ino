int lightInit;  // initial value
int lightVal;   // light reading

//cosntants for the pins where sensors are plugged into.
const int sensorPin = 19;

void setup()
{
  // We'll set up the LED pin to be an output.
  lightInit = analogRead(sensorPin);
  //we will take a single reading from the light sensor and store it in the lightCal        //variable. This will give us a prelinary value to compare against in the loop
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  lightVal = analogRead(sensorPin); // read the current light levels
  Serial.println(lightVal);
  delay(400);
}
