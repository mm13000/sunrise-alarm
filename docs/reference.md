# Notes on various options and resources for future use

## 3D Models, footprints, symbols

> **STEP vs. STL:** I should use `.STEP` files (not `.STL` files) for 3D PCB components. STL files convert geometry into a triangle mesh meant for 3D printing slicing, stripping away exact curves and faces. STEP files preserve exact parametric CAD geometry so things can be snapped and aligned properly in Altium and Onshape.

### Suppliers & Mechanical Sources (Best for Ground Truth)

| Source | What It Has | Good For | Notes |
|:---|:---|:---|:---|
| [DigiKey](https://www.digikey.com/) | Discrete components, datasheets, CAD links | Finding exact part numbers and specs | Product pages often link to [SnapMagic](https://www.snapeda.com/) or [Ultra Librarian](https://www.ultralibrarian.com/) |
| [Mouser](https://www.mouser.com/) | Discrete components, datasheets, CAD links | Finding exact part numbers and specs | Product pages often link directly to [SamacSys](https://supplyframe.com/samacsys) |
| [Adafruit CAD Parts GitHub](https://github.com/adafruit/Adafruit_CAD_Parts) | Official 3D `.STEP` & `.f3d` models | Adafruit breakout boards & sensors | |
| [McMaster-Carr](https://www.mcmaster.com/) | Dimensionally exact 3D `.STEP` models | Screws, standoffs, nuts, washers, heat-set inserts | `.STEP` downloads under Product Detail -> CAD |
| **Manufacturer Sites** (Same Sky/CUI, Molex, TE, Würth) | Official `.STEP` models & mechanical drawings | Electromechanical parts (barrel jacks, switches, connectors) | Best place to find exact PCB hole/slot cutout drawings alongside official 3D STEP files. |

### Aggregators & ECAD Libraries

| Source | What It Has | Good For | Notes |
|:---|:---|:---|:---|
| [Octopart](https://octopart.com/) (owned by Altium) | Datasheets, pricing, stock, some CAD links | Standard discrete parts (resistors, buttons, barrel jacks, MOSFETs) | Search engine that aggregates across suppliers (DigiKey, Mouser, Newark). Powers Altium's built-in "Manufacturer Part Search". |
| **"Manufacturer Part Search"** (inside Altium Designer) | Symbols, footprints, 3D models | Exact manufacturer discrete parts | Uses Octopart under the hood. A **green chip icon** indicates a symbol and footprint (and usually 3D model) are available. Use **Download** (to add to local library), not "Acquire" (cloud). |
| [SnapMagic](https://www.snapeda.com/) (formerly SnapEDA) | Symbols, footprints, some 3D models | Breakout modules, ICs, and parts missing from Altium MPS | Largest vendor- and community-supported ECAD library. Mixes manufacturer-verified models with community/automated ones—look for the **"Verified"** badge and check dimensions. |
| [Ultra Librarian](https://www.ultralibrarian.com/) | Symbols, footprints, 3D models | TI/Analog Devices ICs, DigiKey-linked parts | Integrated into DigiKey. Tends to provide clean, IPC-compliant footprints with direct Altium exports. |
| [SamacSys](https://supplyframe.com/samacsys) | Symbols, footprints, 3D models | Mouser-linked parts | Pushes its "Library Loader" background utility. Would need to skip the utility and download the raw files directly. |
| [GrabCAD](https://grabcad.com/) | 3D `.STEP` / CAD files only | Complete assemblies (Arduino Nano, OLED modules, power supplies) | Community-uploaded mechanical models. Anyone can upload, so verify dimensions. |


## Altium Designer workflow

### Project architecture
- **`.PrjPcb` (Project File):** Plain-text index file that stores project settings and relative paths to design files and libraries.
- **`.SchDoc` & `.PcbDoc`:** Binary schematic and PCB layout files.
- **`.SchLib` & `.PcbLib`:** Custom local schematic symbol and PCB footprint libraries.
- **`.OutJob`:** Output Job configuration file for repeatable exports (Gerbers, NC Drill, BOM, 3D STEP).

### Managing downloaded parts
Downloading a part from SnapMagic or Ultra Librarian gives a standalone `.SchLib`/`.PcbLib` file (or a script that generates one) for that single component. 

I should open the downloaded library file in Altium, copy the component/footprint from its library panel, and paste it into my centralized project `.SchLib` and `.PcbLib` files. Then delete the temporary downloaded files so the repo stays clean.

### Footprint & symbol creation
- **Grid Shortcut (`G`):** Press `G` to change the snap grid. Use `2.54mm` (`100mil`) when placing standard header pads, and `0.1mm` for board outlines or metric features.
- **2D / 3D View Toggle (`2` / `3`):** Press `3` in the footprint or PCB editor to inspect in 3D (`Shift + Right-Click` to orbit). Use `Tools -> 3D Body Placement -> Align Face with Board` to snap a `.STEP` model flush with the PCB surface. Press `2` to return to 2D.
- **Schematic Pin Orientation:** When placing pins in `.SchLib`, the small **white X** (electrical hot spot) at the end of the pin must point **outward** away from the component body.
- **Pad vs. Pin Matching:** The pin **Designator** in `.SchLib` (`1`, `2`, `3`...) must match the pad **Designator** in `.PcbLib`. Always cross-check against the PDF datasheet before fabrication.


## Arduino Nano pinout / reference

| Pin(s) | Functions & Capabilities | Hardware Notes & Gotchas |
|:---|:---|:---|
| `5V` | Regulated 5V power output (or input) | Can power low-current peripherals like the DS3231 RTC and OLED display. |
| `VIN` | Unregulated power input (`7–12V` recommended) | Feeds the onboard linear regulator. Avoid high current draw at 12V+ due to thermal dissipation. |
| `GND` | Ground | Must be tied common across all power supplies, sensors, and low-side switches. |
| `TX1`, `RX0` | Hardware UART Serial (`D0`, `D1`) | Avoid using for general I/O. Reserved for USB-to-Serial programming and debugging. |
| `D2 – D12` | • Digital I/O (with internal `INPUT_PULLUP`) <br> • Pin Change Interrupts (`PCINT`) | Internal pull-up resistors are $20\text{--}50\text{ k}\Omega$. |
| `D2`, `D3` | External Hardware Interrupts (`INT0`, `INT1`) | Best pins for wake-up interrupts (such as the RTC `SQW` alarm interrupt pin). |
| `D9`, `D10` | PWM output at `490.20 Hz`, Timer 1, 16-bit | **Best choice for LED dimming.** 16-bit timer allows custom high-frequency, high-resolution, flicker-free PWM. |
| `D3`, `D11` | PWM output at `490.20 Hz`, Timer 2, 8-bit | Good secondary hardware PWM pins. |
| `D5`, `D6` | PWM output at `976.56 Hz`, Timer 0, 8-bit | Shared with `millis()` and `delay()`. *Note:* `analogWrite(pin, 0)` has a known hardware quirk where it may still output narrow spikes rather than a true 0% duty cycle. |
| `D13` | Digital I/O (Built-in LED) | Tied to onboard LED + series resistor to `GND`. **Do not use for `INPUT_PULLUP` buttons** (the LED pulls the pin down); fine for status output. |
| `A0 – A5` | • Analog Input <br> • Digital I/O (`INPUT_PULLUP`) <br> • Pin Change Interrupts (`PCINT`) | Identical to `D2–D12` for digital I/O and `PCINT` when not needed for ADC reads. |
| `A4`, `A5` | Hardware I2C Bus (`A4` = `SDA`, `A5` = `SCL`) | Dedicated Two-Wire (I2C) communication pins for RTC and OLED modules. |
| `A6`, `A7` | Analog Input ONLY | Have no digital input buffers and can't be used for `digitalRead()`, `INPUT_PULLUP` buttons, or `digitalWrite()`. |
