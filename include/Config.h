/**
 * @project OpenHoop
 * @file Config.h
 * @brief Configuration file for the OpenHoop project.
 * @details This file contains pin configurations, constants, and UUIDs used throughout the project.
 * @author github.com/angelcamelot
 * @date 2024-03-17
 * @license Open-source license.
 */

#ifndef OPENHOOP_CONFIG_H
#define OPENHOOP_CONFIG_H

// ============================================================================
// FEATURE FLAGS - Comment out to disable features and reduce code size
// ============================================================================
#define FEATURE_BLE           // Bluetooth Low Energy control
#define FEATURE_IMU           // IMU for motion-reactive effects (Water, Calibrate)
#define FEATURE_SOUND         // PDM microphone for sound-reactive effects
#define FEATURE_SERIAL_DEBUG  // Serial output for debugging (disable in production)

// Production mode: uncomment to disable all debug output
// #define PRODUCTION_MODE

#ifdef PRODUCTION_MODE
  #undef FEATURE_SERIAL_DEBUG
#endif

// Debug print macros - compile to nothing in production
#ifdef FEATURE_SERIAL_DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(fmt, ...)
#endif

// ============================================================================
// LED STRIP TYPE SELECTION
// ============================================================================
// Uncomment ONE of the following to select your LED strip type:
// #define USE_DOTSTAR    // APA102/DotStar - 2-wire (data + clock), higher refresh rates
#define USE_NEOPIXEL   // WS2812B/NeoPixel - 1-wire (data only), more common/cheaper

// Must include HulaHoopDotStar AFTER defining USE_DOTSTAR or USE_NEOPIXEL
#include "utils/HulaHoopDotStar.h"

// Pin configuration for reading battery voltage.
#define BATTERY_ANALOG_PIN 21  ///< Analog pin used for reading battery voltage.
#define ANALOG_READ_RESOLUTION_BITS 12  ///< ADC resolution used for battery readings.
#define BATTERY_MAX_READING 4095.0  ///< Maximum ADC reading for the configured resolution.
#define BATTERY_SAMPLE_COUNT 8  ///< Number of samples taken to smooth the battery reading.
#define BATTERY_MIN_VOLTAGE 7  ///< Minimum voltage considered for the battery.
#define BATTERY_MAX_VOLTAGE 8.4  ///< Maximum voltage for a fully charged battery.
#define R1 30000.0  ///< Resistance value R1 in Ohms.
#define R2 7500.0   ///< Resistance value R2 in Ohms.
#define REF_VOLTAGE 3.3  ///< Reference voltage in volts.

// ============================================================================
// LED STRIP CONFIGURATION
// ============================================================================
#define NUM_LEDS 288  ///< Number of LEDs in the hoop.

// LED geometry - adjust based on physical hoop layout
#define LED_COVERAGE_DEGREES 360.0f   ///< Degrees of circle covered by LEDs (set to actual coverage if gap exists)
#define ARDUINO_LED_OFFSET_DEGREES 5.0f ///< Angular offset from Arduino to LED #1

#ifdef USE_DOTSTAR
  // DotStar (APA102) uses 2 pins: data + clock
  #define LEDS_DATA_PIN 11   ///< Data pin for the DotStar LED strip.
  #define LEDS_CLOCK_PIN 13  ///< Clock pin for the DotStar LED strip.
  #define LED_COLOR_ORDER DOTSTAR_BGR  ///< Color order for DotStar (BGR is common)
#elif defined(USE_NEOPIXEL)
  // NeoPixel (WS2812B/SK6812) uses 1 pin: data only
  #define LEDS_DATA_PIN 4    ///< Data pin for the NeoPixel LED strip (D4).
  #define LED_COLOR_ORDER NEO_GRBW  ///< Color order for SK6812 RGBW strips
#else
  #error "You must define either USE_DOTSTAR or USE_NEOPIXEL in Config.h"
#endif

// Bluetooth's configuration.
#define EFFECT_SERVICE_UUID "0A92"  ///< UUID for the effect service.
#define SCOLOR_SERVICE_UUID "0A93"  ///< UUID for the solid color service.
#define IMU_SERVICE_UUID "0A94"     ///< UUID for the IMU data stream.
#define ENERGY_SERVICE_UUID "0A95"  ///< UUID for the energy saving service.
#define SOLID_COLOR_DEFAULT_VALUE "NO COLOR"  ///< Default value used for the solid color characteristic.
#define SOLID_COLOR_MAX_LENGTH 8  ///< Maximum length for the solid color characteristic string.
#define DEFAULT_EFFECT_VALUE 255  ///< Default value used for the effect characteristic (idle state).
#define DEFAULT_ENERGY_SAVING_MODE 0  ///< Default energy-saving mode value.
#define ENERGY_SAVING_MAX_LEVEL 8  ///< Maximum allowed energy-saving mode level.
#define BLE_DEVICE_APPEARANCE 0x04C0  ///< BLE appearance code for LED toys.
#define BLE_ADVERTISING_INTERVAL 152  ///< Advertising interval in units of 0.625 ms.
#define MANUFACTURER_CHARACTERISTIC "OpenHoop"  ///< Manufacturer characteristic.
#define MODEL_CHARACTERISTIC "HulaHoopBLE"  ///< Model characteristic.
#define SERIAL_NUMBER_CHARACTERISTIC "HH-BLE-1"  ///< Serial number characteristic.
#define FIRMWARE_CHARACTERISTIC "v1.0.0"  ///< Firmware version characteristic.

// DotStar hoop object.
extern HulaHoopDotStar hoop;  ///< Object representing the DotStar hoop.

#endif //OPENHOOP_CONFIG_H
