
#include <RTClib.h>           // real-time clock module
#include <Adafruit_SSD1306.h> // OLED screen
#include <Wire.h>             // two-wire (I2C) communication

// Global variables shared between things:
// Alarm currently active
// Alarm enabled / disabled
// Lights turned on / off
// Light brightness percentage


// #########################################################################
// Display management and text printing

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

constexpr uint8_t TEXT_HALF_HEIGHT  = 1;  // 8 pixels height  = 1/4 screen height
constexpr uint8_t TEXT_QRTR_HEIGHT  = 2;  // 16 pixels height = 1/2 screen height

// enum TextSize = {TEXT_SMALL, TEXT_MEDIUM, TEXT_LARGE};
// void displayText(char* text, TextSize size) {
//   display.setCursor(0, 0);
//   display.println(text);
// }


// #########################################################################
// RTC clock / alarm management

RTC_DS3231 rtc;

void rtcSetup() {

}


// #########################################################################
// Button management

// Pin definitions
#define LED_STRIP           6
#define CLOCK_INTERRUPT_PIN 2
#define ON_OFF_BUTTON       A7
#define DISPLAY_ON_OFF_BTN  A6
#define DECREASE_BUTTON     A2
#define INCREASE_BUTTON     A0
#define MODE_BUTTON         A3
#define ALM_ENABLE_SWITCH   A1

#define BUTTON_THRESHOLD    2   // out of 5V

float getPinVoltage(int pin) {
  // Read the analog voltage (0-5V) from an analog input pin
  return 5 * (float)analogRead(pin) / 1024;
}

bool isButtonPressed(int button_pin) {
  return getPinVoltage(button_pin) > BUTTON_THRESHOLD;
}


// #########################################################################
// LED strip management




// #########################################################################
// State management, main dispatcher functions




// #########################################################################
// MAIN SETUP AND LOOP

void setup() {
  Serial.begin(9600); // open serial communication
  oledSetup();
}

void loop() {
  // Listen for a button click / event
  // Handle any events / button clicks 
  //    -> set variables
  //    -> change the device state
  // Update the screen
  // Update the state of the lights based on global variables
  //    e.g. lights on if alarm on, or they're turned on manually

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 16);
  display.println("Hello");
  display.display();

  if (isButtonPressed(ALM_ENABLE_SWITCH)) {
    Serial.println("ALM_ENABLE_SWITCH on");
  }

  delay(100);
}