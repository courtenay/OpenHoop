<p align="center"><a href="https://github.com/angelcamelot/OpenHoop"><img src="LOGO.png" alt="OpenHoop logo" height="130"/></a></p>
<h1 align="center">OpenHoop</h1>
<p align="center">An open, community-driven smart hula hoop controller.</p>

<p align="center">
    <a href="https://opensource.org/"><img alt="Static Badge" src="https://img.shields.io/badge/Open%20Source-%E2%9D%A4-red"></a>
    <a href="https://platformio.org/"><img alt="Static Badge" src="https://img.shields.io/badge/PlatformIO-IDE-blue"></a>
    <a href="https://twitter.com/intent/tweet?text=Revolutionize%20your%20hooping%20with%20OpenHoop!%20Control%20your%20LED%20effects%20with%20ease%20and%20unleash%20your%20creativity%20on%20the%20dance%20floor.%20Check%20it%20out:%20https://github.com/angelcamelot/OpenHoop/&hashtags=OpenHoop,hulahoop,LED,Arduino,opensource"><img src="https://img.shields.io/badge/Tweet-1DA1F2?&logo=twitter&logoColor=white" alt="Tweet" height="20"/></a>
</p>
<br/><br/>

## :hear_no_evil: What is OpenHoop?

OpenHoop is a smart hula hoop controller built around an Arduino Nano 33 BLE Sense Rev2 and high-density Adafruit DotStar LEDs. It blends creative expression with robust engineering so performers can choreograph reliable, mesmerizing light shows. Today the project includes a refined Bluetooth® Low Energy (BLE) control stack, an expanding catalog of prebuilt effects, and thorough documentation for crafting custom pixel art.

At the moment, I am building OpenHoop solo. The system is not yet the most advanced hoop controller available, but it has a solid foundation. I firmly believe that by joining forces with other makers, performers, and engineers, we can rapidly turn OpenHoop into the most accessible and powerful hoop platform. This repository is intentionally open so anyone can experiment, iterate, and help shape the future of LED hooping.

## :art: Persistence of Vision & Pixel Art

OpenHoop embraces the phenomenon of [Persistence of Vision (POV)](https://en.wikipedia.org/wiki/Persistence_of_vision) to transform rapid LED animations into continuous images. By synchronizing DotStar LEDs with hoop motion, the controller paints detailed pixel art and scrolling messages in mid-air, achieving results similar to professional POV poi and staff props. The documentation includes a full workflow for translating drawings into LED frames, ensuring repeatable, performance-ready visuals whether you are animating logos, characters, or abstract gradients.

## :bulb: Features

- **Precision POV Rendering:** Generate smooth persistence-of-vision animations and pixel art with per-frame timing guidance.
- **Versatile Effect Utilities:** Access sound levels, gyroscope readings, and more to enhance the flexibility of LED effects.
- **Growing LED Effect Library:** Explore a library of pre-built LED effects or create custom patterns using improved scaffolding examples.
- **Enhanced Bluetooth® Connectivity:** Experience fast effect switching, low-latency color edits, and resilient reconnection flows through an upgraded BLE HID profile.
- **Energy-Saving Modes:** Optimize battery usage with customizable energy-saving levels for extended performances.
- **Developer-Centric Tooling:** Follow updated documentation, templates, and testing checklists to safely extend the firmware.

## :toolbox: Hardware Requirements

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

### Hula Hoop System Diagram

![Hula Hoop System Diagram](DIAGRAM.png)

*This diagram captures the configuration currently powering my personal hoop. It prioritizes a minimum of 144 pixels for POV clarity, rechargeability, and at least two hours of runtime. Community feedback and alternative designs are very welcome so we can validate more configurations that work with the OpenHoop firmware and Adafruit DotStar LED density.*

## :computer: Software Requirements

- PlatformIO

## :link: Dependencies

- [Adafruit DotStar Library](https://github.com/adafruit/Adafruit_DotStar)
- [ArduinoBLE Library](https://github.com/arduino-libraries/ArduinoBLE)
- [PDM Library](https://github.com/arduino-libraries/PDM)

## :inbox_tray: Installation

1. **Clone this repository:** `git clone https://github.com/angelcamelot/OpenHoop.git`
2. **Open the `OpenHoop` folder in PlatformIO.**
3. **Install the required libraries using the PlatformIO Library Manager.**
4. **Connect your hardware and upload the code to your microcontroller.**

## :rocket: Usage

Follow these steps to set up and use OpenHoop for your hula hoop performances:

1. **Power on the hula hoop.**
2. **Connect to the OpenHoop BLE device using a compatible mobile app.** The firmware advertises descriptive GATT services for effect control, live previews, and battery reporting.
3. **Customize LED effects, colors, and energy-saving settings.** Presets provide direct links to persistence-of-vision image banks, mic-reactive modes, and per-performance energy profiles.
4. **Enjoy the mesmerizing light display during your hula hoop performance!**

## :signal_strength: BLE Commands

Use the following commands to customize LED effects. Commands can be sent over the BLE HID UART bridge or through a companion app interface:

- `EffectType`: Change the LED effect (Rainbow, Fire, Pulse, PixelImage, etc.).
- `EffectParameter`: Provide structured JSON data for tuning parameters such as speed, frame rate, and gyroscope thresholds.
- `SolidColor`: Set a solid color using RGB values.
- `ImageUpload`: Stream new pixel art frames to the controller using run-length encoded payloads.
- `EnergySavingMode`: Adjust energy-saving mode levels (0 to 3).
- `SystemStatus`: Query battery percentage, internal temperature, or firmware version.

## :books: Documentation

[Documentation](DOCUMENTATION.md) captures the persistence-of-vision workflow, effect authoring templates, and BLE protocol reference. Refer to it for step-by-step guides, class diagrams, and troubleshooting recipes.

## :warning: Troubleshooting

If you encounter issues, review the troubleshooting section in the codebase or open an issue on the [GitHub repository](https://github.com/angelcamelot/OpenHoop/issues). The documentation includes BLE sniffer walkthroughs, pixel calibration techniques, and recovery procedures for failed image uploads.

## :clipboard: Roadmap

- Conduct thorough testing, including extensive trials with pixel art images.
- Enhance documentation to provide comprehensive guidance and clarity.
- Expand support for additional LED effects to offer more creative options.
- Refine energy-saving algorithms to optimize battery usage.
- Improve the mobile app interface for a smoother and more intuitive user experience.
- Validate alternative hardware configurations contributed by the community.

## :handshake: Contributing

Although I am maintaining OpenHoop by myself today, the vision has always been collaborative. Every contribution—code, hardware experiments, design ideas, documentation, or community support—helps move the project toward a truly open smart hoop ecosystem.

**How to get involved:**

- **Fork the repository:** Create your own copy of the codebase to experiment safely.
- **Make your changes:** Add new features, fix bugs, optimize hardware, or improve documentation.
- **Submit a pull request:** Share your improvements with the community. Each PR receives careful review to keep the project stable and inclusive.

Before opening a pull request, please check the [CONTRIBUTING.md](CONTRIBUTING.md) guidelines and run available tests locally. If you want to collaborate on larger features, open a discussion or issue so we can plan together.

## :scroll: License

OpenHoop is released under the [MIT License](LICENSE.md). See the [LICENSE.md](LICENSE.md) file for the full text.

## :tada: Join the Journey

OpenHoop is a long-term experiment in building a free, open hoop for everyone. The firmware is functional, but it will only reach its full potential when many people shape it together. If you believe in accessible maker tools and love the idea of blending performance art with technology, please star the repo, spread the word, and contribute however you can. Let's co-create the best hoop system on the planet.
