# Project: Arduino Sunrise Alarm Clock

Source of truth for design decisions, component selections, and implementation checklists.

Goal for this project is to implement a programmable light alarm, which gradually brightens an LED strip over a user-defined time. Gradual brightening is accomplished using a PWM signal with a slowly increasing duty cycle. The user should be able to set the clock time, alarm time, and alarm duration.

## Status

### Oct 2026
- **Functional Prototype:** Light alarm is functioning and has been running reliably for over 2 years. The RTC successfully maintains current time when power is disconnected/reconnected (using its coin cell battery). The alarm triggers at the correct time and gradual brightening works.
- **Current Hardware:**
  - Soldered onto two protoboards which are connected vertically with screws and standoffs.
  - Barrel jacks are hot-glued in addition to soldering because they became wiggly over time.
  - Housed inside 3D printed case (incomplete- not fully enclosed).
- **Firmware:** Recently did a major overhaul of `firmware.ino`, moving to a `menu -> options` UI design that is more intuitive and makes it easier to add new features/options.
- **Documentation:** Added this documentation file to track design decisions and progress. Also added `docs/reference.md` for keeping general notes as I learn more about Arduino, various components, etc.

## Next steps
Move the project from a two-layer soldered protoboard (bulky, fragile) to a PCB (slimmer and sturdier) housed in a more refined 3D-printed case. Also move to Altium Designer (previously tried using EasyEDA desktop in projects offline mode).

- Set up local Altium Designer project (`Sunrise_Alarm.PrjPcb`) and custom library files (`.SchLib` and `.PcbLib`).
- Gather or create schematic symbols, PCB footprints, and 3D `.STEP` models for each component.
- Create the circuit wiring schematic (`.SchDoc`).
- Design the PCB layout (`.PcbDoc`). Goal is a compact board with an intuitive button layout and solid mechanical mounting for the barrel jacks, as well as mounting holes or mechanism for the 3D-printed case.
- Export the PCB 3D `.STEP` model and design the 3D-printed enclosure around it.
- Order PCB, solder components, test, etc.

## Things I still need to look into / decide on:
- **Buttons:** Should I use the same two-pin push buttons, or switch to a different tactile button type for better feel/look? Or should I make the 3D case have "button covers" that move to push the buttons (though this could be easily breakable)?
- **Power Supply:** Should I power the Arduino Nano from the 12–24V LED strip power supply so a separate 5V USB-C cable isn't required? *(Previously tried a small adjustable buck converter module, but it burned out over time. Would need a reliable, well-rated step-down regulator if revisited.)*
- 


## Ideas for future improvements

### Firmware:
- **Exponential/Logarithmic Dimming Curve:** Change the brightening curve from linear to exponential or logarithmic. The current linear curve feels too abrupt at the very beginning and too slow near the end.
- **OLED Screen Timeout:** Add an automatic screen timeout to prevent OLED burn-in and extend display lifespan.
- **Refined Button Long-Press Acceleration:** Adjust button handling so it waits slightly longer before entering "continuous" increment/decrement mode on a long press, and increments faster once engaged. Disable continuous mode on binary toggles (e.g., turning the screen on/off).

### Hardware:
- **Single Power Supply (Buck Converter):** Power the Arduino Nano from the 12–24V LED strip power supply so a separate 5V USB-C cable isn't required. *(Previously tried a small adjustable buck converter module, but it burned out over time—will need a reliable, well-rated step-down regulator if revisited.)*


---

## Components

All components will be hand-soldered, so they must be through-hole or larger footprints/breakout modules.

| Component | Protoboard (current prototype) | PCB (planned) | Notes |
|:---|:---|:---|:---|
| **LED Strip** | 24V input LED strip | No change | For reference, one past strip I bought (not the current one I have): 150 LEDs total, 30 LEDs/m, 5m long, 150 Lumens, 3000K, 12V power supply (the current strip uses 24V input). |
| **Microcontroller** | Arduino Nano w/ ATmega328P (*not* "old bootloader") | No change | |
| **Real-Time Clock (RTC)** | DS3231 w/ generic HW-084 board (LIR2032 battery) | [DS3231 w/ Adafruit 3013 board](https://www.adafruit.com/product/3013) (CR1220 battery) | Adafruit's board is significantly more compact than generic version. Yay! |
| **Screen** | Generic 0.91" 128x32 OLED | [Adafruit 0.91" 128x32 OLED (4440)](https://www.adafruit.com/product/4440) | Both use SSD1306 driver; switching to Adafruit gives official 3D CAD models. |
| **Buttons** | Standard two-pin push buttons | No change |  |
| **MOSFET (PWM control)** | Likely IRLB8748 (by Infineon), 30V 78–92A | RFP30N06LE (by ON Semiconductor), 60V 30A | Already have both in kit; but the RFP30N06LE supports higher voltage (`docs/datasheets/RFP30N06LE.pdf`). |
| **Barrel Jacks** | 2.1mm ID, 5.5mm OD | No change | Standard hobbyist low-voltage barrel connector; matches my current power supply and LED strip plugs. |
| **LED Power Supply** | 24V 1.5A output | No change | Of course this is always dependent on the LED strip being used. |
| **Arduino Power Supply** | 5V USB-C | No change | Currently separate from LED power supply (see section above on ideas for future improvements). |
| **Standoffs** | none | 11mm nylon M2 standoffs | Also use nylon screws or at least nylon washers. McMaster-Carr standoffs: [McMaster standoff](https://www.mcmaster.com/92319a255/) / [Digikey standoff](https://www.digikey.com/en/products/detail/raf-electronic-hardware/2056-256-N/7681362) / [McMaster screw 100 pack](https://www.mcmaster.com/94735A706-93135A012/) |
| **Female header sockets** | not sure | 8.5mm length plastic body | Combined with standard 2.54mm plastic body of male spacer pins this makes ~11mm total standoff distance of PCB to breakout boards. [Digikey search](https://www.digikey.com/en/products/filter/headers-receptacles-female-sockets/315?s=N4IgjCBcoEwBxVAYygMwIYBsDOBTANCAPZQDaIAzAKzUAMA7CALqEAOALlCAMrsBOASwB2AcxABfQgDYAnIhApIGHAWJkQAFjBg4MxixAcuvQaImE4CaArRY8hEpHLwwW2iEJ6qYCszadIHn5hMUkQKl15RWV7NSdNWipaWhg-QwCg01DCMHoZd2tou1VHcj0YKTSjQJMQ83AZSMLbFQd1dyYwmFowVOalYrb4sFoKWgQc2ikaD3BxyyqM2rMunqtkFtjShopInJld-X9jYJXCGFy%2BjYHWuPIIA2qQADkAVwBbXEEUMIBaK5skH4rxK6iozHEXXUqFw7ywuAABAALXDoAAmX2wEKAA) / [Digikey 8 pin](https://www.digikey.com/en/products/detail/sullins-connector-solutions/PPTC081LFBN-RC/810147) / [Digikey 15 pin](https://www.digikey.com/en/products/detail/sullins-connector-solutions/PPTC151LFBN-RC/810153) |

### Component Design Notes & Constraints
- **LED Strip:**
  - Generally 12–24V input, so the PCB should safely support that voltage/current range.
  - Currently only supporting non-addressable LED strips with a barrel jack connection that accept simple PWM dimming. Avoiding programmable/remote-controlled RGB strips for now until further research.
- **Barrel Jacks:**
  - **Depth:** If buying new barrel jacks, ensure **11mm inner depth** (matches existing plugs; some jacks are only 9–9.5mm deep).
  - **Diameter Verification:** To verify my barrel jack ID/OD, I used a 5/64" (1.98mm) drill bit, which fits, while a 3/32" (2.38mm) drill bit did not fit (ruling out 2.5mm ID).
  - **Current Rating:** Standard barrel jacks are rated around 2–2.5A. If upgrading to a 12V 3A LED strip later, upgrade to high-current barrel jacks.
- **Screen Upgrade Trade-off:**
  - I considered upgrading to a larger [Adafruit 0.96" 128x64 OLED](https://www.adafruit.com/product/326). Enclosure size isn't an issue, but a 128x64 display requires a 1024-byte memory buffer, which is half of the Arduino Nano's 2048 bytes of SRAM. Sticking with 128x32 for now. If I want to upgrade in the future, probably need to upgrade the microcontroller. Worth looking into probably (or at least interesting/fun to research).

---

## Schematic & PCB

Buttons and screen should be mounted to top of PCB, while everything else gets mounted to bottom. This is so that the buttons and screen are accessible through the 3D printed enclosure without being crowded by the other components. Admittedly, they will still need to avoid the through-hole solder pads of the other components coming up through the bottom of the PCB.

Pin headers and standoffs: I plan to use these for the RTC and Arduino on the underside of the PCB, but not for the OLED screen on the top, so that the OLED is more or less on the same plane as the buttons. Hopefully a better user interface that way - not an awkward plane difference where the screen sticks out more than the buttons.






### Schematic / wiring
- **Common Ground:** Connect `GND` across all subsystems (Arduino, RTC, OLED, LED power supply ground, MOSFET source/pull-down resistor, and buttons).
- **MOSFET Gate Pull-Down Resistor ($10\text{ k}\Omega$):** Place a $10\text{ k}\Omega$ resistor between MOSFET Gate and Source/GND to keep the MOSFET firmly off while the Arduino Nano boots and its pins float (prevents the LED strip from flashing at full brightness on power-up).
- **MOSFET Gate Series Resistor ($220\text{--}330\ \Omega$):** Place a $220\text{--}330\ \Omega$ resistor in series between the Arduino PWM output pin and the MOSFET Gate to limit sharp capacitive current spikes on PWM edges.
- **Buttons via `INPUT_PULLUP`:** Wire pushbuttons between digital input pins (`D2–D12` or `A0–A5`, excluding `D13`, `A6`, `A7`) and `GND`. The next firmware/hardware iteration will use the ATmega328P's internal $20\text{--}50\text{ k}\Omega$ pull-up resistors (`INPUT_PULLUP`, pin normally `HIGH`, goes `LOW` when pressed). This eliminates external pull-down resistors on each button!
- **PWM Pin Selection:** Route the MOSFET gate drive to `D9` or `D10` (16-bit Timer 1) if possible for flicker-free high-frequency PWM, or `D3`/`D11` (Timer 2). Avoid `D5`/`D6` (Timer 0 quirk where `analogWrite(pin, 0)` may not produce true 0% duty cycle).
- **I2C Bus (`A4` / `A5`):** Route `A4` (`SDA`) and `A5` (`SCL`) to both the DS3231 RTC and SSD1306 OLED.

### Footprint / PCB layout
- **Verify Pinouts Against Datasheets:** Double-check all downloaded symbols/footprints against datasheets—especially **Gate / Drain / Source** pin numbers on the `RFP30N06LE` MOSFET.
- **Slotted Holes for Barrel Jacks:** Ensure the barrel jack footprint uses **slotted holes** (`Slot` hole type in Altium) for its flat blade pins rather than round holes. Verify the slot dimensions against the datasheet or caliper measurements.
- **Barrel Jack Overhang:** Consider placing the barrel jacks along the board edge with **1–2mm of overhang** off the side of the PCB for clean mating through the thickness of the 3D printed enclosure. Consider that this might cause difficulties with inserting the soldered/assembled PCB into the enclosure.

---

## Firmware

Full firmware is implemented in `firmware/firmware.ino` and `firmware/config.h`.