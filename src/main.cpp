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
#include <nrf.h>
#include <Arduino_LSM9DS1.h>
#include "../include/services/BleService.h"
#include "../include/services/EffectService.h"
#include "../include/utils/EffectUtils.h"
#include "../include/Config.h"

namespace {
constexpr unsigned long kBatteryUpdateIntervalMs = 1000UL;
constexpr unsigned long kIMUUpdateIntervalMs = 50UL;  // 20Hz IMU streaming
constexpr uint8_t kBatterySampleCount = BATTERY_SAMPLE_COUNT;
constexpr float kAdcToVoltage = REF_VOLTAGE / BATTERY_MAX_READING;
constexpr float kVoltageDividerRatio = (R1 + R2) / R2;
constexpr uint8_t kMaxEnergySavingLevel = ENERGY_SAVING_MAX_LEVEL;
constexpr uint8_t kColorBufferSize = BleService::SOLID_COLOR_VALUE_SIZE + 1;
constexpr unsigned long kBleDimTimeoutMs = 120000UL;
constexpr unsigned long kBleDeepSleepTimeoutMs = 600000UL;
constexpr unsigned long kNormalEffectUpdateIntervalMs = 0UL;
constexpr unsigned long kReducedEffectUpdateIntervalMs = 50UL;
constexpr uint8_t kInactivityBrightnessLevel = static_cast<uint8_t>((255U * 30U + 50U) / 100U);

unsigned long lastBatteryUpdateMs = 0;
unsigned long lastBleActivityMs = 0;
unsigned long lastEffectUpdateMs = 0;
unsigned long lastIMUUpdateMs = 0;
bool isCentralConnected = false;
bool inactivityDimmed = false;
uint8_t storedEnergySavingModeLevel = DEFAULT_ENERGY_SAVING_MODE;
uint8_t storedBrightnessBeforeDim = 255;

constexpr EffectType kBootEffects[] = {
    EffectType::RAINBOW,
    EffectType::COLORWAVE,
    EffectType::FUNKY,
    EffectType::RASTAFARAIFLAG,
    EffectType::FIRE,
    EffectType::LEOPARDRAINBOW,
    EffectType::MUSHROOM,
    EffectType::PULSE,
    EffectType::SPECTRUM
};

constexpr bool isValidEffect(uint8_t effectValue) {
    return (effectValue <= static_cast<uint8_t>(EffectType::MUSHROOM)) ||
           (effectValue == static_cast<uint8_t>(EffectType::WATER)) ||
           (effectValue == static_cast<uint8_t>(EffectType::CALIBRATE)) ||
           (effectValue == static_cast<uint8_t>(EffectType::POV)) ||
           (effectValue == static_cast<uint8_t>(EffectType::PULSE)) ||
           (effectValue == static_cast<uint8_t>(EffectType::SPECTRUM));
}

uint8_t clampEnergySavingLevel(uint8_t level) {
    return level > kMaxEnergySavingLevel ? kMaxEnergySavingLevel : level;
}

EffectType pickRandomBootEffect() {
    constexpr size_t kEffectCount = sizeof(kBootEffects) / sizeof(kBootEffects[0]);
    if (kEffectCount == 0) {
        return EffectType::NO_EFFECT;
    }

    const size_t selectedIndex = static_cast<size_t>(random(static_cast<long>(kEffectCount)));
    return kBootEffects[selectedIndex];
}

void seedRandomGenerator() {
    const long entropy = static_cast<long>(analogRead(BATTERY_ANALOG_PIN)) ^ static_cast<long>(micros());
    randomSeed(entropy);
}

void markBleActivity();
void updateEffects();
void handleInactivity();
void applyInactivityDimming();
void restoreFromInactivityDim();
void enterDeepSleep();
void updateIMU();
} // namespace

// Instantiate the LED strip based on configuration
#ifdef USE_DOTSTAR
  HulaHoopDotStar hoop(NUM_LEDS, LEDS_DATA_PIN, LEDS_CLOCK_PIN, LED_COLOR_ORDER);
#elif defined(USE_NEOPIXEL)
  HulaHoopDotStar hoop(NUM_LEDS, LEDS_DATA_PIN, LED_COLOR_ORDER);
#endif

BleService bleService;
std::unique_ptr<EffectService> effectService = std::make_unique<EffectService>();

void setup() {
    Serial.begin(115200);
    unsigned long serialStart = millis();
    while (!Serial && (millis() - serialStart) < 2000UL) {
        delay(10);
    }

    analogReadResolution(ANALOG_READ_RESOLUTION_BITS);

    seedRandomGenerator();

    // Initialize the PDM library for sound processing
    PDM.onReceive(EffectUtils::onPDMdata);

    // Initialize IMU for motion-reactive effects
    // May need retries on some boards
    Serial.println("Initializing IMU...");
    bool imuOk = false;
    for (int attempt = 0; attempt < 3; attempt++) {
        if (IMU.begin()) {
            imuOk = true;
            Serial.println("IMU initialized successfully!");
            break;
        }
        Serial.print("IMU init attempt ");
        Serial.print(attempt + 1);
        Serial.println(" failed, retrying...");
        delay(100);
    }
    if (!imuOk) {
        Serial.println("WARNING: IMU failed to initialize! Motion effects won't work.");
    }

    // Initialize DotStar hoop
    hoop.begin();
    hoop.show();

    const EffectType bootEffect = pickRandomBootEffect();
    effectService->dispatchEffectCommand(bootEffect);

    // Initialize DotStar BLE services
    if (!bleService.beginAndAdvertise()) {
        while (true) {
            delay(1000);
        }
    }

    bleService.effectCharacteristic.writeValue(static_cast<uint8_t>(bootEffect));

    lastBatteryUpdateMs = millis() - kBatteryUpdateIntervalMs;
    lastBleActivityMs = millis();
    storedEnergySavingModeLevel = hoop.getEnergySavingModeLevel();
    storedBrightnessBeforeDim = hoop.getBrightnessLevel();
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
            markBleActivity();
        }

        if (bleService.solidColorCharacteristic.written()) {
            markBleActivity();
            char colorBuffer[kColorBufferSize] = {};
            const int length = bleService.solidColorCharacteristic.readValue(colorBuffer, BleService::SOLID_COLOR_VALUE_SIZE);
            if (length > 0) {
                colorBuffer[length < BleService::SOLID_COLOR_VALUE_SIZE ? length : BleService::SOLID_COLOR_VALUE_SIZE] = '\0';
                effectService->dispatchSolidColorCommand(String(colorBuffer));
                bleService.effectCharacteristic.writeValue(DEFAULT_EFFECT_VALUE);
            }
        }

        if (bleService.effectCharacteristic.written()) {
            markBleActivity();
            const uint8_t effectValue = bleService.effectCharacteristic.value();
            if (isValidEffect(effectValue)) {
                effectService->dispatchEffectCommand(static_cast<EffectType>(effectValue));
                bleService.solidColorCharacteristic.writeValue(SOLID_COLOR_DEFAULT_VALUE);
            }
        }

        if (bleService.energySavingModeCharacteristic.written()) {
            markBleActivity();
            const uint8_t requestedMode = bleService.energySavingModeCharacteristic.value();
            const uint8_t clampedMode = clampEnergySavingLevel(requestedMode);
            hoop.setEnergySavingMode(clampedMode);
            storedEnergySavingModeLevel = hoop.getEnergySavingModeLevel();
            storedBrightnessBeforeDim = hoop.getBrightnessLevel();
            if (clampedMode != requestedMode) {
                bleService.energySavingModeCharacteristic.writeValue(clampedMode);
            }
        }
    } else if (isCentralConnected) {
        bleService.resetControlCharacteristics();
        hoop.setEnergySavingMode(DEFAULT_ENERGY_SAVING_MODE);
        storedEnergySavingModeLevel = hoop.getEnergySavingModeLevel();
        storedBrightnessBeforeDim = hoop.getBrightnessLevel();
        BLE.advertise();
        isCentralConnected = false;
        lastBleActivityMs = millis();
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

namespace {

void restoreFromInactivityDim() {
    if (!inactivityDimmed) {
        return;
    }

    hoop.setEnergySavingMode(storedEnergySavingModeLevel);
    hoop.setDirectBrightness(storedBrightnessBeforeDim);
    hoop.show();
    storedEnergySavingModeLevel = hoop.getEnergySavingModeLevel();
    storedBrightnessBeforeDim = hoop.getBrightnessLevel();
    inactivityDimmed = false;
    lastEffectUpdateMs = millis();
}

void applyInactivityDimming() {
    if (inactivityDimmed) {
        return;
    }

    storedEnergySavingModeLevel = hoop.getEnergySavingModeLevel();
    storedBrightnessBeforeDim = hoop.getBrightnessLevel();
    hoop.setDirectBrightness(kInactivityBrightnessLevel);
    hoop.show();
    inactivityDimmed = true;
    lastEffectUpdateMs = millis();
}

void markBleActivity() {
    lastBleActivityMs = millis();
    if (inactivityDimmed) {
        restoreFromInactivityDim();
    }
}

void updateEffects() {
    const unsigned long now = millis();
    const unsigned long interval = inactivityDimmed ? kReducedEffectUpdateIntervalMs : kNormalEffectUpdateIntervalMs;
    if ((now - lastEffectUpdateMs) >= interval) {
        effectService->update();
        lastEffectUpdateMs = now;
    }
}

void handleInactivity() {
    const unsigned long now = millis();
    const unsigned long elapsed = now - lastBleActivityMs;

    if (!inactivityDimmed && elapsed >= kBleDimTimeoutMs) {
        applyInactivityDimming();
    }

    if (elapsed >= kBleDeepSleepTimeoutMs) {
        enterDeepSleep();
    }
}

void enterDeepSleep() {
    BLE.stopAdvertise();
    BLE.disconnect();
    BLE.end();

    hoop.fill(0);
    hoop.show();

    PDM.end();

    delay(10);

    NRF_POWER->SYSTEMOFF = 1;
    while (true) {
        __WFE();
    }
}

void updateIMU() {
    const unsigned long now = millis();
    if ((now - lastIMUUpdateMs) < kIMUUpdateIntervalMs) {
        return;
    }
    lastIMUUpdateMs = now;

    float ax, ay, az, gx, gy, gz;
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
        IMU.readAcceleration(ax, ay, az);
        IMU.readGyroscope(gx, gy, gz);
        bleService.updateIMUData(ax, ay, az, gx, gy, gz);
    }
}

} // namespace

void loop() {
    // Update BLE communication, check for incoming commands
    updateBLE();

    // Update battery level based on analog reading
    updateBatteryLevel();

    // Stream IMU data to BLE (for visualization)
    updateIMU();

    // Update LED effects
    updateEffects();

    // Apply inactivity policies
    handleInactivity();
}
