// ESP32's deepsleep library is limited to a maximum of about 30 minutes...
// This isn't necessarily too relevant for this situation, but good to know.
// This code is designed to wakeup the ESP-32 and send out the Zigbee call to the 
// HomeAssistant OS, once the sunrise alarm's brightness is past a certain threshold.
// One way to detect this it to just, sleep until a digital input of brightness is detected.
// ISSUE... the light sensor is an analog value. How do we convert it to a HIGH or LOW 
// so that the ext0 sleep can be used?

// An Answer: just dont lol.

// A better answer, dont do the ADC nonsense, instead we use programming magic to make up for hardware limitation... like a good engineer.
// Instead, we revert to the old diffuser logic of sleeping in 30-minute increments,
// Keep repeating the 30 minute increments of sleep. Wake up every 30 minutes,
// read the photoresistor, if it's above ~3000? (NOTE: adjust this as we get a more tight reading of the
// alarm) (NOTE NOTE: use the MQTT setup to forward the readings from the ESP-32 everytime it wakes up to the Node-red server and save with timestamp,
// This should help me narrow down and zero-in on what a proper value should be).
// ANYWAYYYY... If the value is above 3000, keep a bool called "alarm_is_started" or something True and go to sleep for another
// 30 minutes. Then, if still bright, keep alarm_is_started = True and set shake_time = True.
// After waking up now, if still bright and alarm_is_started AND shake_time, send Zigbee the message. It's time to gives this dude some seizures mf.

// ALSO, everyday, at somepoint during the day (idfk, 3 AM PST?), the ESP-32 will need to be restarted, so that the miliseconds of each execution
// Doesnt result in the alarm trigger occuring a whole minute or more after the alarm going off. After each restart, the firmware acts as if void setup()
// is executing for the first time and therefore, the first time execution code of timer = time it takes to get to the nearest 30 minute interval (if starting at 2:20, sleep until 2:30 or 
// if starting at 2:50, sleep until 3 and then after waking keep sleeping in 30 minute intervals) until alarm, yadda yadda.
// Cool. It's right here apparently?
// https://thelinuxcode.com/esp32-software-reset-arduino-ide/

// MORE IMPORTANTLY:
// For the physical setup of the circuit, ensure to shrink wrap the photoresistor and and point the photoresistor
// towards the back wall and not towards the light, to reduce the chances of a false trigger.

/* Alarm-Detector.ino. 
 * This program uses a photoresistor to detect when my
 * sunrise alarm clock goes off to then send a signal to my home assistant OS
 * via the Zigbee protocol to then trigger and turn on my bed shaker to shake me awake.
 * The logic for this code is that the ESP-32 here will sleep in 30 minute intervals
 * specifically, ensuring to wake up at the hour or 30 minutes into it, this will
 * require that the first sleep be triggered to approach the nearest 30 minute interval
 * and then every sleep after be triggered every 30 minutes. This will ensure that
 * the ESP-32 is awake and ready to detect light via the photoresistor from the 
 * alarm clock. The logic is that the first time light is detected,
 * that means the alarm will sound off in 1 hour, or 2 sleep cycles. 
 * From this point forward, I'll be using a 6 AM alarm as an example to explain.
 * We can use an RTC_DATA attribute to keep track of light being detected at 5 AM
 * then being detected at 5:30 AM again, and then the next time the ESP-32 wakes up,
 * if light is being detected and there's sound (not sure if necessary), send
 * out the signal to home assistant OS and trigger the bed shaker. Nice.
 * If there was a successful trigger of the bedshaker, set an RTC_DATA attribute
 * to keep track that it was triggered and then go to sleep for 10 MINUTES only.
 * Then, wakeup and if the bedshaker was triggered previously and light is still detected,
 * send the zigbee signal to close the shaker. This is an emergency failsafe to ensure
 * that the bed shaker is not shaking all day in case I forgot to turn it off.
 * Then lastly, ensure the esp32 restarts fully, thus running the code as if it was the first
 * time, thus running the cycle of sleeping until the next 30 minute time, aka from 6:10 to 6:30
 * and then repeating all over again.
 */

// Importing header file
#include "secrets.h"

// Setting the timer to 1800 seconds (DEFAULT) (30 minutes, because this is the max that can be done)
uint64_t sleepTimer = 1800 * uS_TO_S_FACTOR;

void setup() {

  // IMPORTANT: this is the logic for knowing if this is first boot, this is then used at the end of the code, to adjust
  // the timer to be a differential until the next 30 minute interval, or just a regular 30 minute timer (any boot except the first one.)
  bool diff_adjusted_timer = false;
  if (bootCount == 0) {
    diff_adjusted_timer = true;
  }
  // <DEBUG> Updating boot count every time and restarting on the second boot, to see if the restart works.
  bootCount++;
  // if (bootCount == 2) {
  //   ESP.restart();
  // }
  
  // Enable Serial using SERIAL_8N1 for ESP32. NOTE: ESP8266 requires different pins.
  Serial.begin(115200, SERIAL_8N1);

  // <DEBUG> Printing boot number:
  Serial.print("Boot Count: ");
  Serial.println(bootCount);
  print_wakeup_reason();

  // <DEBUG> (NOTE: this is unnecessary). Turn off the bright onboard LED
  // pinMode(ONBOARD_LED,OUTPUT);
  // digitalWrite(ONBOARD_LED,LOW);
  
  // Startup wifi and delay until fully connected.
  // <NOTE> Adding to the current seconds as well, just in case
  // the wifi fails, that way when the stored time is used
  // it is slightly less inaccurate.
  WiFi.begin(secrit1, secrit2);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    currSec = currSec + 1;
    Serial.print(".");
  }

  // <DEBUG> Printing out that we've officially connected and what our IP is.
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setting the server to be the mqtt_server from header file
  // and the specific port that server uses (1883)
  client.setServer(mqtt_server, 1883);

  // Initialize a NTPClient to get time and configure
  // it to daylight savings PST.
  const long  gmtOffset_sec = -8 * 3600;
  const int   daylightOffset_sec = 3600;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Store the current time to the timeinfo struct
  // TODO: track how long it takes to fail to get the time
  // and add to the offline time that estimated differential,
  // AGAIN, similar to adding the wifi delay to the currSec, this serves to
  // add to the seconds during offline operation to ensure closest accuracy
  // possible to proper NTP tracked time.
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    // <DEBUG> Printing that we're using the previously stored time += timer
    // rather than the actual updated time, thus this time will be off by a few seconds
    // every time, adding up to a few mintues, everyday.
    Serial.println("Failed to obtain time. Sticking to time += timer from previous sleep cycle. We're off by a few seconds at least.");
  }
  else {
    // Getting the current time from the NTP server.
    currHour = timeinfo.tm_hour;
    currMin = timeinfo.tm_min;
    currSec = timeinfo.tm_sec;
  }

  // FUTURE TODO: Adjusting ntp offset based on the Daylight savings?
  // TODO: Push this tm_isdst info to the NodeRed database, to keep track
  // if isdst updates and if that updates the time used and works properly.
  // If isdst updates then the FUTURE TODO of adjusting ntp offset based on 
  // daylight savings is unnecessary.
  // <DEBUG> Printing out daylight savings being true or not
  Serial.println("Daylight savings?");
  Serial.println(timeinfo.tm_isdst);

  int lightVal = analogRead(sensorPin); // read the current light levels
  String val = String(lightVal);
  char pub_val[val.length() + 1];
  val.toCharArray(pub_val, sizeof(pub_val));

  // Loop reconnecting to the MQTT until done?
  int count = 0;
  client.loop();
  
  client.publish("flashing_lights", pub_val);
  Serial.println(client.connected());
  
  // <DEBUG> Printing out the photoresistor value.
  Serial.println();
  Serial.println("This is the photoresistor value: ");
  Serial.println(lightVal);

  // <DEBUG> Printing out the current timme
  Serial.println("Current time: ");
  Serial.print(currHour);
  Serial.print(":");
  Serial.print(currMin);
  Serial.print(":");
  Serial.println(currSec);
  
  // Converting current time into seconds since midnight, allowing for checking diff
  // against the nearest 30 min interval, to generate the timer in seconds.
  int currTimeAsSeconds = (currHour * 3600) + (currMin * 60) + currSec;

  // <DEBUG> Printing out the current time as seconds since 0:00 o clock.
  // Serial.println(currTimeAsSeconds);
  
  // Working on finding the nearest 30 minute interval. If the current time is >= 30 min,
  // then the nearest interval is just the next hour (7:33 -> 8:00 as nearest 30 min interval)
  // Whereas if time < 30, then the nearest interval is the same hour with minutes = 30.
  // (7:13 -> 7:30)
  int nearestSleepHour = currHour;
  int nearestSleepMin = currMin;
  if (currMin >= 30){
    nearestSleepHour = nearestSleepHour + 1;
    nearestSleepMin = 0;
  }
  else {
    nearestSleepMin = 30;
  }
  
  // Converting the wakeup time into seconds since midnight and then subtracting
  // to get the actual time we want to sleep.
  int wakeUpTimeAsSeconds = (nearestSleepHour * 3600) + (nearestSleepMin * 60);
  // <DEBUG> Printing out this wakeUpTime
  Serial.println(wakeUpTimeAsSeconds);
  Serial.println(wakeUpTimeAsSeconds - currTimeAsSeconds);

  sleepTimer = (wakeUpTimeAsSeconds - currTimeAsSeconds) * uS_TO_S_FACTOR;
  // <DEBUG> Setting the timer to be just 10 seconds, why...
  sleepTimer = 10 * uS_TO_S_FACTOR;

  // Lastly, setting the current time to what the projected nearest 30 minute interval was, again, this should be correct upon wakeup, and while it will be off by a few seconds, 
  // the ntp server will correct it and if the ntp server doesnt catch it and this, worst case the alarm is off by a few seconds, I think.
  currHour = nearestSleepHour;
  currMin = nearestSleepMin;
  currSec = 0;

  // <DEBUG> Printing out the future projected time.
  Serial.println("Future projected time: ");
  Serial.print(currHour);
  Serial.print(":");
  Serial.print(currMin);
  Serial.print(":");
  Serial.println(currSec);

  // Calls on deep sleep with the timer
  deep_sleep();
}

// This should never run, as the ESP enters sleep at the end of setup()
void loop() {

}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void deep_sleep(){
  esp_sleep_enable_timer_wakeup(sleepTimer);
  Serial.println("Going to sleep now");
  esp_deep_sleep_start();
}