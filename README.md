<p align="center"><strong>OpenHoop is an open-source LED hoop controller for artists, makers, and performers who want to merge light, movement, and code.</strong></p>

<p align="center"><a href="https://github.com/angelcamelot/OpenHoop"><img src="LOGO.png" alt="OpenHoop logo" height="130"/></a></p>
<h1 align="center">OpenHoop</h1>
<p align="center">An open, community-driven smart hula hoop controller.</p>

<p align="center">
    <a href="LICENSE.md"><img alt="License: MIT" src="https://img.shields.io/badge/License-MIT-yellow.svg"></a>
    <img alt="Build Status" src="https://img.shields.io/badge/Build-Community%20Tested-blueviolet">
    <a href="https://platformio.org/"><img alt="PlatformIO 6" src="https://img.shields.io/badge/PlatformIO-6.x-blue"></a>
    <img alt="Made with love in Montreal" src="https://img.shields.io/badge/Made%20with%20%E2%9D%A4%EF%B8%8F-Montreal-orange">
</p>

<p align="center">
    <img src="DIAGRAM.png" alt="OpenHoop LED hoop demonstration" width="600"/>
</p>

## Quick Overview

**What it does:** OpenHoop drives high-density DotStar LEDs with precision timing, delivering persistence-of-vision (POV) patterns and responsive lighting effects that follow every spin. The firmware combines a modular effect engine with Bluetooth® connectivity so performers can adapt routines on the fly.

**Why it stands out:** The project pairs polished documentation with composable effect utilities, making it easy to script new animations or integrate alternative hardware builds. Modular effect templates, sensor integrations, and a flexible BLE protocol keep experimentation fast and fun.

🎥 **Demo video:** _We are preparing a showcase video—stay tuned!_

<details>
<summary><strong>Table of Contents</strong></summary>

- [About](#about)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Setup Guide](#setup-guide)
- [How to Contribute](#how-to-contribute)
- [License](#license)

</details>

## About

OpenHoop is a smart hula hoop controller built around nRF52840-based microcontrollers (Seeed Studio XIAO BLE Sense or Arduino Nano 33 BLE Sense Rev2) and high-density Adafruit DotStar LEDs. It blends creative expression with robust engineering so performers can choreograph reliable, mesmerizing light shows. Today the project includes a refined Bluetooth® Low Energy (BLE) control stack, an expanding catalog of prebuilt effects, and thorough documentation for crafting custom pixel art.

At the moment, OpenHoop is primarily built by a solo maintainer. The system already has a solid foundation, and with help from other makers, performers, and engineers, it can quickly become the most accessible and powerful hoop platform. This repository is intentionally open so anyone can experiment, iterate, and help shape the future of LED hooping.

OpenHoop embraces the phenomenon of [Persistence of Vision (POV)](https://en.wikipedia.org/wiki/Persistence_of_vision) to transform rapid LED animations into continuous images. By synchronizing DotStar LEDs with hoop motion, the controller paints detailed pixel art and scrolling messages in mid-air, achieving results similar to professional POV poi and staff props. The documentation includes a full workflow for translating drawings into LED frames, ensuring repeatable, performance-ready visuals whether you are animating logos, characters, or abstract gradients.

## Features

- ✅ **Precision POV rendering** – Generate smooth persistence-of-vision animations with per-frame timing guidance for detailed pixel art.
- ✅ **Versatile effect utilities** – Tap into sound levels, IMU data, and battery metrics to drive adaptive lighting behaviors.
- ✅ **Growing effect library** – Mix and match pre-built effects with customizable parameters for rehearsals and performances.
- ✅ **Enhanced Bluetooth® connectivity** – Switch effects, tweak colors, and upload assets over a resilient BLE HID profile.
- ✅ **Energy-saving modes** – Configure performance profiles that balance brightness, runtime, and responsiveness.
- 🛠️ **Bluetooth companion tooling** – Companion mobile/desktop control experiments are underway; community prototypes welcome.
- 🛠️ **Alternative hardware profiles** – Actively validating community-sourced battery packs, LED densities, and hoop dimensions.

## Hardware Requirements

### LED Strip Selection

OpenHoop now supports **both** DotStar and NeoPixel LED strips! Choose based on your needs:

#### NeoPixel (WS2812B) - Recommended for Beginners
- ✅ **Single data wire** - simpler wiring, fewer connections
- ✅ **More affordable** - typically 30-50% cheaper than DotStar
- ✅ **Widely available** - easier to source from multiple suppliers
- ⚠️ Slightly lower refresh rate (still excellent for most effects)

#### DotStar (APA102) - Advanced Performance
- ✅ **Higher refresh rates** - better for fast persistence-of-vision effects
- ✅ **More reliable signaling** - two-wire protocol less susceptible to noise
- ⚠️ Requires two pins (data + clock)
- ⚠️ Typically more expensive

**Configuration:** Edit `include/Config.h` to select your LED type:
```cpp
// Uncomment ONE of these:
#define USE_DOTSTAR    // For APA102/DotStar strips
// #define USE_NEOPIXEL   // For WS2812B/NeoPixel strips
```

Both types work with all effects and features. See [WIRING_GUIDE.md](WIRING_GUIDE.md) for detailed pin connections.

### Simplified Design (Recommended)

**Core Electronics:**
- **Seeed Studio XIAO BLE Sense (nRF52840)** - Microcontroller (note: built-in USB-C charging is for small 1S Li-Po only; main 2S packs charge separately)
- **LED Strip** - Choose one:
  - **NeoPixel (WS2812B):** 2 meters, 144 LEDs/m (recommended for beginners)
  - **DotStar (APA102):** 2 meters, 144 LEDs/m (advanced performance)
- **Battery Options** (choose one for weight distribution):
  - **Option 1**: 2x 2S Li-Po packs (7.4V, 1500mAh each) - Distributed on opposite sides of hoop
  - **Option 2**: 3x 2S Li-Po packs (7.4V, 1000mAh each) - Distributed evenly around hoop
  - **Option 3**: Single 2S Li-Po pack (7.4V, 2500-3000mAh) - Centralized (simpler but less balanced)
- **2S Li-Po BMS Protection Board** - One per battery pack for protection and balancing
- **DC-DC Buck Converter** (7.4V → 5V, 3-5A) - Single converter for LED power
- **Signal Level Shifter Components** (optional, for long strips or noisy environments):
  - 1x WS2812/SK6812 LED (sacrificial LED for voltage boost)
  - 1x 1N4148 signal diode
  - 1x 100Ω resistor
- **On/Off Toggle Switch**
- **Assorted Cables and Connectors**
- **1000µF Capacitor** (for LED power smoothing, optional but recommended)

> **Why this design?** The XIAO BLE Sense includes built-in USB-C charging, eliminating separate charging circuits. The 2S battery configuration with buck converter is more efficient than boost converters. NeoPixel strips simplify wiring with single-wire data. Optional level shifting (using the sacrificial LED trick) boosts 3.3V signals to ~4.6V for long strips without requiring expensive level shifter ICs. This reduces component count, cost, and complexity while maintaining full functionality.

**📘 Complete assembly instructions:** See [WIRING_GUIDE.md](WIRING_GUIDE.md) for detailed step-by-step wiring with incremental testing at each stage.

### Alternative Design (Original)

For reference, the original design used:
- Arduino Nano 33 BLE Sense Rev2
- x2 High-Efficiency Output 5V 5A Mini560 Step-Down DC-DC Converters
- x6 3000mAh 14500 3.7V Rechargeable Li-Ion Batteries
- 7.4V–8.4V 2S BMS PCM Charge/Discharge Protection Board
- Voltage Tester Sensor Measurement Detection Module
- DC-DC Step-Up Converter Booster Power Module
- USB 3.1 Type-C Connector (24 Pins Female Plug Socket)

### Hula Hoop Components
- HDPE Tubing (7/8 inch, 2 meters)
- Polypro Connector for Tubing (7/8 inch)
- Stainless Steel Paddle Spring Buckle

### Additional Tools and Materials
- Basic Hand Tools
- Soldering Iron (recommended for assembly)
- Heat Shrink Tubing
- Wire Stripper/Cutter

### Power System Design Notes

**Why 2S (7.4V) instead of 1S (3.7V)?**

For LED applications with high current draw, 2S configuration with buck (step-down) converter is superior:
- **Higher efficiency**: Buck converters (85-95%) vs boost converters (80-90%)
- **Better voltage headroom**: 7.4V nominal provides stable 5V even when battery drains to 6.4V
- **Lower current draw**: At 15W LED power, 2S draws ~2A vs 1S drawing ~4A
- **Less voltage sag**: Lower current means less performance degradation under load
- **Cooler operation**: More efficient = less heat in the hoop

**Battery Wiring for Distributed Packs:**

When using multiple 2S packs:
1. Wire all packs in **parallel** (positive to positive, negative to negative)
2. Each pack needs its own BMS protection board
3. Use appropriately rated connectors (XT30 or XT60 recommended)
4. Connect combined battery output to buck converter input
5. Buck converter output feeds LED strip 5V rail
6. XIAO BLE powers from 5V rail (via 5V pin or USB-C connector)

**Total capacity examples:**
- 2x 1500mAh = 3000mAh total @ 7.4V
- 3x 1000mAh = 3000mAh total @ 7.4V

**Charging Options:**

For 2S battery packs:
- **Option A** (Simple): Remove packs and charge with external 2S Li-Po balance charger
- **Option B** (Integrated): Install 2S balance charge port in hoop (requires 2S charging module like TP4056-based 2S charger)
- **Option C** (Quick-swap): Use multiple sets of charged packs for extended performance sessions

Note: The XIAO's built-in charging circuit is designed for small single-cell (1S) Li-Po batteries only and is not used for the main 2S power system.

### Signal Level Shifting Wiring (Diode Trick)

To boost the 3.3V data signal from the XIAO BLE to ~4.6V for reliable DotStar operation:

1. Connect XIAO data pin → 100Ω resistor → Anode of sacrificial LED
2. Connect Cathode of sacrificial LED → Anode of 1N4148 diode
3. Connect Cathode of 1N4148 diode → DotStar Data In pin
4. The sacrificial LED's cathode also connects to GND
5. LED power comes from 5V buck converter (not the 3.3V line)

This creates approximately 4.6V on the data line (3.3V + LED forward voltage drop), ensuring reliable signal levels without a dedicated level shifter IC. See [Hackaday's guide](https://hackaday.com/2017/01/20/cheating-at-5v-ws2812-control-to-use-a-3-3v-data-line/) for more details on this technique.

> _The hero diagram reflects the original design. The simplified design above reduces component count by ~50% while maintaining all functionality. Both designs prioritize 144+ pixels for POV clarity, rechargeable packs, and 2+ hours of runtime. Share your builds so we can validate additional configurations!_

## Software Requirements

- PlatformIO

## Dependencies

- [Adafruit DotStar Library](https://github.com/adafruit/Adafruit_DotStar) - For APA102/DotStar LED strips
- [Adafruit NeoPixel Library](https://github.com/adafruit/Adafruit_NeoPixel) - For WS2812B/NeoPixel LED strips
- [ArduinoBLE Library](https://github.com/arduino-libraries/ArduinoBLE)
- [PDM Library](https://github.com/arduino-libraries/PDM)

**Note:** Both LED libraries are included in `platformio.ini`. Only the one selected in `Config.h` will be compiled into your build.

## Setup Guide

### Software Configuration

1. **Clone this repository:** `git clone https://github.com/angelcamelot/OpenHoop.git`
2. **Open the `OpenHoop` folder in PlatformIO.**
3. **Configure LED strip type in `include/Config.h`:**
   ```cpp
   // Uncomment ONE of these lines:
   #define USE_DOTSTAR    // For APA102/DotStar strips
   // #define USE_NEOPIXEL   // For WS2812B/NeoPixel strips
   ```
4. **Select the appropriate build environment:**
   - For **Seeed Studio XIAO BLE Sense** (simplified design): Use `env:xiao_ble_sense`
   - For **Arduino Nano 33 BLE Sense Rev2** (original design): Use `env:nano33ble`
5. **Install the required libraries using the PlatformIO Library Manager** (done automatically on first build).
6. **Connect your hardware via USB and upload the code to your microcontroller.**

### Hardware Assembly

**📘 For complete hardware assembly instructions with incremental testing at each stage, see [WIRING_GUIDE.md](WIRING_GUIDE.md).**

The wiring guide covers:
- Detailed pin connections for both XIAO and Nano 33
- Step-by-step assembly with testing after each stage
- Both NeoPixel and DotStar wiring diagrams
- Battery and BMS installation
- Buck converter setup
- Troubleshooting common issues

## Usage

Follow these steps to set up and use OpenHoop for your hula hoop performances:

1. **Power on the hula hoop.**
2. **Connect to the OpenHoop BLE device using a compatible mobile app.** The firmware advertises descriptive GATT services for effect control, live previews, and battery reporting.
3. **Customize LED effects, colors, and energy-saving settings.** Presets provide direct links to persistence-of-vision image banks, mic-reactive modes, and per-performance energy profiles.
4. **Enjoy the mesmerizing light display during your hula hoop performance!**

## BLE Commands

Use the following commands to customize LED effects. Commands can be sent over the BLE HID UART bridge or through a companion app interface:

- `EffectType`: Change the LED effect (Rainbow, Fire, Pulse, PixelImage, etc.).
- `EffectParameter`: Provide structured JSON data for tuning parameters such as speed, frame rate, and gyroscope thresholds.
- `SolidColor`: Set a solid color using RGB values.
- `ImageUpload`: Stream new pixel art frames to the controller using run-length encoded payloads.
- `EnergySavingMode`: Adjust energy-saving mode levels (0 to 3).
- `SystemStatus`: Query battery percentage, internal temperature, or firmware version.

## Documentation

[Documentation](DOCUMENTATION.md) captures the persistence-of-vision workflow, effect authoring templates, and BLE protocol reference. Refer to it for step-by-step guides, class diagrams, and troubleshooting recipes.

## Troubleshooting

If you encounter issues, review the troubleshooting section in the codebase or open an issue on the [GitHub repository](https://github.com/angelcamelot/OpenHoop/issues). The documentation includes BLE sniffer walkthroughs, pixel calibration techniques, and recovery procedures for failed image uploads.

## Roadmap

- Conduct thorough testing, including extensive trials with pixel art images.
- Enhance documentation to provide comprehensive guidance and clarity.
- Expand support for additional LED effects to offer more creative options.
- Refine energy-saving algorithms to optimize battery usage.
- Improve the mobile app interface for a smoother and more intuitive user experience.
- Validate alternative hardware configurations contributed by the community.

## How to Contribute

We welcome makers, coders, artists, and dreamers! Whether you want to fix bugs, design effects, or improve docs—there’s space for you.

1. **Fork the repository:** Create your own copy of the codebase to experiment safely.
2. **Make your changes:** Add new features, fix bugs, optimize hardware, or improve documentation.
3. **Open a pull request:** Share your improvements with the community. Every PR is reviewed with the goal of keeping OpenHoop friendly and reliable.

Before opening a pull request, please read the [CONTRIBUTING.md](CONTRIBUTING.md) guidelines and run available tests locally. If you want to collaborate on larger features, open a discussion or issue so we can plan together.

## License

OpenHoop is released under the [MIT License](LICENSE.md). See the [LICENSE.md](LICENSE.md) file for the full text.

## Join the Journey

OpenHoop is a long-term experiment in building a free, open hoop for everyone. The firmware is functional, but it will only reach its full potential when many people shape it together. If you believe in accessible maker tools and love the idea of blending performance art with technology, please star the repo, spread the word, and contribute however you can. Let's co-create the best hoop system on the planet.
