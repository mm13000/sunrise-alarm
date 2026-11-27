# Sunrise Alarm PCB — Session Handoff & Resume Guide

> **Purpose:** Self-contained checkpoint for picking this project back up in a brand-new conversation or after a break. Upload this file alongside `docs/implementation_notes.md` and `docs/reference.md` to restore full context immediately.

---

## 1. Exact Current Progress (Where We Paused)

### Repository & Altium Project Setup — **DONE**
- Local Altium Designer project created and committed to GitHub at `hardware/altium/sunrise_alarm/sunrise_alarm.PrjPcb`.
- `.gitignore` is configured and verified (ignoring Altium's `History/`, `__Previews/`, logs, and lock files).
- Core project files created and linked:
  - `sunrise_alarm.SchDoc` & `sunrise_alarm.PcbDoc`
  - `Libraries/sunrise_alarm.SchLib` & `Libraries/sunrise_alarm.PcbLib`

### Component Library Progress (`sunrise_alarm.PcbLib` & `sunrise_alarm.SchLib`)
- **Adafruit 3013 DS3231 RTC (`ADAFRUIT_3013_DS3231`) — IN PROGRESS (~70% done on footprint):**
  - [x] Official `.STEP` file downloaded from Adafruit CAD Parts GitHub (`3013 DS3231 RTC`) and **embedded** (`Embed Model`) in `sunrise_alarm.PcbLib`.
  - [x] 8 through-hole header pads (`1`–`8`) placed at `2.54 mm` pitch (`1.0 mm` hole, `1.7 mm` copper pad, Pad 1 rectangular).
  - [x] 3D model aligned flat to board plane (`View -> 3D Layout Mode`, `Align Face with Board`).
  - [x] Unplated mechanical mounting hole pad created using **Designator = `0`** (Altium's newer Pad Stack UI rejects blank designators; `0` is the standard mechanical hole convention), **`2.5 mm` hole**, **`2.5 mm` copper X/Y**, **`Plated` unchecked**.
  - [ ] **NEXT IMMEDIATE STEP:** Align that first mounting hole (`0`) to the RTC's 3D mounting hole, duplicate it for the second mounting hole (verify pasted Designator stays `0`), and update the RTC 3D body's **`Standoff Height` to `11.0 mm`** (matching the mated `8.5 mm` female header + `2.54 mm` male spacer).
  - [ ] Draw the `Top Overlay` silkscreen outline (`23.0 mm × 17.6 mm`) + Pin 1 marker dot/label, then save `.PcbLib`.
  - [ ] Build the `ADAFRUIT_3013_DS3231` schematic symbol in `sunrise_alarm.SchLib` (Pins `1`–`8`: `VIN, GND, SCL, SDA, BAT, 32K, SQW, RST`) and link it to the footprint.
- **Remaining Components Not Yet Started in Altium:**
  - [ ] **Adafruit 4440 0.91" OLED** (Top-side, directly soldered — download `.STEP` from Adafruit CAD Parts GitHub folder `4440`).
  - [ ] **Arduino Nano** (Bottom-side, socketed on two `1×15` female headers — note: Arduino Nano has no mounting holes; held by the 30-pin headers).
  - [ ] **RFP30N06LE MOSFET** (Bottom-side, TO-220 — pull from Altium Manufacturer Part Search via **Download**, verify G/D/S pinout against `docs/datasheets/RFP30N06LE.pdf`, and consider laying flat 90° with keepout under its Drain-connected metal tab).
  - [ ] **Barrel Jacks (×2)** (Bottom-side, 2.1mm ID / 5.5mm OD, 11mm depth — **must use `Slot` hole type** for flat blade pins; measure with calipers or verify datasheet).
  - [ ] **Tactile Pushbuttons** (Top-side — depends on button style decision below).
  - [ ] **Resistors** ($10\text{ k}\Omega$ MOSFET gate pull-down, $220\text{--}330\ \Omega$ MOSFET gate series resistor).

---

## 2. Key Decisions Made This Session (Context Not Yet Fully in `implementation_notes.md`)

1. **Dual-Sided Architecture (UI on Top, Guts on Bottom):**
   - **Top of PCB:** Only the **Adafruit 4440 OLED** and **pushbuttons**.
   - **Bottom of PCB:** Arduino Nano, Adafruit 3013 RTC, RFP30N06LE MOSFET, barrel jacks, and resistors.
   - **Altium Rule:** Always build **all** footprints normally on the **Top Layer** in `.PcbLib`. When placing bottom-side components in `.PcbDoc`, drag the component and press **`L`** so Altium automatically flips and mirrors pads/silkscreen/3D model to the Bottom Layer.
2. **Standoff & Female Header Stackup (Verified):**
   - Verified the Adafruit 3013 mounting holes are **exactly `2.5 mm` diameter** (an M2.5 screw is zero-clearance/snug and may bind).
   - **Selected Hardware:** Imperial **`#2-56`** nylon hardware (`2.18 mm` screw OD → `0.32 mm` clean clearance in a `2.5 mm` hole).
     - **Standoffs:** **`7/16"` (`11.11 mm`)** `#2-56` nylon hex standoffs (McMaster `92319A255` or DigiKey RAF `2056-256-N`). *Note: Update `implementation_notes.md` BOM row which currently says "11mm nylon M2 standoffs" to `#2-56, 7/16" (11.11mm)` so the links and text match!*
     - **Screws:** **`#2-56` × `3/16"`** nylon pan-head Phillips screws (McMaster `94735A706`) — `3/16"` ensures two screws entering opposite ends of the `7/16"` standoff will not collide.
     - **Female Headers:** **Sullins** standard `8.50 mm` insulation height, `~3.0 mm` solder post length, tin-plated `Female Socket` (no US tariff surcharge on DigiKey):
       - 8-pin (RTC): `PPTC081LFBN-RC`
       - 15-pin (Arduino Nano, qty 2): `PPTC151LFBN-RC`
     - **Total Mated Gap:** `8.50 mm` (female header) + `2.54 mm` (male header plastic spacer) = **`11.04 mm`**, matching the `11.11 mm` standoff within `0.07 mm`.
3. **OLED Direct-Soldering & Underside Keepout:**
   - The OLED has no underside battery, so it will be **soldered directly** (~`5.5–6.0 mm` tall) instead of socketed (~`14 mm`), keeping it close to button height.
   - **Layout Rule (Enforce in `.PcbDoc`):** Do **not** place bottom-side through-hole component pins underneath the OLED's `Top Overlay` outline (clipped leads stick up `1–2 mm` and can short the back of the OLED or become impossible to rework). Routing copper traces on the bottom layer under the OLED is totally fine.

---

## 3. Open Decisions Still To Be Made

1. **Button Height & Enclosure UI Alignment:**
   - *Option A (Most polished):* Use short (`4.3–5 mm`) tactile switches on the PCB + design **3D-printed button plungers/caps** in Onshape integrated into the top lid.
   - *Option B (Fastest CAD):* Use taller **`8–9 mm`** `6×6 mm` tactile switches poking through simple round holes in a `2 mm` lid (roughly flush with the ~`6 mm` direct-soldered OLED).
2. **Mechanical Support for the Direct-Soldered OLED:**
   - Because the Adafruit 4440 only has header pins along one edge, decide whether to support the far end with a **3D-printed `2.54 mm` spacer** (bolted before soldering pins) or **downward ribs on the 3D-printed top lid**.
3. **MOSFET Orientation (`RFP30N06LE`):**
   - Decide whether to bend the TO-220 legs 90° to **lay it flat** against the bottom of the PCB (saves ~15 mm of enclosure thickness; requires keeping exposed pads/traces away from its Drain-connected metal tab).
4. **Power Supply (Single 24V vs. Dual 24V + 5V USB-C):**
   - Decide before schematic capture whether to keep separate 5V USB-C power for the Arduino Nano or integrate a reliable buck converter (e.g., Recom/Pololu 24V→5V module) on the PCB.
5. **Exact Pin Mapping (`config.h`):**
   - Before drawing `sunrise_alarm.SchDoc`, pull the exact pin assignments from `firmware/config.h` (confirming PWM pin is `D9`/`D10` Timer 1 and buttons use `INPUT_PULLUP` on valid pins).

---

## 4. Minor Cleanup Note for `docs/implementation_notes.md`
- In the **Components** table under **Standoffs**, the text currently says `11mm nylon M2 standoffs` while the links point to `#2-56` (`7/16"` / `11.11 mm`) standoffs and screws. Update the text to `#2-56, 7/16" (11.11mm) nylon standoffs` so there is no confusion when ordering.
