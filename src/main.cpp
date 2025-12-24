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
#include "../include/Config.h"  // Must be first for feature flags

#ifdef FEATURE_IMU
#include <Arduino_LSM9DS1.h>
#include "../include/utils/MadgwickFilter.h"
#endif

#include "../include/services/BleService.h"
#include "../include/services/EffectService.h"
#include "../include/utils/EffectUtils.h"

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
#ifdef FEATURE_IMU
unsigned long lastIMUUpdateMs = 0;
MadgwickFilter madgwick(20.0f);  // 20Hz sample rate (1000ms / 50ms interval)
// Gyro bias calibration values (measured at startup)
float gyroBiasX = 0, gyroBiasY = 0, gyroBiasZ = 0;
bool gyroCalibrated = false;
#endif
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
    // 0-7: Standard effects (NO_EFFECT through MUSHROOM)
    // 80-86: Pattern presets
    // 87-91: Sound-reactive patterns
    // 95-99: Special effects (WATER, CALIBRATE, POV, PULSE, SPECTRUM)
    return (effectValue <= static_cast<uint8_t>(EffectType::MUSHROOM)) ||
           (effectValue >= 80 && effectValue <= 91) ||
           (effectValue >= 95 && effectValue <= 99);
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
#ifdef FEATURE_IMU
void updateIMU();
#endif
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

#ifdef FEATURE_SOUND
    // Initialize the PDM library for sound processing
    PDM.onReceive(EffectUtils::onPDMdata);
#endif

#ifdef FEATURE_IMU
    // Initialize IMU for motion-reactive effects
    // May need retries on some boards
    DEBUG_PRINTLN("Initializing IMU...");
    bool imuOk = false;
    for (int attempt = 0; attempt < 3; attempt++) {
        if (IMU.begin()) {
            imuOk = true;
            DEBUG_PRINTLN("IMU initialized successfully!");
            break;
        }
        DEBUG_PRINT("IMU init attempt ");
        DEBUG_PRINT(attempt + 1);
        DEBUG_PRINTLN(" failed, retrying...");
        delay(100);
    }
    if (!imuOk) {
        DEBUG_PRINTLN("WARNING: IMU failed to initialize! Motion effects won't work.");
    } else {
        // Check magnetometer - may need a moment after IMU.begin()
        DEBUG_PRINT("Magnetometer sample rate: ");
        DEBUG_PRINT(IMU.magneticFieldSampleRate());
        DEBUG_PRINTLN(" Hz");

        // Wait for magnetometer data (up to 500ms)
        float mx, my, mz;
        bool magAvailable = false;
        for (int i = 0; i < 50; i++) {
            if (IMU.magneticFieldAvailable()) {
                if (IMU.readMagneticField(mx, my, mz)) {
                    magAvailable = true;
                    break;
                }
            }
            delay(10);
        }

        if (magAvailable) {
            DEBUG_PRINTLN("Magnetometer available - 9-DOF mode enabled");
            DEBUG_PRINT("Initial mag: X=");
            DEBUG_PRINT(mx, 1);
            DEBUG_PRINT(" Y=");
            DEBUG_PRINT(my, 1);
            DEBUG_PRINT(" Z=");
            DEBUG_PRINTLN(mz, 1);
            // Increase beta for better stability with magnetometer
            madgwick.setBeta(0.2f);  // Higher beta = faster convergence, more stable
        } else {
            DEBUG_PRINTLN("WARNING: Magnetometer not available - yaw will drift!");
            DEBUG_PRINTLN("(LSM9DS1 mag uses I2C addr 0x1C, accel/gyro use 0x6A)");
            madgwick.setBeta(0.1f);  // Lower beta for 6-DOF mode
        }

        // Calibrate gyro bias (device should be stationary during startup)
        DEBUG_PRINTLN("Calibrating gyro bias (keep still)...");
        constexpr int kCalibrationSamples = 100;
        float sumGx = 0, sumGy = 0, sumGz = 0;
        int validSamples = 0;
        for (int i = 0; i < kCalibrationSamples; i++) {
            float gx, gy, gz;
            if (IMU.gyroscopeAvailable() && IMU.readGyroscope(gx, gy, gz)) {
                sumGx += gx;
                sumGy += gy;
                sumGz += gz;
                validSamples++;
            }
            delay(10);  // ~100 samples over 1 second
        }
        if (validSamples > 50) {
            gyroBiasX = sumGx / validSamples;
            gyroBiasY = sumGy / validSamples;
            gyroBiasZ = sumGz / validSamples;
            gyroCalibrated = true;
            DEBUG_PRINT("Gyro bias: X=");
            DEBUG_PRINT(gyroBiasX, 2);
            DEBUG_PRINT(" Y=");
            DEBUG_PRINT(gyroBiasY, 2);
            DEBUG_PRINT(" Z=");
            DEBUG_PRINTLN(gyroBiasZ, 2);
        } else {
            DEBUG_PRINTLN("Gyro calibration failed - not enough samples");
        }
    }

    // Load saved calibration from flash (if available)
    if (EffectUtils::loadCalibration()) {
        DEBUG_PRINTLN("Previous calibration restored");
    }
#endif

    // Initialize DotStar hoop
    hoop.begin();
    hoop.show();

    // Initialize BLE services BEFORE starting effects
    // (Some effects like SPECTRUM use PDM which can conflict with BLE if started first)
    if (!bleService.beginAndAdvertise()) {
        while (true) {
            delay(1000);
        }
    }

    // Now start a random boot effect
    const EffectType bootEffect = pickRandomBootEffect();
    effectService->dispatchEffectCommand(bootEffect);
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

#ifdef FEATURE_IMU
// Stationary detection threshold (deg/s after bias removal)
constexpr float kStationaryGyroThreshold = 5.0f;  // Increased from 2.0
constexpr float kStationaryBeta = 0.5f;  // High beta when stationary for fast convergence
constexpr float kMovingBeta = 0.1f;      // Lower beta when moving for smooth tracking
bool wasStationary = false;

void updateIMU() {
    const unsigned long now = millis();
    if ((now - lastIMUUpdateMs) < kIMUUpdateIntervalMs) {
        return;
    }
    lastIMUUpdateMs = now;

    float ax, ay, az, gx, gy, gz, mx, my, mz;
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
        IMU.readAcceleration(ax, ay, az);
        IMU.readGyroscope(gx, gy, gz);

        // Subtract gyro bias (calibrated at startup)
        if (gyroCalibrated) {
            gx -= gyroBiasX;
            gy -= gyroBiasY;
            gz -= gyroBiasZ;
        }

        // Stationary detection: if gyro is nearly zero, we're not rotating
        float gyroMagnitude = sqrtf(gx * gx + gy * gy + gz * gz);
        bool isStationary = gyroMagnitude < kStationaryGyroThreshold;

        if (isStationary) {
            // Device is stationary - zero gyro and boost beta for instant convergence
            gx = gy = gz = 0.0f;
            madgwick.setBeta(kStationaryBeta);
        } else if (wasStationary) {
            // Just started moving - restore normal beta
            madgwick.setBeta(kMovingBeta);
        }
        wasStationary = isStationary;

        // Convert gyro from deg/s to rad/s for Madgwick
        float gxRad = gx * DEG_TO_RAD;
        float gyRad = gy * DEG_TO_RAD;
        float gzRad = gz * DEG_TO_RAD;

        // Update Madgwick filter - use magnetometer if available for stable yaw
        if (IMU.magneticFieldAvailable() && IMU.readMagneticField(mx, my, mz)) {
            // 9-DOF mode: accel + gyro + magnetometer (no yaw drift!)
            madgwick.update(gxRad, gyRad, gzRad, ax, ay, az, mx, my, mz);
        } else {
            // 6-DOF fallback: accel + gyro only (yaw will drift)
            madgwick.update(gxRad, gyRad, gzRad, ax, ay, az);
        }

        // Send filtered orientation via BLE
        // Pack: roll, pitch, yaw (as int16 * 100 for precision) + raw accel for effects
        bleService.updateIMUData(
            madgwick.getRoll(),
            madgwick.getPitch(),
            madgwick.getYaw(),
            ax, ay, az
        );
    }
}
#endif

} // namespace

void loop() {
    // Update BLE communication, check for incoming commands
    updateBLE();

    // Update battery level based on analog reading
    updateBatteryLevel();

#ifdef FEATURE_IMU
    // Stream IMU data to BLE (for visualization)
    updateIMU();
#endif

    // Update LED effects
    updateEffects();

    // Apply inactivity policies
    handleInactivity();
}
