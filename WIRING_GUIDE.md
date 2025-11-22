# OpenHoop Wiring Guide

This guide provides detailed, step-by-step wiring instructions for building your OpenHoop. Each stage includes testing to ensure everything works before moving to the next step.

## Table of Contents

- [Before You Start](#before-you-start)
- [Pin Reference Tables](#pin-reference-tables)
- [Stage 1: Microcontroller USB Power & BLE Test](#stage-1-microcontroller-usb-power--ble-test)
- [Stage 2: LED Strip Connection (USB Powered)](#stage-2-led-strip-connection-usb-powered)
- [Stage 3: Battery & BMS Installation](#stage-3-battery--bms-installation)
- [Stage 4: Buck Converter & Full Power System](#stage-4-buck-converter--full-power-system)
- [Stage 5: Final Assembly & Testing](#stage-5-final-assembly--testing)
- [Troubleshooting](#troubleshooting)

---

## Before You Start

### Choose Your LED Strip Type

OpenHoop supports two types of LED strips. Configure in `include/Config.h`:

**Option 1: NeoPixel (WS2812B)** - *Recommended for beginners*
- ✅ Single data wire (simpler wiring)
- ✅ More common and cheaper
- ✅ Widely available
- ⚠️ Slightly lower refresh rate

**Option 2: DotStar (APA102)**
- ✅ Higher refresh rates (better for POV)
- ✅ More reliable signal transmission
- ⚠️ Requires two wires (data + clock)
- ⚠️ More expensive

Edit `include/Config.h` and uncomment the appropriate line:
```cpp
// #define USE_DOTSTAR    // Uncomment for DotStar/APA102
#define USE_NEOPIXEL   // Uncomment for NeoPixel/WS2812B
```

### Choose Your Microcontroller

Both boards work with either LED type:

- **Seeed XIAO BLE Sense** (Recommended) - Smaller, cheaper, newer
- **Arduino Nano 33 BLE Sense Rev2** (Original design) - Larger, more pins

---

## Pin Reference Tables

### Seeed XIAO BLE Sense Pinout

| Function | Pin Number | Pin Label | Notes |
|----------|------------|-----------|-------|
| **Power** |
| USB 5V Input | - | 5V | When connected via USB-C |
| 5V Output | - | 5V | Can supply 5V to external devices |
| 3.3V Output | - | 3V3 | Regulated 3.3V output |
| Ground | - | GND | Multiple GND pins available |
| **LED Strip** |
| NeoPixel Data | D10 | D10/A10 | For WS2812B (1-wire) |
| DotStar Data | D10 | D10/A10 | For APA102 (2-wire) |
| DotStar Clock | D8 | D8/A8 | For APA102 only |
| **Battery Monitoring** |
| Battery Voltage Sense | A0 | A0/D0 | Via voltage divider (30kΩ + 7.5kΩ) |

**XIAO Power Notes:**
- Built-in USB-C charging circuit is for small 1S Li-Po only
- For 2S battery packs, use external charging
- Can be powered from 5V pin (from buck converter)

### Arduino Nano 33 BLE Sense Rev2 Pinout

| Function | Pin Number | Pin Label | Notes |
|----------|------------|-----------|-------|
| **Power** |
| USB 5V Input | - | VIN | When connected via Micro-USB |
| 5V Output | - | 5V | Can supply 5V to external devices |
| 3.3V Output | - | 3.3V | Regulated 3.3V output |
| Ground | - | GND | Multiple GND pins available |
| **LED Strip** |
| NeoPixel Data | D11 | D11 | For WS2812B (1-wire) |
| DotStar Data | D11 | D11 | For APA102 (2-wire) |
| DotStar Clock | D13 | D13 | For APA102 only |
| **Battery Monitoring** |
| Battery Voltage Sense | A7 | A7 | Via voltage divider (30kΩ + 7.5kΩ) |

**Note:** Update `BATTERY_ANALOG_PIN` in `Config.h` based on your board:
- XIAO: `#define BATTERY_ANALOG_PIN 0` (A0/D0)
- Nano 33: `#define BATTERY_ANALOG_PIN 21` (A7)

---

## Stage 1: Microcontroller USB Power & BLE Test

**Goal:** Verify the microcontroller works and BLE connects *before* connecting any hardware.

### What You Need
- Microcontroller (XIAO BLE Sense or Nano 33 BLE)
- USB cable (USB-C for XIAO, Micro-USB for Nano 33)
- Computer with PlatformIO

### Steps

1. **Flash the firmware:**
   ```bash
   # For XIAO BLE Sense:
   pio run -e xiao_ble_sense -t upload

   # For Arduino Nano 33 BLE:
   pio run -e nano33ble -t upload
   ```

2. **Open Serial Monitor:**
   ```bash
   pio device monitor -b 115200
   ```

3. **Verify boot:**
   - You should see startup messages
   - Random boot effect will be selected
   - Note: LEDs won't light up yet (nothing connected)

4. **Test BLE Connection:**
   - Use a BLE scanner app on your phone (like nRF Connect)
   - Look for device named "OpenHoop" or "HulaHoopBLE"
   - Connect and verify you can see BLE services
   - Service UUIDs: `0A92` (effects), `0A93` (color), `0A95` (energy)

### ✅ Success Criteria
- [x] Firmware uploads without errors
- [x] Serial monitor shows boot messages
- [x] BLE device is discoverable and connectable
- [x] No smoke or unusual heat

**If this works, proceed to Stage 2!**

---

## Stage 2: LED Strip Connection (USB Powered)

**Goal:** Get the LEDs working with USB power *only* (low brightness, limited pixel count).

### What You Need
- Microcontroller (already flashed from Stage 1)
- LED strip (NeoPixel or DotStar)
- Jumper wires
- USB power

### Wiring Diagrams

#### Option A: NeoPixel (WS2812B) - 1-Wire Connection

```
XIAO BLE Sense:                         NeoPixel Strip:
┌─────────────┐                         ┌──────────────┐
│             │                         │              │
│     3V3 ────┼─────────────────────────┼─── VCC (5V)  │
│             │                         │              │
│     GND ────┼─────────────────────────┼─── GND       │
│             │                         │              │
│  D10/A10 ───┼─────────────────────────┼─── DIN       │
│             │                         │              │
└─────────────┘                         └──────────────┘
```

**NeoPixel connections:**
- LED Strip VCC → XIAO 3V3 (or 5V for brighter, but test carefully)
- LED Strip GND → XIAO GND
- LED Strip DIN (Data In) → XIAO D10/A10

#### Option B: DotStar (APA102) - 2-Wire Connection

```
XIAO BLE Sense:                         DotStar Strip:
┌─────────────┐                         ┌──────────────┐
│             │                         │              │
│     3V3 ────┼─────────────────────────┼─── VCC (5V)  │
│             │                         │              │
│     GND ────┼─────────────────────────┼─── GND       │
│             │                         │              │
│  D10/A10 ───┼─────────────────────────┼─── DATA      │
│             │                         │              │
│   D8/A8 ────┼─────────────────────────┼─── CLK       │
│             │                         │              │
└─────────────┘                         └──────────────┘
```

**DotStar connections:**
- LED Strip VCC → XIAO 3V3 (or 5V for brighter, but test carefully)
- LED Strip GND → XIAO GND
- LED Strip DATA → XIAO D10/A10
- LED Strip CLK (Clock) → XIAO D8/A8

### ⚠️ Important USB Power Limitations

**USB can only provide ~500mA safely!**

To test safely with USB power:

1. **Limit the number of LEDs** in `Config.h`:
   ```cpp
   #define NUM_LEDS 30  // Temporarily reduce from 288
   ```

2. **Set low energy mode** via BLE:
   - Connect with BLE app
   - Set energy saving mode to 8 (lowest power)
   - Or edit `Config.h`: `#define DEFAULT_ENERGY_SAVING_MODE 8`

3. **Use 3.3V instead of 5V** for initial testing (dimmer but safer)

### Steps

1. **Update Config.h** for testing:
   ```cpp
   #define NUM_LEDS 30  // Test with fewer LEDs
   #define DEFAULT_ENERGY_SAVING_MODE 8  // Lowest power mode
   ```

2. **Re-upload firmware** with new settings

3. **Wire LED strip** according to diagram above

4. **Connect USB** and power on

5. **Verify LEDs light up:**
   - Should see boot animation (rainbow, fire, etc.)
   - LEDs will be dim (normal for USB power)
   - Test BLE control: send color commands

### ✅ Success Criteria
- [x] First 30 LEDs light up with boot effect
- [x] Can change colors via BLE
- [x] No flickering or unstable behavior
- [x] USB port doesn't get hot

### 🔧 Troubleshooting
- **No LEDs:** Check data pin number in Config.h matches wiring
- **Wrong colors:** Try different `LED_COLOR_ORDER` in Config.h (NEO_GRB, NEO_RGB, etc.)
- **Flickering:** Add 1000µF capacitor between VCC and GND on LED strip
- **Only first LED works:** Check data connection is solid

**If this works, proceed to Stage 3!**

---

## Stage 3: Battery & BMS Installation

**Goal:** Install batteries with BMS protection for portable operation.

### What You Need
- 2S Li-Po battery pack(s) - 7.4V nominal
  - Option 1: 2x 1500mAh packs (distributed)
  - Option 2: 3x 1000mAh packs (distributed)
  - Option 3: 1x 2500-3000mAh pack (centralized)
- 2S BMS protection board(s) - one per battery pack
- XT30 or XT60 connectors (recommended)
- Wire (18-22 AWG for battery connections)
- Heat shrink tubing
- Soldering iron

### Understanding 2S Battery Configuration

**2S = Two Cells in Series:**
- Each Li-Po cell: 3.7V nominal (3.0V min, 4.2V max)
- 2S pack: 7.4V nominal (6.0V min, 8.4V max)

**Why we need BMS (Battery Management System):**
- ⚠️ **Over-discharge protection:** Cuts power below 6.0V (prevents damage)
- ⚠️ **Overcurrent protection:** Protects against shorts
- ⚠️ **Cell balancing:** Keeps both cells at same voltage
- ✅ **Essential for safety** - Li-Po fires are serious!

### BMS Wiring Diagram

```
2S Li-Po Pack:          2S BMS Board:           To System:
┌──────────┐           ┌──────────┐            ┌──────────┐
│          │           │          │            │          │
│  Cell 1  ├───(+)─────┤ B+       │            │          │
│  3.7V    │           │          │            │          │
│          ├───(bal)───┤ B1       │            │          │
│          │           │          │            │          │
│  Cell 2  ├───(bal)───┤ B2    P+ ├────(+)─────┤ Buck IN+ │
│  3.7V    │           │          │            │          │
│          ├───(-)─────┤ B-    P- ├────(-)─────┤ Buck IN- │
│          │           │          │            │          │
└──────────┘           └──────────┘            └──────────┘
     │                                              │
     └──────(balance connector)────────────────────┘
```

### BMS Connection Steps

1. **Identify BMS terminals:**
   - **B+, B-, B1, B2** (or similar): Battery side
   - **P+, P-** (or OUT+, OUT-): Load/output side
   - **Balance connector:** 3-pin JST for cell balancing

2. **Connect battery to BMS:**
   - Battery + (red) → BMS B+
   - Battery - (black) → BMS B-
   - Balance connector → BMS balance port
   - **⚠️ Check polarity carefully! Wrong polarity = fire hazard**

3. **Connect BMS output to system:**
   - BMS P+ (output +) → Will connect to buck converter in Stage 4
   - BMS P- (output -) → Will connect to buck converter in Stage 4

4. **Test BMS protection:**
   - Use multimeter to measure voltage at P+ and P-
   - Should read 7.4V - 8.4V depending on charge level
   - If reads 0V, BMS may be in protection mode (try charging)

### Multiple Battery Pack Wiring

If using 2 or 3 packs for weight distribution:

```
Pack 1 BMS:      Pack 2 BMS:      Combined Output:
┌──────┐        ┌──────┐         ┌────────────┐
│      │        │      │         │            │
│  P+ ─┼────┬───┼─ P+  │         │  To Buck   │
│      │    │   │      │    ────→│  Converter │
│  P- ─┼────┴───┼─ P-  │         │  Input     │
│      │        │      │         │            │
└──────┘        └──────┘         └────────────┘
```

**Parallel connection rules:**
- All positives connected together
- All negatives connected together
- Capacities add: 2x 1500mAh = 3000mAh total
- Each pack needs its own BMS!

### Voltage Divider for Battery Monitoring

To monitor battery voltage with the microcontroller:

```
Battery +7.4V ────┬─────[ 30kΩ R1 ]─────┬───── ADC Pin (A0/D0)
                  │                     │
                  │                     └─[ 7.5kΩ R2 ]─── GND
                  │
              (Optional: XT60 connector for external charging)
```

**How it works:**
- 30kΩ + 7.5kΩ = 37.5kΩ total
- Divider ratio = 37.5 / 7.5 = 5
- 8.4V battery → 1.68V at ADC pin (safe for 3.3V ADC)
- Firmware converts back to actual voltage

**Assembly:**
1. Solder 30kΩ resistor between battery + and ADC pin
2. Solder 7.5kΩ resistor between ADC pin and GND
3. Use heat shrink to insulate connections
4. Update `BATTERY_ANALOG_PIN` in Config.h (A0 for XIAO, A7 for Nano 33)

### ✅ Success Criteria
- [x] BMS outputs correct voltage (7.4V - 8.4V)
- [x] No sparks or smoke during connection
- [x] BMS protection can be tested (short P+ to P- briefly should trip protection)
- [x] Voltage divider reads reasonable value on serial monitor

### ⚠️ Safety Warnings
- **Never** short battery terminals
- **Always** connect BMS before connecting load
- **Double-check** polarity before applying power
- Work on non-flammable surface
- Have fire extinguisher nearby when testing batteries

**If this works, proceed to Stage 4!**

---

## Stage 4: Buck Converter & Full Power System

**Goal:** Complete the power system with buck converter to power full LED strip.

### What You Need
- DC-DC Buck converter (7.4V → 5V, 3-5A rating)
- All components from previous stages
- 1000µF capacitor (for LED power smoothing)
- On/off toggle switch
- Wire and connectors

### Why Buck Converter?

**Problem:** LEDs need 5V, but batteries provide 7.4V (and vary 6.0-8.4V)

**Solution:** Buck converter steps down voltage efficiently
- Input: 6.0V - 8.4V (from 2S battery)
- Output: 5.0V regulated (stable for LEDs)
- Efficiency: 85-95% (much better than linear regulator)
- Current: 3-5A (enough for 288 LEDs)

### Complete System Wiring Diagram

```
                                    ┌─────────────────────────────────────────┐
                                    │         COMPLETE SYSTEM DIAGRAM         │
                                    └─────────────────────────────────────────┘

2S Battery Pack(s)          BMS Protection          Buck Converter          LED Strip
┌──────────────┐           ┌─────────────┐         ┌──────────────┐        ┌─────────────┐
│              │           │             │         │              │        │             │
│   7.4V       │           │  Over-      │         │  Input:      │        │  144-288    │
│   Li-Po      │           │  discharge  │         │  6-8.4V      │        │  LEDs       │
│              │           │  Overcurrent│         │              │        │             │
│  Cell 1 ─────┼───(+)─────┤ B+          │         │              │        │             │
│  3.7V        │           │             │         │              │        │             │
│              ├───(bal)───┤ B1          │    ┌────┤ Output:      ├───(+)──┤ VCC         │
│              │           │             │    │    │  5.0V        │        │             │
│  Cell 2 ─────┼───(bal)───┤ B2       P+ ├────┤    │  3-5A        │        │             │
│  3.7V        │           │             │    │    │              │        │             │
│              ├───(-)─────┤ B-          │    │    └───────┬──────┘        │             │
│              │           │             │    │            │               │             │
│              │           │          P- ├────┼────────────┴───────(-)─────┤ GND         │
└──────────────┘           └──────┬──────┘    │                            │             │
      │                           │           │            ┌───────────────┤ DATA IN     │
      │                           │           │            │               │             │
      │                           │           │         [XIAO BLE Sense]   │  (DotStar:) │
      │                           │           │            │               │  CLOCK IN   │
      │                           │           │            ├── D10/A10 ────┤             │
      │                    [Voltage Divider]  │            │               │             │
      │                           │           │            ├── D8/A8 ──────┤ (optional)  │
      │                      30kΩ │           │            │               │             │
      └───────────────────────────┴───────────┴────────────┤ 5V (power in) │             │
                                  │           └────────────┤               │             │
                                7.5kΩ                      │               │             │
                                  │                        ├── A0/D0       │             │
                                  │                        │  (bat sense)  │             │
                                 GND ──────────────────────┤ GND           │             │
                                                           │               │             │
                                                           └───────────────┴─────────────┘
                                                                  │
                                                            [1000µF Cap]
                                                            (across 5V/GND)

Optional Power Switch: ────┬─── Between Battery+ and BMS B+
```

### Step-by-Step Assembly

#### 1. Buck Converter Setup

1. **Adjust output voltage (BEFORE connecting anything!):**
   - Power buck converter from a bench supply or 7.4V battery
   - Use multimeter to measure output voltage
   - Adjust potentiometer to exactly 5.0V
   - **⚠️ Critical:** 5V±0.1V only! Too high damages LEDs, too low causes flickering

2. **Test buck converter load:**
   - Connect a resistor or small load (100Ω, 1W)
   - Verify voltage stays at 5.0V under load
   - Check converter doesn't overheat

#### 2. Main Power Connections

**Follow this order:**

1. **Battery → BMS** (already done in Stage 3)
   - Battery + → BMS B+
   - Battery - → BMS B-
   - Balance connector → BMS balance port

2. **BMS → Buck Converter Input**
   - BMS P+ → Buck IN+
   - BMS P- → Buck IN-

3. **Buck → LED Strip Power**
   - Buck OUT+ (5V) → LED strip VCC
   - Buck OUT- (GND) → LED strip GND
   - **Add 1000µF capacitor** between 5V and GND at LED strip

4. **Buck → Microcontroller Power**
   - Buck OUT+ (5V) → XIAO 5V pin
   - Buck OUT- (GND) → XIAO GND

5. **Microcontroller → LED Strip Data**
   - For NeoPixel: XIAO D10 → LED DIN
   - For DotStar: XIAO D10 → LED DATA, XIAO D8 → LED CLK

#### 3. Optional: Power Switch

Install between battery + and BMS B+:
```
Battery + ────[ Switch ]──── BMS B+
```

This lets you turn the entire hoop on/off.

#### 4. Battery Voltage Monitoring

Already covered in Stage 3, but verify:
- Voltage divider connected: Battery+ → 30kΩ → ADC pin → 7.5kΩ → GND
- Update `BATTERY_ANALOG_PIN` in Config.h

### Testing the Complete System

1. **Visual inspection:**
   - Check all polarity markings
   - Look for loose wires or exposed metal
   - Verify capacitor is connected correctly (+/- orientation)

2. **Voltage checks (with multimeter):**
   - Battery voltage: 7.0V - 8.4V ✓
   - BMS output (P+ to P-): Same as battery ✓
   - Buck converter output: 5.0V ±0.1V ✓

3. **Update firmware for full operation:**
   ```cpp
   // Config.h - restore to full settings
   #define NUM_LEDS 288  // Full LED count
   #define DEFAULT_ENERGY_SAVING_MODE 0  // Full brightness
   ```

4. **Power on sequence:**
   - Upload new firmware
   - Disconnect USB
   - Turn on power switch
   - Should see boot animation at full brightness!

5. **Battery monitoring test:**
   - Open serial monitor (via USB)
   - Check battery percentage reading
   - Should show 70-100% depending on charge

6. **BLE control test:**
   - Connect via BLE app
   - Try different effects
   - Test energy saving modes (1-8)
   - Verify colors display correctly

### ✅ Success Criteria
- [x] All 288 LEDs light up at full brightness
- [x] Buck converter stays cool (<50°C)
- [x] Battery percentage reads correctly over BLE
- [x] Can run effects wirelessly (no USB)
- [x] No voltage drops or flickering during operation
- [x] Runtime: 2+ hours at medium brightness

### ⚠️ Safety Checks
- Buck converter should be warm, not hot (>60°C = problem)
- Battery should stay cool during discharge
- No burning smell or discoloration
- BMS protection trips if you short P+ to P- (tests overcurrent protection)

**If this works, proceed to Stage 5!**

---

## Stage 5: Final Assembly & Testing

**Goal:** Install everything in the hoop and perform full system validation.

### Physical Installation

1. **Hoop preparation:**
   - Cut HDPE tubing to desired length (typical: 2 meters)
   - Mark battery placement positions
   - Plan wire routing path

2. **Component placement (for weight distribution):**
   - **Option A (distributed):** 2-3 battery packs spaced evenly around hoop
   - **Option B (centralized):** All batteries near microcontroller
   - Microcontroller: accessible for USB programming
   - Buck converter: near batteries, secured firmly

3. **LED strip installation:**
   - Clean inside of tubing
   - Apply LED strip with adhesive backing
   - Route wires through tubing to microcontroller
   - Test fit before permanent mounting

4. **Secure components:**
   - Use zip ties, hot glue, or 3M VHB tape
   - Ensure BMS and buck converter can't move
   - Protect solder joints with heat shrink
   - Label battery +/- for future reference

### Full System Test Sequence

#### Test 1: Basic Power & LEDs
- [x] Turn on hoop
- [x] Boot animation plays
- [x] All pixels light correctly
- [x] No flickering or dead pixels

#### Test 2: BLE Connectivity
- [x] Hoop is discoverable
- [x] Can connect from 10+ feet away
- [x] Effect changes work reliably
- [x] Color changes work reliably

#### Test 3: Battery Life
- [x] Full charge gives 2+ hours runtime (energy mode 0)
- [x] Battery percentage decreases smoothly
- [x] Low battery warning at ~20%
- [x] BMS cuts off before damage (<6.0V)

#### Test 4: Motion & Performance
- [x] Hoop balanced enough to spin comfortably
- [x] LEDs stay illuminated during spin
- [x] No loose components rattling
- [x] Effects look smooth during motion

#### Test 5: Energy Saving Modes
- [x] Mode 0: Full brightness, all pixels
- [x] Mode 8: Lowest power, longest runtime
- [x] Each mode visibly different
- [x] Mode change over BLE works

### POV (Persistence of Vision) Testing

If using hoop for POV effects:

1. **Set up image upload** (see DOCUMENTATION.md)
2. **Test spinning:** Effects should be visible during rotation
3. **Calibrate timing:** Adjust frame rate based on RPM
4. **Verify image quality:** Clear, sharp images when spinning

### Maintenance & Care

**Weekly:**
- Check battery voltage (should be 7.4V - 8.4V)
- Inspect solder joints for cracks
- Clean LED diffuser

**Monthly:**
- Re-flash firmware with any updates
- Tighten any loose components
- Balance charge batteries

**Storage:**
- Store batteries at 7.4V (50% charge)
- Keep hoop in cool, dry place
- Disconnect battery if storing >1 month

---

## Troubleshooting

### Problem: No LEDs light up

**Check:**
1. ✓ Buck converter outputting 5.0V?
2. ✓ Data pin correct in Config.h?
3. ✓ LED strip getting power (measure VCC pin)?
4. ✓ Try different `LED_COLOR_ORDER` setting
5. ✓ Test with just 1-2 LEDs first

### Problem: LEDs flicker or show wrong colors

**Causes:**
- Insufficient power (add larger capacitor)
- Voltage drop on long wires (use thicker wire)
- Wrong color order (change `LED_COLOR_ORDER` in Config.h)
- Faulty LED (skip bad LED, or replace section)

**Fixes:**
- Add 1000µF capacitor at LED strip power input
- Use 18 AWG or thicker for power wires
- For DotStar: Add 100Ω resistor on data line
- For NeoPixel: Add 470Ω resistor on data line

### Problem: BLE won't connect

**Check:**
1. ✓ Hoop powered on and boot animation running?
2. ✓ Phone Bluetooth enabled?
3. ✓ Not already connected to another device?
4. ✓ Try resetting: power cycle hoop
5. ✓ Check serial monitor for BLE initialization errors

### Problem: Battery drains too fast

**Optimize:**
- Use energy saving mode 3-8
- Reduce brightness via BLE
- Check for "vampire drain" when powered off (BMS quiescent current)
- Verify buck converter efficiency (should be >85%)
- Check for LED stuck on when it shouldn't be

### Problem: Hot components

**Normal:**
- Buck converter warm (40-50°C) ✓
- BMS slightly warm (30-40°C) ✓

**Danger:**
- Buck converter hot (>60°C) ✗ - May be overloaded
- Battery hot (>45°C) ✗ - Stop using immediately!
- Microcontroller hot ✗ - Check for short circuit

**Actions:**
- If too hot: power off immediately
- Check for short circuits
- Verify current draw isn't excessive
- May need higher-current buck converter

### Problem: Serial monitor shows low voltage warning

**Causes:**
- Battery needs charging (normal)
- Voltage divider resistors wrong values
- ADC pin not connected correctly
- Wrong `BATTERY_ANALOG_PIN` in Config.h

**Check:**
- Measure actual battery voltage with multimeter
- Verify R1=30kΩ, R2=7.5kΩ
- Update Config.h with correct pin number

### Problem: Inactivity timeout too aggressive

**Adjust in main.cpp:**
```cpp
constexpr unsigned long kBleDimTimeoutMs = 120000UL;  // 2 minutes
constexpr unsigned long kBleDeepSleepTimeoutMs = 600000UL;  // 10 minutes
```

Change these values to adjust timeouts.

---

## Advanced: Level Shifting for Reliable Signals

For long LED strips or noisy environments, you may need to boost the 3.3V data signal to 5V.

### Option 1: Sacrificial LED "Diode Trick" (Cheap)

Works for both NeoPixel and DotStar:

```
XIAO D10 ──[ 100Ω ]──┬──(+)LED(-)──[ 1N4148 diode ]──→ LED Strip DIN
                     │
                    GND
```

**How it works:**
- LED forward voltage drop ≈ 3.0V
- Diode forward voltage drop ≈ 0.7V
- Total output: 3.3V + 3.0V - 0.7V ≈ 5.6V (clamped to ~4.6V by strip)

**Parts:**
- 1x WS2812 or SK6812 LED (any color)
- 1x 1N4148 signal diode
- 1x 100Ω resistor

### Option 2: Dedicated Level Shifter IC (Professional)

Use 74HCT245 or similar:
- More reliable
- Cleaner signals
- Recommended for >3 meter strips

---

## Summary of Pin Connections

### Quick Reference: XIAO BLE Sense

| From | To | Notes |
|------|-----|-------|
| Battery + | BMS B+ | Via 30kΩ resistor to A0 (voltage sense) |
| Battery - | BMS B- | |
| BMS P+ | Buck IN+ | Power path |
| BMS P- | Buck IN- | |
| Buck 5V OUT | XIAO 5V | Microcontroller power |
| Buck 5V OUT | LED VCC | LED strip power |
| Buck GND | XIAO GND | Common ground |
| Buck GND | LED GND | |
| XIAO D10 | LED DIN | NeoPixel data |
| XIAO D10 | LED DATA | DotStar data |
| XIAO D8 | LED CLK | DotStar clock only |
| XIAO A0 | Voltage divider | Battery monitoring |

### Quick Reference: Nano 33 BLE

Same as above, except:
- D10 → D11 (NeoPixel/DotStar data)
- D8 → D13 (DotStar clock)
- A0 → A7 (voltage sensing)

---

## External Resources

- [Adafruit NeoPixel Überguide](https://learn.adafruit.com/adafruit-neopixel-uberguide)
- [DotStar Guide](https://learn.adafruit.com/adafruit-dotstar-leds)
- [Li-Po Battery Safety](https://learn.adafruit.com/li-ion-and-lipoly-batteries)
- [Buck Converter Basics](https://www.ti.com/lit/an/slva057/slva057.pdf)

---

**Congratulations! Your OpenHoop is complete! 🎉**

Now upload some custom effects, create POV images, and start performing!

For effect development, see [DOCUMENTATION.md](DOCUMENTATION.md).
For BLE control, see the README.md BLE Commands section.
