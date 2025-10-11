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

// LED configuration for the hoop.
#define NUM_LEDS 288  ///< Number of LEDs in the hoop.
#define LEDS_DATA_PIN 11  ///< Data pin for the DotStar LED strip.
#define LEDS_CLOCK_PIN 13 ///< Clock pin for the DotStar LED strip.

// Bluetooth's configuration.
#define EFFECT_SERVICE_UUID "0A92"  ///< UUID for the effect service.
#define SCOLOR_SERVICE_UUID "0A93"  ///< UUID for the solid color service.
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
