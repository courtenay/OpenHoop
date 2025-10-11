/**
 * @project OpenHoop
 * @file main.cpp
 * @brief Main program file for the OpenHoop project.
 * @details This file contains the setup and loop functions for the HulaHoop device, handling BLE communication,
 * updating battery levels, and managing LED effects.
 * @author github.com/angelcamelot
 * @date 2024-03-17
 * @license Open-source license.
 */

#include <Arduino.h>
#include "../include/services/BleService.h"
#include "../include/services/EffectService.h"
#include "../include/utils/EffectUtils.h"
#include "../include/Config.h"

namespace {
constexpr unsigned long kBatteryUpdateIntervalMs = 1000UL;
constexpr uint8_t kBatterySampleCount = BATTERY_SAMPLE_COUNT;
constexpr float kAdcToVoltage = REF_VOLTAGE / BATTERY_MAX_READING;
constexpr float kVoltageDividerRatio = (R1 + R2) / R2;
constexpr uint8_t kMaxEnergySavingLevel = ENERGY_SAVING_MAX_LEVEL;
constexpr uint8_t kColorBufferSize = BleService::SOLID_COLOR_VALUE_SIZE + 1;

unsigned long lastBatteryUpdateMs = 0;
bool isCentralConnected = false;

constexpr bool isValidEffect(uint8_t effectValue) {
    return (effectValue <= static_cast<uint8_t>(EffectType::MUSHROOM)) ||
           (effectValue == static_cast<uint8_t>(EffectType::PULSE)) ||
           (effectValue == static_cast<uint8_t>(EffectType::SPECTRUM));
}

uint8_t clampEnergySavingLevel(uint8_t level) {
    return level > kMaxEnergySavingLevel ? kMaxEnergySavingLevel : level;
}
} // namespace

HulaHoopDotStar hoop(NUM_LEDS, LEDS_DATA_PIN, LEDS_CLOCK_PIN);

BleService bleService;
std::unique_ptr<EffectService> effectService = std::make_unique<EffectService>();

void setup() {
    Serial.begin(115200);
    unsigned long serialStart = millis();
    while (!Serial && (millis() - serialStart) < 2000UL) {
        delay(10);
    }

    analogReadResolution(ANALOG_READ_RESOLUTION_BITS);

    // Initialize the PDM library for sound processing
    PDM.onReceive(EffectUtils::onPDMdata);

    // Initialize DotStar hoop
    hoop.begin();
    hoop.show();

    // Initialize DotStar BLE services
    if (!bleService.beginAndAdvertise()) {
        while (true) {
            delay(1000);
        }
    }

    lastBatteryUpdateMs = millis() - kBatteryUpdateIntervalMs;
}

/**
 * @brief Update BLE communication, check for incoming commands.
 * Handles color code writes, gesture commands, and energy-saving mode changes.
 */
void updateBLE() {
    BLE.poll();

    BLEDevice central = BLE.central();
    if (central && central.connected()) {
        if (!isCentralConnected) {
            isCentralConnected = true;
        }

        if (bleService.solidColorCharacteristic.written()) {
            char colorBuffer[kColorBufferSize] = {};
            const int length = bleService.solidColorCharacteristic.readValue(colorBuffer, BleService::SOLID_COLOR_VALUE_SIZE);
            if (length > 0) {
                colorBuffer[length < BleService::SOLID_COLOR_VALUE_SIZE ? length : BleService::SOLID_COLOR_VALUE_SIZE] = '\0';
                effectService->dispatchSolidColorCommand(String(colorBuffer));
                bleService.effectCharacteristic.writeValue(DEFAULT_EFFECT_VALUE);
            }
        }

        if (bleService.effectCharacteristic.written()) {
            const uint8_t effectValue = bleService.effectCharacteristic.value();
            if (isValidEffect(effectValue)) {
                effectService->dispatchEffectCommand(static_cast<EffectType>(effectValue));
                bleService.solidColorCharacteristic.writeValue(SOLID_COLOR_DEFAULT_VALUE);
            }
        }

        if (bleService.energySavingModeCharacteristic.written()) {
            const uint8_t requestedMode = bleService.energySavingModeCharacteristic.value();
            const uint8_t clampedMode = clampEnergySavingLevel(requestedMode);
            hoop.setEnergySavingMode(clampedMode);
            if (clampedMode != requestedMode) {
                bleService.energySavingModeCharacteristic.writeValue(clampedMode);
            }
        }
    } else if (isCentralConnected) {
        bleService.resetControlCharacteristics();
        hoop.setEnergySavingMode(DEFAULT_ENERGY_SAVING_MODE);
        BLE.advertise();
        isCentralConnected = false;
    }
}

/**
 * @brief Update battery level based on the analog reading.
 */
void updateBatteryLevel() {
    const unsigned long now = millis();
    if ((now - lastBatteryUpdateMs) < kBatteryUpdateIntervalMs) {
        return;
    }
    lastBatteryUpdateMs = now;

    uint32_t accumulatedReading = 0;
    for (uint8_t sample = 0; sample < kBatterySampleCount; ++sample) {
        accumulatedReading += static_cast<uint32_t>(analogRead(BATTERY_ANALOG_PIN));
        delayMicroseconds(50);
    }

    const float averageReading = static_cast<float>(accumulatedReading) / kBatterySampleCount;
    const float adcVoltage = averageReading * kAdcToVoltage;
    const float batteryVoltage = adcVoltage * kVoltageDividerRatio;

    bleService.updateBatteryLevel(batteryVoltage);
}

void loop() {
    // Update BLE communication, check for incoming commands
    updateBLE();

    // Update battery level based on analog reading
    updateBatteryLevel();

    // Update LED effects
    effectService->update();
}
