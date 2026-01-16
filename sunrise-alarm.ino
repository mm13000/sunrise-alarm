
#include <RTClib.h>           // real-time clock module
#include <Adafruit_SSD1306.h> // OLED screen
#include <Wire.h>             // two-wire (I2C) communication

// ######################################################################################
// Global variables shared between things

DateTime dailyAlarmTime = DateTime(2000, 1, 1, 6, 0, 0);
bool dailyAlarmEnabled = false;
uint8_t dailyAlarmDurationHours = 0;
uint8_t dailyAlarmDurationMinutes = 30;
bool dailyAlarmCurrentlyActive = false;
bool time24hr = true; // military / 24-hour time

uint8_t tempHour = 0;
uint8_t tempMin = 0;

enum State {
  SCREEN_OFF,
  HOME_DISPLAY_CLOCK,
  MENU_SET_ALARM,
  MENU_SET_ALM_DURATION,
  MENU_SET_CLOCK,
  MENU_SET_24HR,
  SET_ALARM_HOUR,
  SET_ALARM_MIN,
  SET_CLOCK_HOUR,
  SET_CLOCK_MIN,
  SET_ALARM_DUR_HOUR,
  SET_ALARM_DUR_MIN,
  LED_ON
};
State currentState = HOME_DISPLAY_CLOCK;

uint8_t LEDbrightness = 0; // out of 255


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
void rtcClockSetup() {   // Should be run once at Arduino reboot
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("RTC not found or not connected");
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
  // rtc.setAlarm2(dailyAlarmTime, DS3231_A2_Hour);
}

bool clockLostPower() {
  // Detects if the RTC power was lost (battery lost its charge)
  // Could be used in the future to prompt for new time/date entry
  return rtc.lostPower();
}

void setClockTime(const uint8_t &hour, const uint8_t &min) {
  DateTime now = rtc.now();
  rtc.adjust(DateTime(now.year(), now.month(), now.day(), hour, min, 0));
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

void setDailyAlarm(uint8_t hour, uint8_t minute) {  //Alarm
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
    dailyAlarmCurrentlyActive = true;
    currentState = State::LED_ON;
  }
}


// ######################################################################################
// Display  and text printing

constexpr uint8_t SCREEN_WIDTH      = 128;   // pixels
constexpr uint8_t SCREEN_HEIGHT     = 32;    // pixels
constexpr uint8_t SCREEN_ADDRESS    = 0x3C;  // I2C address

Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

void oledScreenSetup() { // Initial setup of the OLED screen
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

enum DisplayCursorState {
  LARGE_UPPER,
  LARGE_LOWER,
  SMALL_UPPER,
  SMALL_LOWER
};

void setDisplayCursor(DisplayCursorState state); // needed because of Arduino's auto-prototyping
void setDisplayCursor(DisplayCursorState state) {
  switch (state) {
    case DisplayCursorState::LARGE_UPPER:
      display.setTextSize(TEXT_HALF_HEIGHT);
      display.setCursor(0, 0);
      break;
    case DisplayCursorState::LARGE_LOWER:
      display.setTextSize(TEXT_HALF_HEIGHT);
      display.setCursor(0, 16);
      break;
    case DisplayCursorState::SMALL_UPPER:
      display.setTextSize(TEXT_QRTR_HEIGHT);
      display.setCursor(0, 0);
      break;
    case DisplayCursorState::SMALL_LOWER:
      display.setTextSize(TEXT_QRTR_HEIGHT);
      display.setCursor(0, 24);
      break;
  }
}

void displayPrintMenuLabel() {
  setDisplayCursor(DisplayCursorState::LARGE_UPPER);
  display.print("Menu");
}

void displayPrintMenuUpArrow() {
  display.setTextSize(TEXT_HALF_HEIGHT);
  display.setCursor(116, 0);
  display.write(0x18);
}

void displayPrintMenuDownArrow() {
  display.setTextSize(TEXT_HALF_HEIGHT);
  display.setCursor(116, 16);
  display.write(0x19);
}

void displayPrintTime(const DateTime &time, bool minutes) {  
  uint8_t h = time.hour();
  uint8_t hour = time24hr ? h : (h == 0 ? 12 : (h > 12 ? h - 12 : h));
  if (hour < 10) display.print("0"); // leading zero
  display.print(hour);
  
  display.print(":");
  if (time.minute() < 10) display.print("0");
  display.print(time.minute());
  
  if (minutes) {
    display.print(":");
    if (time.second() < 10) display.print("0");
    display.print(time.second());
  }

  if (!time24hr) {
    display.print((time.hour() < 12) ? "am" : "pm");
  }
}

void printScreenHomeDisplayClock() {
  display.clearDisplay();

  setDisplayCursor(DisplayCursorState::LARGE_UPPER);
  DateTime now = rtc.now(); 
  displayPrintTime(now, true);

  setDisplayCursor(DisplayCursorState::SMALL_LOWER);
  display.print("Alarm ");

  if (dailyAlarmEnabled) displayPrintTime(dailyAlarmTime, false);
  else display.print("off");
  
  display.display();
}

void printScreenMenuSetAlarm() {
  display.clearDisplay();

  displayPrintMenuLabel();
  setDisplayCursor(DisplayCursorState::SMALL_LOWER);
  display.print("Set alarm");
  displayPrintMenuDownArrow();
  
  display.display();
}

void printScreenMenuSetAlmDuration() {
  display.clearDisplay();

  displayPrintMenuLabel();
  setDisplayCursor(DisplayCursorState::SMALL_LOWER);
  display.print("Set alm duration");
  displayPrintMenuUpArrow();
  displayPrintMenuDownArrow();
  
  display.display();
}

void printScreenMenuSetClock() {
  display.clearDisplay();

  displayPrintMenuLabel();
  setDisplayCursor(DisplayCursorState::SMALL_LOWER);
  display.print("Set clock");
  displayPrintMenuUpArrow();
  displayPrintMenuDownArrow();
  
  display.display();
}

void printScreenMenuSet24Hr() {
  display.clearDisplay();

  displayPrintMenuLabel();
  setDisplayCursor(DisplayCursorState::SMALL_LOWER);
  display.print("Clock mode: ");
  display.print(time24hr ? "24-hour" : "12-hour");
  displayPrintMenuUpArrow();
  
  display.display();
}

void printScreenSetHour() {
  display.clearDisplay();

  setDisplayCursor(DisplayCursorState::LARGE_UPPER);
  displayPrintTime(DateTime(0, 0, 0, tempHour, tempMin, 0), false);
  
  // Underline the hour
  display.drawFastHLine(0, 16, 22, SSD1306_WHITE);
  display.drawFastHLine(0, 17, 22, SSD1306_WHITE);
  
  display.display();
}

void printScreenSetMin() {
  display.clearDisplay();

  setDisplayCursor(DisplayCursorState::LARGE_UPPER);
  displayPrintTime(DateTime(0, 0, 0, tempHour, tempMin, 0), false);
  
  // Underline the minute
  display.drawFastHLine(36, 16, 22, SSD1306_WHITE);
  display.drawFastHLine(36, 17, 22, SSD1306_WHITE);
  
  display.display();
}

void printScreenLEDOn() {
  display.clearDisplay();

  setDisplayCursor(DisplayCursorState::LARGE_UPPER);
  display.print("LEDs On");

  setDisplayCursor(DisplayCursorState::SMALL_LOWER);
  display.print("Brightness ");
  display.print(LEDbrightness);
  display.print("/255");
  
  display.display();
}

void updateScreen() {
  switch(currentState) {
    case State::SCREEN_OFF:
      display.clearDisplay();
      display.display();
      break;
    case State::HOME_DISPLAY_CLOCK:
      printScreenHomeDisplayClock();
      break;
    case State::MENU_SET_ALARM:
      printScreenMenuSetAlarm();
      break;
    case State::MENU_SET_ALM_DURATION:
      printScreenMenuSetAlmDuration();
      break;
    case State::MENU_SET_CLOCK:
      printScreenMenuSetClock();
      break;
    case State::MENU_SET_24HR:
      printScreenMenuSet24Hr();
      break;
    case State::SET_ALARM_HOUR:
    case State::SET_ALARM_DUR_HOUR:
    case State::SET_CLOCK_HOUR:
      printScreenSetHour();
      break;
    case State::SET_ALARM_MIN:
    case State::SET_ALARM_DUR_MIN:
    case State::SET_CLOCK_MIN:
      printScreenSetMin();
      break;
    case State::LED_ON:
      printScreenLEDOn();
      break;
  }
}

// enum TextSize = {TEXT_SMALL, TEXT_MEDIUM, TEXT_LARGE};
// void displayText(char* text, TextSize size) {
//   display.setCursor(0, 0);
//   display.println(text);
// }


// ######################################################################################
// LED strip 

#define LED_CONTROL_SWITCH 6

void LEDSetup() {
  pinMode(LED_CONTROL_SWITCH, OUTPUT);
}

void updateLEDs() {
  if (dailyAlarmCurrentlyActive) {
    long alarmDurationSec = (long(dailyAlarmDurationHours) * 60 + dailyAlarmDurationMinutes) * 60;
    if (alarmDurationSec <= 0) {
      LEDbrightness = 255;
      return;
    }
    
    DateTime now = rtc.now();
    DateTime alarmStart = DateTime(now.year(), now.month(), now.day(), dailyAlarmTime.hour(), dailyAlarmTime.minute(), 0);
    TimeSpan timeSinceAlarmStart = now - alarmStart;
    long elapsed = timeSinceAlarmStart.totalseconds();
    if (elapsed < 0) elapsed = 0;
    
    float progress = (float)elapsed / alarmDurationSec;
    progress = constrain(progress, 0.0, 1.0);
    LEDbrightness = (uint8_t)(progress * 255.0);
  }
  analogWrite(LED_CONTROL_SWITCH, LEDbrightness);
}


// ######################################################################################
// State management, main dispatcher functions

void runHomeBtnPush() {
  // From the "home" (display clock screen), this turns the screen on or off. 
    // When the screen is off no other buttons do anything.
    // An alarm can still be triggered, however. This also turns the screen back on.
  // From any other screen, this takes you back to the home screen. It doesn't save changes.
  switch (currentState) {
    case State::SCREEN_OFF:
      currentState = State::HOME_DISPLAY_CLOCK;
      break;
    case State::HOME_DISPLAY_CLOCK:
      currentState = State::SCREEN_OFF;
      break;
    case State::MENU_SET_ALARM:
      currentState = State::HOME_DISPLAY_CLOCK;
      break;
    case State::MENU_SET_ALM_DURATION:
      currentState = State::HOME_DISPLAY_CLOCK;
      break;
    case State::MENU_SET_CLOCK:
      currentState = State::HOME_DISPLAY_CLOCK;
      break;
    case State::MENU_SET_24HR:
      currentState = State::HOME_DISPLAY_CLOCK;
      break;
    case State::SET_ALARM_HOUR:
      currentState = State::HOME_DISPLAY_CLOCK;
      break;
    case State::SET_ALARM_MIN:
      currentState = State::HOME_DISPLAY_CLOCK;
      break;
    case State::SET_CLOCK_HOUR:
      currentState = State::HOME_DISPLAY_CLOCK;
      break;
    case State::SET_CLOCK_MIN:
      currentState = State::HOME_DISPLAY_CLOCK;
      break;
    case State::SET_ALARM_DUR_HOUR:
      currentState = State::HOME_DISPLAY_CLOCK;
      break;
    case State::SET_ALARM_DUR_MIN:
      currentState = State::HOME_DISPLAY_CLOCK;
      break;
    case State::LED_ON:
      break;
  }
}

void runLEDOnOffBackBtnPush() {
  // Turns the LEDs on or off.
  // If an alarm is active (that's why the LEDs are on) de-activate the alarm
  // From the home screen, LEDs are turned on
  // From other screens (menu and setting screens) goes back without saving
  switch (currentState) {
    case State::SCREEN_OFF:
      break;
    case State::HOME_DISPLAY_CLOCK:
      LEDbrightness = 0;
      currentState = State::LED_ON;
      break;
    case State::MENU_SET_ALARM:
      currentState = State::HOME_DISPLAY_CLOCK;
      break;
    case State::MENU_SET_ALM_DURATION:
      currentState = State::HOME_DISPLAY_CLOCK;
      break;
    case State::MENU_SET_CLOCK:
      currentState = State::HOME_DISPLAY_CLOCK;
      break;
    case State::MENU_SET_24HR:
      currentState = State::HOME_DISPLAY_CLOCK;
      break;
    case State::SET_ALARM_HOUR:
      currentState = State::MENU_SET_ALARM;
      break;
    case State::SET_ALARM_MIN:
      currentState = State::SET_ALARM_MIN;
      break;
    case State::SET_CLOCK_HOUR:
      currentState = State::MENU_SET_CLOCK;
      break;
    case State::SET_CLOCK_MIN:
      currentState = State::SET_CLOCK_HOUR;
      break;
    case State::SET_ALARM_DUR_HOUR:
      currentState = State::MENU_SET_ALM_DURATION;
      break;
    case State::SET_ALARM_DUR_MIN:
      currentState = State::SET_ALARM_DUR_MIN;
      break;
    case State::LED_ON:
      LEDbrightness = 0;
      dailyAlarmCurrentlyActive = false;
      currentState = State::HOME_DISPLAY_CLOCK;
      break;
  }
}

void runDecreaseBtnPush() {
  // Decrease a current numeric value (setting alarm or clock)
  // Scroll down through menu items
  // Decrease the LED brightness
    // If in LEDs on state and alarm is active, decrease how many "minutes into" the alarm it is, which changes the brightness
  // If in "display clock" state, disable the daily alarm
  // Values wrap as appropriate (0 minutes goes around to 59 minutes)
  switch (currentState) {
    case State::SCREEN_OFF:
      break;
    case State::HOME_DISPLAY_CLOCK:
      dailyAlarmEnabled = false;
      disableDailyAlarm();
      break;
    case State::MENU_SET_ALARM:
      currentState = State::MENU_SET_ALM_DURATION;
      break;
    case State::MENU_SET_ALM_DURATION:
      currentState = State::MENU_SET_CLOCK;
      break;
    case State::MENU_SET_CLOCK:
      currentState = State::MENU_SET_24HR;
      break;
    case State::MENU_SET_24HR:
      break;
    case State::SET_ALARM_HOUR:
    case State::SET_CLOCK_HOUR:
    case State::SET_ALARM_DUR_HOUR:
      if (tempHour == 0) tempHour = 23; else tempHour -= 1;
      break;
    case State::SET_ALARM_MIN:
    case State::SET_CLOCK_MIN:
    case State::SET_ALARM_DUR_MIN:
      if (tempMin == 0) tempMin = 59; else tempMin -= 1;
      break;
    case State::LED_ON:
      dailyAlarmCurrentlyActive = false;
      if (LEDbrightness > 0) LEDbrightness -= 1;
      break;
  }
}

void runIncreaseBtnPush() {
  // Increase a current numeric value (setting alarm or clock)
  // Scroll up through menu items
  // Increase the LED brightness
    // If in LEDs on state and alarm is active, increase how many "minutes into" the alarm it is, which changes the brightness
  // If in "display clock" state, enable the daily alarm
  // Values wrap as appropriate (59 minutes goes back around to 0 minutes)
  switch (currentState) {
    case State::SCREEN_OFF:
      break;
    case State::HOME_DISPLAY_CLOCK:
      dailyAlarmEnabled = true;
      enableDailyAlarm();
      break;
    case State::MENU_SET_ALARM:
      break;
    case State::MENU_SET_ALM_DURATION:
      currentState = State::MENU_SET_ALARM;
      break;
    case State::MENU_SET_CLOCK:
      currentState = State::MENU_SET_ALM_DURATION;
      break;
    case State::MENU_SET_24HR:
      currentState = State::MENU_SET_CLOCK;
      break;
    case State::SET_ALARM_HOUR:
    case State::SET_CLOCK_HOUR:
    case State::SET_ALARM_DUR_HOUR:
      tempHour += 1;
      if (tempHour > 23) tempHour = 0;
      break;
    case State::SET_ALARM_MIN:
    case State::SET_CLOCK_MIN:
    case State::SET_ALARM_DUR_MIN:
      tempMin += 1;
      if (tempMin > 59) tempMin = 0;
      break;
    case State::LED_ON:
      dailyAlarmCurrentlyActive = false;
      if (LEDbrightness < 255) LEDbrightness += 1;
      break;
  }
}

void runMenuEnterBtnPush() {
  // Show the menu and select menu items
    // If in a "set" state rather than "home" or a "menu" state, 
    // then save the item being set and go back to menu
  switch (currentState) {
    case State::SCREEN_OFF:
      break;
    case State::HOME_DISPLAY_CLOCK:
      currentState = State::MENU_SET_ALARM;
      break;
    case State::MENU_SET_ALARM:
      tempHour = dailyAlarmTime.hour();
      tempMin = dailyAlarmTime.minute();
      currentState = State::SET_ALARM_HOUR;
      break;
    case State::MENU_SET_ALM_DURATION:
      tempHour = dailyAlarmDurationHours;
      tempMin = dailyAlarmDurationMinutes;
      currentState = State::SET_ALARM_DUR_MIN;
      break;
    case State::MENU_SET_CLOCK:
      tempHour = rtc.now().hour();
      tempMin = rtc.now().minute();
      currentState = State::SET_CLOCK_HOUR;
      break;
    case State::MENU_SET_24HR:
      time24hr = time24hr ? false : true;
      break;
    case State::SET_ALARM_HOUR:
      currentState = State::SET_ALARM_MIN;
      break;
    case State::SET_ALARM_MIN:
      setDailyAlarm(tempHour, tempMin);
      dailyAlarmEnabled = true;
      currentState = State::MENU_SET_ALARM;
      break;
    case State::SET_CLOCK_HOUR:
      currentState = State::SET_CLOCK_MIN;
      break;
    case State::SET_CLOCK_MIN:
      setClockTime(tempHour, tempMin);
      currentState = State::MENU_SET_CLOCK;
      break;
    case State::SET_ALARM_DUR_HOUR:
      currentState = State::SET_ALARM_DUR_MIN;
      break;
    case State::SET_ALARM_DUR_MIN:
      dailyAlarmDurationHours = tempHour;
      dailyAlarmDurationMinutes = tempMin;
      currentState = State::MENU_SET_ALM_DURATION;
      break;
    case State::LED_ON:
      break;
  }
}


// ######################################################################################
// Buttons

// Pin definitions
#define HOME_BUTTON  A6
#define LED_ON_OFF_BUTTON       A7
#define DECREASE_BUTTON         A2
#define INCREASE_BUTTON         A0
#define MENU_ENTER_BUTTON       A3
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
constexpr unsigned long doublePressTime_ms = 140;
unsigned long lastPress_HomeButton = 0;
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

  if (isButtonPressed(HOME_BUTTON) && millis() - lastPress_HomeButton > doublePressTime_ms) {
    lastPress_HomeButton = millis();
    runHomeBtnPush();
  }
  if (isButtonPressed(LED_ON_OFF_BUTTON) && millis() - lastPress_LEDOnOffButton > doublePressTime_ms) {
    lastPress_LEDOnOffButton = millis();
    runLEDOnOffBackBtnPush();
  }
  if (isButtonPressed(DECREASE_BUTTON) && millis() - lastPress_DecreaseButton > doublePressTime_ms) {
    lastPress_DecreaseButton = millis();
    runDecreaseBtnPush();
  }
  if (isButtonPressed(INCREASE_BUTTON) && millis() - lastPress_IncreaseButton > doublePressTime_ms) {
    lastPress_IncreaseButton = millis();
    runIncreaseBtnPush();
  }
  if (isButtonPressed(MENU_ENTER_BUTTON) && millis() - lastPress_ModeButton > doublePressTime_ms) {
    lastPress_ModeButton = millis();
    runMenuEnterBtnPush();
  }
}


// ######################################################################################
// MAIN SETUP AND LOOP

void setup() {
  Serial.begin(9600); // open serial communication
  oledScreenSetup();
  rtcClockSetup();
  LEDSetup();
}

void loop() {
  // Listen for a button click / event / alarm going off
  checkAlarm();
  handleButtonPresses();
  updateScreen();
  updateLEDs();
  // Handle any events / button clicks 
  //    -> set variables
  //    -> change the device state
  // Update the screen
  // Update the state of the lights based on global variables
  //    e.g. lights on if alarm on, or they're turned on manually

  // if (isButtonPressed(ALM_ENABLE_SWITCH)) {
  //   Serial.println("ALM_ENABLE_SWITCH on");
  // }

  delay(100);
}