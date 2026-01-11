
#include <RTClib.h>           // real-time clock module
#include <Adafruit_SSD1306.h> // OLED screen
#include <Wire.h>             // two-wire (I2C) communication

// Global variables shared between things:
DateTime dailyAlarmTime = DateTime(2000, 1, 1, 6, 0, 0);
bool dailyAlarmActive = false;
bool screenOn = true;

// Alarm enabled / disabled
// Lights turned on / off
// Light brightness percentage


// ######################################################################################
// Display  and text printing

constexpr uint8_t SCREEN_WIDTH      = 128;   // pixels
constexpr uint8_t SCREEN_HEIGHT     = 32;    // pixels
constexpr uint8_t SCREEN_ADDRESS    = 0x3C;  // I2C address

Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

void oledSetup() { // Initial setup of the OLED screen
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextColor(WHITE);  // necessary
  display.display();
}

constexpr uint8_t TEXT_QRTR_HEIGHT  = 1;  // 8 pixels height  = 1/4 screen height
constexpr uint8_t TEXT_HALF_HEIGHT  = 2;  // 16 pixels height = 1/2 screen height
constexpr uint8_t TEXT_3QRTR_HEIGHT = 3;  // 24 pixels height = 3/4 screen height
constexpr uint8_t TEXT_FULL_HEIGHT  = 4;  // 32 pixels height = total screen height

void clearDisplay() { // First step before putting new things on the display
  display.clearDisplay();
}

void refreshDisplay() {
  display.display();
}

// enum TextSize = {TEXT_SMALL, TEXT_MEDIUM, TEXT_LARGE};
// void displayText(char* text, TextSize size) {
//   display.setCursor(0, 0);
//   display.println(text);
// }


// ######################################################################################
// RTC clock / alarm 

// Setup for using Arduino interrupt mechanism to wake it from sleep when an alarm goes 
// off. Currently this is not really used, because I don't want to put the Arduino to 
// sleep if the *only* way to wake it up is with an alarm. I think if I want to use this 
// "sleep" functionality, I should also attach a button to the D2 pin and allow the 
// button to wake it up. But honestly, this all might be overkill. Does it really need 
// to sleep if it is going to be plugged in all the time?
// #define CLOCK_INTERRUPT_PIN 2
// attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), wakeISR, FALLING);

RTC_DS3231 rtc;

void rtcSetup() {   // Should be run once at Arduino reboot
  Wire.begin();
  if (!rtc.begin()) {
    // TODO: handle an error where the RTC is not found / not connected
  }

  rtc.writeSqwPinMode(DS3231_OFF);  // disable SQW alarm interrupt
  rtc.disable32K();                 // 32K consumes power and isn't needed

  // Alarms should be disabled because they may have been flagged while Arduino
  // was powered off and could trigger immediately
  rtc.disableAlarm(1);  
  rtc.disableAlarm(2);
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  // Since alarm times cannot be queried from the RTC, they need to be reset
  // if the Arduino was rebooted
  rtc.setAlarm2(dailyAlarmTime, DS3231_A2_Hour);
}

bool clockLostPower() {
  // Detects if the RTC power was lost (battery lost its charge)
  // Could be used in the future to prompt for new time/date entry
  return rtc.lostPower();
}

void setClockTime(const DateTime &newTime) {
  DateTime now = rtc.now();
  rtc.adjust(DateTime(
    now.year(), now.month(), now.day(),
    newTime.hour(), newTime.minute(), newTime.second()
  ));
}

void setClockDate(const DateTime &newDate) {
  DateTime now = rtc.now();
  rtc.adjust(DateTime(
    newDate.year(), newDate.month(), newDate.day(),
    now.hour(), now.minute(), now.second()
  ));
}

void setPreciseAlarm(DateTime alarmTime) {  // Alarm 1 has seconds-level precision
  rtc.clearAlarm(1);
  rtc.setAlarm1(
    DateTime(0, 0, 0, alarmTime.hour(), alarmTime.minute(), 0), 
    DS3231_A1_Hour
  );
}

void setDailyAlarm(int hour, int minute) {  //Alarm
  dailyAlarmTime = DateTime(2000, 1, 1, hour, minute, 0);
  rtc.clearAlarm(2);
  rtc.setAlarm2(dailyAlarmTime, DS3231_A2_Hour);
}

bool isPreciseAlarmTriggered() {
  if (rtc.alarmFired(1)) {
    rtc.clearAlarm(1);
    return true;
  }
  return false;
}

bool isDailyAlarmTriggered() {
  if (rtc.alarmFired(2)) {
    rtc.clearAlarm(2);
    return true;
  }
  return false;
}

void disableDailyAlarm() {
  rtc.disableAlarm(2);
}

void enableDailyAlarm() {
  rtc.clearAlarm(2);
  rtc.setAlarm2(dailyAlarmTime, DS3231_A2_Hour);
}

void checkAlarm() {
  if (isDailyAlarmTriggered()) {
    dailyAlarmActive = true;
    // TODO: wake up screen
  }
}


// ######################################################################################
// LED strip 




// ######################################################################################
// State management, main dispatcher functions

// Some probably-shared variables:
  // Is an alarm active? What time was it started?
  // Is the screen turned on or off? That one gets set by a button

// Buttons
  // SCREEN_ON_OFF_BUTTON
    // Turns the screen on or off
    // When the screen is off no other buttons do anything
    // An alarm can still be triggered, however. This also turns the screen back on.
  // LED_ON_OFF_BUTTON
    // Turns the LEDs on or off
    // If an alarm is active (that's why the LEDs are on) de-activate the alarm
    // Immediately switch into or out of "LEDs on" state on the screen
      // If switching out of that state, always return to "home screen" (display clock)
    // Also functions as "secret" method of getting into "Set clock" mode
      // If pressed while the alarm is being set, save alarm and move to set clock mode
      // If pressed in set clock mode, save clock and move to set home screen
  // DECREASE_BUTTON and INCREASE_BUTTON
    // Increase or decrease a current numeric value (setting alarm or clock)
    // Increase or decrease the LED brightness
      // If in LEDs on state and alarm is active, change how many "minutes into" the alarm it is, which changes the brightness
    // If in "display clock" state, enable or disable the daily alarm
    // Values wrap as appropriate (59 minutes goes back around to 0 minutes)
  // MODE_BUTTON
    // Cycle from home (display time) -> set alarm -> set alarm duration
      // If in set clock mode, save clock and move to home screen
    // When exiting, moving from one state to another, save what was just set

// States:
  // Display clock
  // Set alarm
  // Set alarm duration
  // LEDs on (including alarm on)
    // Display brightness percentage of LEDs
    // Display alarm activation time if an alarm is currently active
    // Display minutes until full brightness if an alarm is currently active



// ######################################################################################
// Buttons

// Pin definitions
#define LED_STRIP           6
#define SCREEN_ON_OFF_BTN   A6
#define LED_ON_OFF_BUTTON   A7
#define DECREASE_BUTTON     A2
#define INCREASE_BUTTON     A0
#define MODE_BUTTON         A3
// #define ALM_ENABLE_SWITCH   A1

constexpr float buttonVoltageThreshold = 2;   // out of 5V

float getPinVoltage(int pin) {
  // Read the analog voltage (0-5V) from an analog input pin
  return 5 * (float)analogRead(pin) / 1024;
}

bool isButtonPressed(int button_pin) {
  return getPinVoltage(button_pin) > buttonVoltageThreshold;
}

// Most recent times each of the buttons were pressed
constexpr unsigned long doublePressTime_ms = 200;
unsigned long lastPress_ScreenOnOffButton = 0;
unsigned long lastPress_LEDOnOffButton = 0;
unsigned long lastPress_DecreaseButton = 0;
unsigned long lastPress_IncreaseButton = 0;
unsigned long lastPress_ModeButton = 0;

void handleButtonPresses() {
  // run through the buttons
    // compare current time to last time pressed
    // determine if button is "validly" pressed. If so:
      // Update "last time pressed"
      // Call method that handles this button being pressed (in State management section)

  if (isButtonPressed(SCREEN_ON_OFF_BTN) && millis() - lastPress_ScreenOnOffButton > doublePressTime_ms) {
    lastPress_ScreenOnOffButton = millis();
    // Do button stuff (function that is called in the "state management" area?)
  }
  if (isButtonPressed(LED_ON_OFF_BUTTON) && millis() - lastPress_LEDOnOffButton > doublePressTime_ms) {
    lastPress_LEDOnOffButton = millis();
    // Do button stuff
  }
}


// ######################################################################################
// MAIN SETUP AND LOOP

void setup() {
  Serial.begin(9600); // open serial communication
  oledSetup();
  rtcSetup();
}

void loop() {
  // Listen for a button click / event / alarm going off
  checkAlarm();
  handleButtonPresses();
  // Handle any events / button clicks 
  //    -> set variables
  //    -> change the device state
  // Update the screen
  // Update the state of the lights based on global variables
  //    e.g. lights on if alarm on, or they're turned on manually

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 16);
  display.println("Hello");
  display.display();

  // if (isButtonPressed(ALM_ENABLE_SWITCH)) {
  //   Serial.println("ALM_ENABLE_SWITCH on");
  // }

  delay(100);
}