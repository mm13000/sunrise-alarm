# Project: Arduino Sunrise Alarm Clock

Source of truth for design decisions and implementation details, as well as notes to self for the design process.

## Overview

My goal for this project is to implement a programmable light alarm, which gradually brightens an LED strip over a user-defined time. Gradual brightening can be accomplished using a PWM signal with slowly increasing duty cycle. The user should be able to set the clock time, alarm time, and alarm duration. 

## Notes on components

| Component | Pre-PCB (current) | Post-PCB (planned) | Notes |
|-----------|-------------------|--------------------|-------|
| LED strip | Not completely sure | no change | A past one I bought: 150 LEDs total, 30 LEDs/m, 5m long, 150 Lumens, 3000K, 12V power supply (current one has 24V input) |
| Microcontroller | Arduino Nano | no change |  |
| Real-Time Clock (RTC) | DS3231 w/ generic HW-084 board (LIR2032 battery) | [DS3231 w/ Adafruit 3013 board](https://www.adafruit.com/product/3013) (CR1220 battery) | Adafruit's board is significantly more compact |
| Screen | generic 0.91" 128x32 OLED | [Adafruit 0.91" 128x32 OLED](https://www.adafruit.com/product/4440) | Both use SSD1306 driver, switching to Adafruit offers ... |
| Buttons | Standard two-pin push buttons | no change |  |
| MOSFET (for PWM control) | Probably: IRLB8748 (International Rectifier, now Infineon), 30V 78-92A | RFP30N06LE (Fairchild Semiconductor, now ON Semiconductor) 60V 30A | Already have both of these in my kit. New one supports higher voltage |
| Barrel jacks | 2.1mm ID 5.5mm OD | no change | This is the standard for hobbyist low-voltage barrel connectors and matches the power supply I have. |
| LED strip power supply | 24V 1.5A output | no change | Entirely dependent on the LED strip being used |
| Arduino power supply | 5V USB-C | no change |  |


LED strip: 
- Generally they are 12-24V input so I want to support that range. 
- Currently I only plan to support LED strips that use a barrel jack input. Are there others? Probably ones that take USB input, but I don't know if they would support PWM-style dimming. Generally trying to avoid programmable strips that have their own controller/remote, color changing, etc. unless I do more research on them: I worry that I won't be able to do simple PWM on/off/dimming control.

Arduino power supply: 
- Currently requiring separate 5V USB-C power. Would be nice to remove this (see note at bottom).

Screen: 
- Would be cool to have a larger screen such as [Adafruit 0.96" 128x64](https://www.adafruit.com/product/326). Would require a larger case, which isn't a big deal. Bigger potential problem is the memory buffer required for a 128x64 screen is 1024 bytes, which is half of the total memory available on the Arduino Nano (2048 bytes).

Barrel jacks: 
- If at any point buying new barrel jacks, ensure 11mm inner depth. That is the length of the plugs I have for the LED strip and power supply, but apparently some barrel jacks are only 9-9.5mm deep.
- Verified inner diameter (ID) by inserting a 5/64" (1.98mm) drill bit, which fits, then trying a 3/32" (2.38mm) bit, which did not fit: rules out the common 2.5mm ID.
- If I switch to a 12V 3A LED strip, should probably upgrade barrel jacks to "high-current" since standard barrel jacks are limited to 2-2.5A.

## Notes on wiring

Include a 10k pull-down resistor between Gate to Source/GND on the MOSFET to keep it firmly off while the Arduino Nano is booting and its pins are floating. Missing this could cause the LED strip to flash on at full brightness on power-up.

Include a 220-330 Ohm resistor in series with Arduino PWM to Gate. This is because the MOSFET gate behaves like a capacitor and w/o a series resistor the PWM edges draw a sharp current spike from the Arduino pin.

## Notes on PCB design

Using EasyEDA desktop version (projects offline) to design the PCB.

Place barrel jack on PCB with 1-2mm of overhang off side of PCB, possibly. 


## Notes on firmware

Full firmware is implemented in `firmware.ino`.


## Current project status

- Light alarm is functioning and has been functioning for over 2 years. RTC successfully maintains current time when power is disconnected/reconnected (using rechargeable coin cell battery). Alarm triggers at correct time and gradual brightening works.
- Hardware is stable/reliable: 
    - Soldered on two protoboards connected vertically with screws and standoffs
    - Barrel jacks are hot-glued in addition to soldering because they were wiggly after a while
    - Placed inside an incomplete and not fully enclosed 3D printed case 
- Recently did a major overhaul of the firmware moving it to a "menu -> option selections" design which makes it a little more intuitive and also allows more easily adding new features in the future.

## Current projects / next steps

Move the project from a soldered protoboard (somewhat bulky, two layers, has potential to break) to a slim and sturdy PCB housed in a 3D printed case:

1. Create EasyEDA components in my component library for each piece of light alarm.
2. Create EasyEDA wiring schematic using the components
3. Design and PCB: wiring based on EasyEDA schematic; try to make it compact and make button layout intuitive to use
4. Create 3D model of the case based on PCB dimensions and mounting holes
5. Order PCB
6. Solder PCB and test
7. Mount in the 3D printed case

Potential future improvements later on:

- Add a screen timeout to improve life of OLED screen
- Improve the button handling in the firmware so that it doesn't jump to "continuous" mode too fast on long-press, but once it does it increments/decrements more quickly. Could also disable "continuous" mode in a lot of scenarios which might be nice (such as turning the screen off/on).
- Figure out a way to reliably power the Arduino with the same power supply as the LED strip. Previously used a small power converter that was adjustable on some range (wish I could remember the model) but it burned out after some time.