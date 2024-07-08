// NOTE on 12/03/2023:
// NEVERMIND. This doesn't work... as I think it should.
//  First thing I should have googled...
// ESP32's deepsleep library is limited to a maximum of like 30 minutes.

// Thus, the best you can do is to have iterative 30 minutes of sleep everytime.
// Until ~30 min before 3 pm. Then, set the timer to be X amount of seconds until
// 3 pm. Then, at 3 pm, set a timer for 30 min and press the buttons to turn on the
// diffuser and set the pressed condition to be true, so that way the buttons arent
// pressed just based on the time being 3 < time < 6 pm. Then, at 6 pm or later,
// set the pressed condition to be false. Then iterate 30 min each time again. Nice.

/* diffuserInfrared.ino. This file uses
 * the IRRemoteESP8266 library to send 
 * out an on signal to my essential oil
 * diffuser at a specific time.
 * The time is determined by 
 * connecting to the internet
 * and then determining the time required
 * from now to 3 pm pacific time.
 * A timer is started based on that specific time.
 * Then the second timer which is a fixed amount,
 * is started, for the diffuser to run for 3 hours.
 * A boolean is used to determine which timer is triggered.
 * The first run of code HAS to be for determining
 * diffuserStartSleepTimer and then going to sleep based 
 * on diffuserStartSleepTimer.
 */

// Importing header file
#include "secrets.h"

// Setting the timer to 1800 seconds (30 minutes, because this is the max that can be done)
uint64_t sleepTimer = 1800 * uS_TO_S_FACTOR;

void setup() {  
  // Updating boot count every time.
  // bootCount++;

  // Enable Serial using different pins, depending if ESP8266 or not
  #if ESP8266
    Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  #else  // ESP8266
    Serial.begin(115200, SERIAL_8N1);
  #endif  // ESP8266
  
  // Printing boot number:
  // Serial.print("Boot Count: ");
  // Serial.println(bootCount);
  // print_wakeup_reason();

  // Turn off the bright onboard LED
  // pinMode(ONBOARD_LED,OUTPUT);
  // digitalWrite(ONBOARD_LED,LOW);
  
  // Startup wifi and delay until fully connected
  WiFi.begin(secrit1, secrit2);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(". ");
  }
  Serial.println("WiFi connected.");

  // Initialize a NTPClient to get time and configure
  // it to daylight savings PST.
  const long  gmtOffset_sec = -8 * 3600;
  const int   daylightOffset_sec = 3600; 
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Store the current time to the timeinfo struct
  // TODO: add a set of RTC_DATA_ATTR variables that 
  // store the previous time, so that if failed to obtain time,
  // then, start using the offline time to determine currHour, etc.
  // Update as needed, etc etc. Since each iteration results in time being
  // off by like 2 seconds each time, eventually time goes off by a decent amount?
  // But then you can solve that by plugging in again? Or just using online time...
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  // Storing current time
  int currHour = timeinfo.tm_hour;
  int currMin = timeinfo.tm_min;
  int currSec = timeinfo.tm_sec;

  // TODO: Adjusting ntp offset based on the Daylight savings?
  Serial.println("Daylight savings?");
  Serial.println(timeinfo.tm_isdst);

  // Printing out the current timme
  // Serial.println("Current time: ");
  // Serial.print(currHour);
  // Serial.print(":");
  // Serial.print(currMin);
  // Serial.print(":");
  // Serial.println(currSec);
  
  // Converting current time into seconds since midnight, allowing for checking diff
  // with 15 o clock, thus allowing us to set a timer for an exact time that ends
  // at 15 o clock. The rest of the time, timer is set for 
  int currTimeAsSeconds = (currHour * 3600) + (currMin * 60) + currSec;
  Serial.println(currTimeAsSeconds);

  if (currHour == 14 && currMin >= 30) {
    int diff = diffuseTimeStart - currTimeAsSeconds;
    sleepTimer = diff * uS_TO_S_FACTOR;
  }
  else {
    sleepTimer = 1800 * uS_TO_S_FACTOR;
  }

  // Setting currHour to 15 as a test, comment it out when not testing.
  // currHour = 15;

  // Sending out a power button press and setting it up for 3 hours.
  if (pressed == false && currHour >= 15 && currHour < 18) {
    // Start up the Infrared communication
    irsend.begin();
    Serial.println("Sending out NEC power button press and the timer adjustment");
    // On
    irsend.sendNEC(0xFF00FF);
    delay(500);
    // Timer button, set to 1 hour
    irsend.sendNEC(0xFF10EF);
    delay(500);
    // Timer button, set to 2 hours
    irsend.sendNEC(0xFF10EF);
    delay(500);
    // Timer button, set to 3 hours
    irsend.sendNEC(0xFF10EF);
    pressed = true;
    // Setting the timer to 10 seconds as a test, comment it out when not testing.
    // sleepTimer = 10 * uS_TO_S_FACTOR;
  }

  if (currHour >= 18) {
    pressed = false;
  }

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