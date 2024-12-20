
#include <RTClib.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Global constants definitions
#define SCREEN_WIDTH            128
#define SCREEN_HEIGHT           32
#define SCREEN_ADDRESS          0x3C
#define BUTTON_THRESHOLD        2.5
#define LED_STRIP_UPDATE_PERIOD 0.5
#define DISPLAY_UPDATE_PERIOD   1

// Define peripheral objects
RTC_DS3231 rtc;
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// Pin definitions
#define LED_STRIP           6
#define CLOCK_INTERRUPT_PIN 2
#define ALARM_OFF_BUTTON    A2

// EEPROM (non-volatile memory) address for storing alarm duration
// #define EEPROM_ADDR_ALM_DURATION  0


/* Global Variables: Options set by the user */
int alarmDuration = 1; // Duration of alarm (time for LEDs to reach full brightness), in minutes


/* Global variables */
volatile bool alarmTriggered = false;
bool alarmActive = false;
DateTime alarmStart((uint32_t)0);


/* Function declarations */
void rtcSetup();
void oledSetup();
void onRTCAlarm();
void setLEDBrightness();
void refreshDisplay();
void displayTime();
bool isButtonPushed(int button_pin);
float getPinVoltage(int pin);

// In the main loop: an "AlarmTriggered" boolean. This is turned on by the "onRTCAlarm" method.
// If that is set to True, we want to start the alarm, which is the LED strip. I want the time
// it takes for the LED strip to reach full brightness to be adjustable by the user.
// So maybe a function that takes the time since the alarm started, the desired total alarm time,
// and spits out an integer (0 to 255) of how bright the LED strip should be.

byte compteur;


void setup() {
  // Open Serial communication
  Serial.begin(9600);

  // Set up the RTC
  rtcSetup();

  // Set up the 0.91 OLED
  oledSetup();

  // Setup a alarm for 10 seconds from now
  if(!rtc.setAlarm1(
    rtc.now() + TimeSpan(5),
    DS3231_A1_Second // this mode triggers the alarm when the seconds match. See Doxygen for other options
  )) {
    Serial.println("Error, alarm wasn't set!");
  } else {
    Serial.println("Alarm will happen in 5 seconds!");
  }

  // Set up Pins
  pinMode(LED_STRIP, OUTPUT);
}


void loop() {
  // When an alarm has been triggered by the RTC, turn on the alarm
  if (alarmTriggered) {
    alarmTriggered = false;
    alarmActive = true;
    alarmStart = rtc.now();
  }

  // Set LED brightness
  static DateTime t;
  if (!alarmActive) {
    t = rtc.now();
  } else {
    if (rtc.now().unixtime() - t.unixtime() > LED_STRIP_UPDATE_PERIOD) {
      setLEDBrightness();
      t = rtc.now();
    }
  }
  
  // Set display output
  refreshDisplay();

  // Eventually this could be a sort of state machine
  // Setting different user options is one state
  // Regular function is another state
  // So that when you are setting the user options I can just focus on that in the code
  // Necessary? not sure

}

void onRTCAlarm() {
  alarmTriggered = true;
}

void setLEDBrightness() {
  uint32_t elapsedSec = rtc.now().unixtime() - alarmStart.unixtime();
  float incrementPerSec = 255 / (alarmDuration * 60);
  
  uint8_t brightness = min((uint8_t)(elapsedSec * incrementPerSec), 255);
  analogWrite(LED_STRIP, brightness);

  // Serial.print(" Brightness: ");
  // Serial.print(brightness);
  // Serial.println(" / 255");
}

void refreshDisplay() {
  static uint32_t lastRefresh = rtc.now().unixtime();
  if (rtc.now().unixtime() - lastRefresh >= DISPLAY_UPDATE_PERIOD) {
    lastRefresh = rtc.now().unixtime();
  } else {
    return;
  }

  display.clearDisplay();

  display.setCursor(0, 0);
  display.print("T ");
  displayTime(rtc.now());

  display.setCursor(0, 16);
  display.print("A ");
  displayTime(rtc.getAlarm1());

  display.display();
}

void displayTime(DateTime time) {
  uint8_t hour = time.hour();
  uint8_t minute = time.minute();
  uint8_t second = time.second();
  
  if (hour < 10) display.print("0");
  display.print(hour);
  display.print(":");
  if (minute < 10) display.print("0");
  display.print(minute);
  display.print(":");
  if (second < 10) display.print("0");
  display.print(second);
}

bool isButtonPushed(int button_pin) {
  return getPinVoltage(button_pin) > BUTTON_THRESHOLD;
}

float getPinVoltage(int pin) {
  return 5 * (float)analogRead(pin) / 1024;
}

/* Setup of Peripherals */

void rtcSetup() {
  if(!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    Serial.flush();
    while (1) delay(10);
  }
  if (rtc.lostPower()) {
    // Adjust the date and time at compilation
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  // Disable 32K pin because we don't need it
  rtc.disable32K();

  // Make it so that RTC alarm will trigger interrupt
  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onRTCAlarm, FALLING);

  // Set alarm 1 & 2 flag to false (alarm 1 and 2 didn't happen yet)
  // If not done this easily leads to problems, since both registers aren't reset on reboot/recompile
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  // Stop oscillating signals at SQW Pin, otherwise setAlarm1 will fail
  rtc.writeSqwPinMode(DS3231_OFF);

  // turn off alarm 2 (in case it isn't off already)
  // again, this isn't done at reboot, so a previously set alarm could easily go overlooked
  rtc.disableAlarm(2);
}

void oledSetup() {
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.display();
  delay(200);
  display.clearDisplay();
  display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE);
}