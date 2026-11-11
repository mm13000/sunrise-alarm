#pragma once

// OLED
constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 32;
constexpr uint8_t SCREEN_ADDRESS = 0x3C;

// LED strip
constexpr uint8_t LED_CONTROL_PIN = 6;

// Buttons
constexpr uint8_t HOME_BUTTON_PIN = A6;
constexpr uint8_t LED_ON_OFF_BUTTON_PIN = A7;
constexpr uint8_t DECREASE_BUTTON_PIN = A2;
constexpr uint8_t INCREASE_BUTTON_PIN = A0;
constexpr uint8_t MENU_ENTER_BUTTON_PIN = A3;
// constexpr uint8_t ALARM_ENABLE_SWITCH_PIN = A1;

// Input and timing settings
constexpr float BUTTON_VOLTAGE_THRESHOLD = 2.0f;
constexpr unsigned long BUTTON_DEBOUNCE_MS = 140;

// Serial and loop settings
constexpr unsigned long SERIAL_BAUD_RATE = 9600;
constexpr unsigned long LOOP_DELAY_MS = 100;

// Default behavior
constexpr uint8_t DEFAULT_ALARM_HOUR = 6;
constexpr uint8_t DEFAULT_ALARM_MINUTE = 0;
constexpr uint8_t DEFAULT_ALARM_DURATION_HOURS = 0;
constexpr uint8_t DEFAULT_ALARM_DURATION_MINUTES = 30;
constexpr bool DEFAULT_24_HOUR_MODE = true;