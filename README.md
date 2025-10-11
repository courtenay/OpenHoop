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

OpenHoop is a smart hula hoop controller built around an Arduino Nano 33 BLE Sense Rev2 and high-density Adafruit DotStar LEDs. It blends creative expression with robust engineering so performers can choreograph reliable, mesmerizing light shows. Today the project includes a refined Bluetooth® Low Energy (BLE) control stack, an expanding catalog of prebuilt effects, and thorough documentation for crafting custom pixel art.

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

### Microcontroller, Power Supply, LED, and Connectivity
- Arduino Nano 33 BLE Sense Rev2
- x2 High-Efficiency Output 5V 5A Mini560 Step-Down DC-DC Converters
- x6 3000mAh 14500 3.7V Rechargeable Li-Ion Batteries
- 7.4V–8.4V 2S BMS PCM Charge/Discharge Protection Board
- Voltage Tester Sensor Measurement Detection Module
- DC-DC Step-Up Converter Booster Power Module
- Adafruit DotStar LED Strip (2 meters, 144 LEDs per meter)
- USB 3.1 Type-C Connector (24 Pins Female Plug Socket)
- Assorted Cables and Connectors
- 6.3V 4000 Capacitor (optional)
- Ceramic Capacitors (optional)
- Male and Female Butt Socket Electric Motorcycle Wire Connector (optional)

### Hula Hoop Components
- HDPE Tubing (7/8 inch, 2 meters)
- Polypro Connector for Tubing (7/8 inch)
- Stainless Steel Paddle Spring Buckle

### Additional Tools and Materials
- Basic Hand Tools
- Soldering Iron (optional, for advanced assembly)

> _The hero diagram at the top reflects the maintainer's personal hoop. It prioritizes at least 144 pixels for POV clarity, rechargeable packs, and two hours of runtime. Share your tweaks so we can validate additional builds!_

## Software Requirements

- PlatformIO

## Dependencies

- [Adafruit DotStar Library](https://github.com/adafruit/Adafruit_DotStar)
- [ArduinoBLE Library](https://github.com/arduino-libraries/ArduinoBLE)
- [PDM Library](https://github.com/arduino-libraries/PDM)

## Setup Guide

1. **Clone this repository:** `git clone https://github.com/angelcamelot/OpenHoop.git`
2. **Open the `OpenHoop` folder in PlatformIO.**
3. **Install the required libraries using the PlatformIO Library Manager.**
4. **Connect your hardware and upload the code to your microcontroller.**

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
