# OpenHoop Documentation

Welcome to the OpenHoop documentation! This document provides comprehensive guidance on creating LED images and effects for your hula hoop using the OpenHoop project.

## Table of Contents

1. [Introduction](#introduction)
2. [Persistence of Vision Workflow](#persistence-of-vision-workflow)
3. [Creating LED Images](#creating-led-images)
4. [Developing LED Effects](#developing-led-effects)
5. [Advanced Features](#advanced-features)
6. [BLE Commands](#ble-commands)
7. [Troubleshooting Toolkit](#troubleshooting-toolkit)

## Introduction

OpenHoop is an open-source Arduino Nano 33 BLE Sense Rev2 project that seamlessly integrates cutting-edge technology with artistic expression. With OpenHoop, you can design and display custom LED effects and pixel art on your hula hoop. This documentation will guide you through the process of creating your own LED images and effects to personalize your hula hoop experience. Recent updates focus on fine-tuning persistence-of-vision rendering, strengthening the BLE control layer, and providing makers with reproducible workflows for both firmware and creative assets.

## Persistence of Vision Workflow

The OpenHoop firmware leverages the phenomenon of persistence of vision (POV) to transform sequential LED frames into fully rendered images while the hoop spins. When the LEDs refresh quickly enough, the human eye blends consecutive frames, resulting in cohesive light paintings suspended in the air.

### Planning Your POV Animation

1. **Capture Motion Data:** Record hoop speed using the built-in IMU or external motion capture. Average rotations per minute (RPM) help you determine how many frames you can display per revolution.
2. **Map Canvas Dimensions:** Translate the RPM and LED count into a virtual canvas. For example, 288 LEDs at two meters with a 40 RPM spin can support up to 24 frames before motion blur becomes visible.
3. **Storyboard the Sequence:** Sketch your design as a sprite sheet. Each column represents a radial slice of the hoop. Keep high-contrast edges and avoid single-pixel noise that may disappear in motion.

### Converting Artwork into Frames

1. **Prepare Source Art:** Use your favorite editor (e.g., Aseprite, Krita) to export artwork in indexed PNG format.
2. **Run the Frame Converter:** Utilize the provided Python utilities in `tools/pov/` (or your own scripts) to translate PNG columns into DotStar color data. The converter handles gamma correction and optional run-length encoding.
3. **Validate Against the Simulator:** Preview the animation using the desktop simulator or mock data logging included in the repository to ensure colors match expectations.

### Uploading and Testing

1. **Transfer Frames:** Send the generated frames via the `ImageUpload` BLE command. The controller acknowledges each block and writes it to flash.
2. **Calibrate Timing:** Use the `EffectParameter` command to adjust frame rate and alignment offsets based on your hoop's inertia.
3. **Field Test:** Spin the hoop under performance lighting to confirm the POV effect reads clearly from multiple viewing angles.

## Creating LED Images

### Overview

LED images are pixel art patterns that can be displayed on the LED strip of your hula hoop. Whether you're creating static designs or dynamic patterns, LED images offer endless possibilities for customization. The POV workflow described above ensures these images remain legible while in motion.

### LedImage Class Overview

The `LedImage` class is an abstract base class representing an image composed of LEDs. It provides methods for setting and getting pixel colors, as well as retrieving the width and height of the image.

### LedImage Class Methods

* **Constructor**: Initializes the LedImage object with a specified width and height.
* **Destructor**: Frees the memory allocated for the pixel array.
* **setPixel()**: Sets the color of a pixel at a specific position.
* **getPixel()**: Retrieves the color of a pixel at a specific position.
* **getWidth()**: Gets the width of the image.
* **getHeight()**: Gets the height of the image.

### Example: Creating a Custom LED Image

Follow these steps to create a custom LED image:

1. **Define the Image Dimensions**: Determine the width and height of your LED image based on the size of the desired pixel art image.

2. **Create the Header File (OpenHoop/include/images/YourCustomImage.h)**:

    ```c++
    #ifndef OPENHOOP_YOURCUSTOMIMAGE_H
    #define OPENHOOP_YOURCUSTOMIMAGE_H
    
    #include "LedImage.h"
    
    /**
     * @brief Represents an image of a custom pattern for the Hula Hoop LED display.
     */
    class YourCustomImage : public LedImage {
    public:
        /**
         * @brief Constructor for the YourCustomImage class.
         * @param primary_color Color for the main pattern.
         * @param secondary_color Color for the background or secondary pattern.
         */
        explicit YourCustomImage(uint32_t primary_color, uint32_t secondary_color);
    
    private:
        /**
         * @brief Set the colors for your custom pattern.
         * @param primary_color Color for the main pattern.
         * @param secondary_color Color for the background or secondary pattern.
         */
        void setPatternColors(uint32_t primary_color, uint32_t secondary_color) const;
    };
    
    #endif //OPENHOOP_YOURCUSTOMIMAGE_H
    
    ```

3. **Implement the Source File (OpenHoop/src/images/YourCustomImage.cpp)**:

    ```c++
    #include "../../include/images/YourCustomImage.h"
    
    YourCustomImage::YourCustomImage(uint32_t primary_color, uint32_t secondary_color) : LedImage(30, 30) {
        setPatternColors(primary_color, secondary_color);
    }
    
    void YourCustomImage::setPatternColors(uint32_t primary_color, uint32_t secondary_color) const {
        // Define the pixel colors for the pattern
        setPixel(0, 0, secondary_color);
        setPixel(1, 0, primary_color);
        setPixel(4, 0, primary_color);
        setPixel(2, 2, 0xffffff);
        setPixel(4, 2, 0xff0000);
        // Add more setPixel calls to complete the pattern
        ...
    }
    
    ```

4. **Upload to your hula hoop**: Incorporate the `YourCustomImage` class into your OpenHoop codebase and upload it to your Arduino Nano 33 BLE Sense Rev2 microcontroller connected to the Adafruit Neopixel strip of your hula hoop.

By following these steps, you can create and customize LED images for your hula hoop, adding visually appealing patterns and effects to enhance your performances.

## Developing LED Effects

### Overview

LED effects bring life to your hula hoop, allowing you to create dynamic visual experiences. Customizing patterns and developing LED effects involves defining the behavior, appearance, and interactivity of the LED display. The latest firmware exposes new hooks for gyroscope-driven modulation, time-sliced pixel updates, and synchronized audio reactivity, making it easier to craft multilayered performances.

### Effect Class Overview

The `Effect` class is an abstract base class representing an LED effect. It provides pure virtual methods for starting, updating, and stopping an LED effect.

### Effect Class Methods

* **start()**: Initiates the effect.
* **update()**: Updates the effect.
* **stop()**: Terminates the effect.

### Developing a Custom LED Effect

#### Steps:

1. **Choose an Effect:**
    - Decide on the type of effect you want to create, such as a rainbow gradient, pulsating colors, or a scrolling pattern. Consider the visual impact and mood you want to convey with your effect.

2. **Identify Effect Elements:**
    - Determine which aspects of your LED effect can be customized. This may include color transitions, animation speeds, patterns, or any other visual or behavioral elements you wish to incorporate.

3. **Define Effect Logic and Parameters:**
    - Write code to define the behavior of your effect and the characteristics of the pattern. For example, you may define how colors transition, how patterns move or evolve, and any user-adjustable parameters such as speed or intensity.

4. **Implement Effect Variations:**
    - Create variables or parameters in your code to control the characteristics of the effect. This allows users to adjust the appearance dynamically to suit their preferences. Consider providing a range of customization options to enhance user experience.

5. **Test and Iterate:**
    - Upload the code to your hula hoop and test the effect. Make adjustments as needed to fine-tune the appearance and behavior, ensuring it meets your expectations and provides a visually appealing experience.

6. **Document Your Effect:**
    - Provide clear documentation for your effect, including instructions on usage and customization parameters. Explain how users can activate and interact with the effect, and provide guidance on adjusting parameters to achieve desired effects.

#### Example Effect Implementation:

Let's say we want to create a custom effect that simulates a flickering candle flame.

- **Effect Logic and Parameters:**
    - We'll define a pattern of color transitions to mimic the flickering flame, with variations in brightness and intensity.
    - Parameters such as speed, intensity, and color range will be adjustable to allow users to customize the appearance of the flame.

- **Implementation:**
    - We'll create a `CandleEffect` class that inherits from the `Effect` base class.
    - The `CandleEffect` class will implement the `start()`, `update()`, and `stop()` methods to initiate, update, and terminate the effect, respectively.
    - Within the `update()` method, we'll define the logic for the flickering flame effect, adjusting color transitions and brightness based on random fluctuations.

- **Integration with OpenHoop:**
    - Once the `CandleEffect` class is implemented, we'll add it to the `EffectType` enum in `OpenHoop/include/services/EffectService.h`.
    - In `OpenHoop/src/services/EffectService.cpp`, we'll update the `dispatchEffectCommand()` method to include a case for `EffectType::CANDLE`.
    - Users can then activate the custom candle flame effect by selecting it from the available options in the OpenHoop app.

By following these steps and implementing our custom candle flame effect, users can enjoy a unique and immersive visual experience during their hula hoop performances.

### Example: Implementing Custom Image in Custom Effect

Combine the POV frame uploader with real-time sensor data to build hybrid experiences. For instance, preload a pixel-art background and overlay IMU-reactive sparkles using the `EffectParameter` channel for dynamic thresholds.

To integrate a custom LED image into a custom LED effect, follow these steps:

1. **Define Custom Effect Header (`YourCustomEffect.h`)**:

    ```c++
    #ifndef OPENHOOP_YOURCUSTOMEFFECT_H
    #define OPENHOOP_YOURCUSTOMEFFECT_H

    #include "Effect.h"
    #include "../images/YourCustomImage.h" // Include the custom image header
    #include "Arduino.h"

    /**
     * @brief Represents a custom LED effect using a custom image pattern.
     */
    class YourCustomEffect : public Effect {
    public:
        YourCustomEffect();

        /**
         * @brief Start the custom effect.
         */
        void start() override;

        /**
         * @brief Update the custom effect.
         */
        void update() override;

        /**
         * @brief Stop the custom effect.
         */
        void stop() override;

    private:
        YourCustomImage customImage; // Instance of the custom image
        int counter;  ///< Counter for effect animation.
    };

    #endif //OPENHOOP_YOURCUSTOMEFFECT_H
    ```

2. **Implement Custom Effect Source (`YourCustomEffect.cpp`)**:

    ```c++
    #include "../../include/effects/YourCustomEffect.h"
    #include "../../include/utils/EffectUtils.h"

    YourCustomEffect::YourCustomEffect() : counter(0), customImage(0xFF0000, 0x0000FF) {
        // Constructor implementation goes here
    }

    void YourCustomEffect::start() {
        // Initialization code for the custom effect goes here
    }
   
    void YourCustomEffect::update() {
    // Effect update code goes here
    for (int x = 0; x < customImage.getWidth(); ++x) {
    for (int y = 0; y < customImage.getHeight(); ++y) {
    // Get the color of each pixel from the custom LED image
    uint32_t pixelColor = customImage.getPixel(x, y, 0x000000);
    
                // Apply the pixel color to the LED strip or perform other actions
                // based on the effect logic
            }
        }
    
        counter++;
    }

    void YourCustomEffect::stop() {
        // Cleanup code for the custom effect goes here
    }
    ```

By following these steps, you can seamlessly integrate a custom LED image into your custom LED effect, enhancing the visual experience of your hula hoop performances.

### Examples of Customized Patterns and LED Effects

1. **Rainbow Gradient**: Develop an effect that creates a smooth transition of colors across the LED strip, customizable by speed and direction.

2. **Pulsating Colors**: Create an effect that alternates between bright and dim LED intensities, with adjustable speed and intensity.

3. **Scrolling Pattern**: Develop an effect that displays a scrolling pattern or text message across the LED strip, customizable by content, speed, and direction.

4. **Interactive Modes**: Implement modes allowing users to control LED effects using external inputs, such as buttons or touch sensors.

## Advanced Features

### Overview

Advanced features in OpenHoop extend project capabilities and offer additional functionality for exploration and experimentation. These features may include sensor integration, interactive modes, network connectivity, and data visualization.

### Examples of Advanced Features

1. **Sensor integration**: Incorporate data from onboard sensors to influence LED effects in response to movement or environmental conditions.

2. **Interactive modes**: Implement modes allowing users to control LED effects using external inputs, such as buttons or touch sensors.

3. **Network connectivity**: Enable communication between multiple OpenHoop devices over a network, allowing synchronized LED displays.

4. **Data visualization**: Create LED effects visualizing real-time data streams, such as music visualization or weather forecasts.

### Developer Utilities

OpenHoop ships with helper utilities to streamline experimentation:

- **EffectUtils helpers:** Centralize color math, easing functions, and energy-saving adjustments so new effects stay consistent with global settings.
- **POV Toolchain:** Python scripts under `tools/pov/` convert raster artwork into frame data, apply gamma correction, and validate color temperature.
- **Calibration Suites:** Use the `tools/calibration` scripts to remap LED positions after hardware repairs and balance brightness across segments.

#### Utilizing EffectUtils

The `EffectUtils` class provides utility functions for enhancing LED effects, including energy-saving mode adjustments.

##### applyEnergySavingMode Function

The `applyEnergySavingMode()` function modifies the color of LEDs based on the specified energy-saving mode level. It takes a color value as input and returns the adjusted color according to the energy-saving mode level.

```c++
uint32_t EffectUtils::applyEnergySavingMode(uint32_t color);
```

Integrate this function into your LED effect code to dynamically adjust LED intensity based on the selected energy-saving mode level.

## BLE Commands
The BLE control layer now supports robust, low-latency communication for both rehearsals and live performances. Interactions follow a UART-style characteristic that accepts UTF-8 payloads terminated with newlines. Each command returns an acknowledgment code so companion apps can confirm state transitions.

### Command Overview

- `EffectType`: Change the LED effect (Rainbow, Fire, Pulse, PixelImage, etc.).
- `EffectParameter`: Provide structured JSON data for tuning parameters such as speed, frame rate, and gyroscope thresholds.
- `SolidColor`: Set a solid color using RGB values.
- `ImageUpload`: Stream new pixel art frames to the controller using run-length encoded payloads.
- `EnergySavingMode`: Adjust energy-saving mode levels (0 to 3).
- `SystemStatus`: Query battery percentage, internal temperature, or firmware version.
- `Reboot`: Restart the controller after firmware updates or persistent faults.

### BLE Service Architecture

- **Advertising:** The hoop exposes a custom OpenHoop service UUID with a readable device name and battery level characteristic for quick pairing.
- **Command Channel:** A UART-like characteristic handles effect control, supporting JSON payloads with checksum validation to ensure reliable updates.
- **Telemetry Channel:** Subscribe to this notification characteristic to receive frame timing metrics, IMU data, and power usage stats in real time.
- **Security Considerations:** Optionally enable passkey pairing for public performances. The firmware caches trusted devices and re-establishes encrypted connections automatically.

### Recommended Companion App Flows

1. **Discovery:** Scan for devices advertising `OpenHoop-POV` and display RSSI to help performers locate their hoop.
2. **Preset Selection:** Load saved presets, then send `EffectType` followed by `EffectParameter` to configure nuance.
3. **Live Preview:** Stream color previews to the telemetry channel to confirm updates without spinning the hoop.
4. **Diagnostics:** On disconnect, prompt the user to save telemetry logs for future debugging and share them with the community.

## Troubleshooting Toolkit

Stay performance-ready with a proactive troubleshooting approach.

### Common Scenarios

- **Uneven POV Images:** Recalibrate timing using the `EffectParameter` command to tweak frame offsets. Check for loose wiring that may introduce flicker.
- **BLE Dropouts:** Confirm the hoop battery is above 20%, then review logs from the telemetry channel. Move interfering Wi-Fi devices away from the 2.4 GHz band.
- **Image Upload Failures:** Ensure payloads respect the maximum MTU size. The controller emits an error code through the command channel when it needs a retransmission.

### Diagnostic Tools

- **BLE Sniffer Walkthrough:** Follow the step-by-step guide in `tools/ble-sniffer.md` for capturing packets during pairing and effect switching.
- **Pixel Calibration Script:** Use `tools/calibration/pixel_tuner.py` to sweep gamma curves and map LED positions after repairs.
- **Firmware Recovery:** If a transfer fails mid-way, restart in safe mode (hold the button during power-up) and re-flash via USB before reattempting uploads.

