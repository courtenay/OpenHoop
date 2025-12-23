/**
* @project OpenHoop
 * @file EffectUtils.cpp
 * @brief Source file for the EffectUtils class, containing utility functions for LED effects.
 * @details Implements static methods for applying brightness, interpolating colors, converting HSV to RGB, mapping values,
 *          generating colors based on wheel position, calculating sound spectrum intensity, and managing energy-saving mode.
 * @author github.com/angelcamelot
 * @date 2024-03-17
 * @license Open-source license.
 */

#include "../../include/utils/EffectUtils.h"
#include "../../include/Config.h"
#include <Arduino_LSM9DS1.h>
#include <mbed.h>
#include <FlashIAP.h>

// Flash storage for calibration persistence
namespace {
    // Magic number to validate stored data
    constexpr uint32_t CALIBRATION_MAGIC = 0xCAFE1234;

    // Structure for flash storage (must be aligned to 4 bytes)
    struct CalibrationStorage {
        uint32_t magic;
        float baselineX;
        float baselineY;
        float baselineZ;
        float ledOffsetAngle;
        uint32_t checksum;
    };

    // Use last sector of flash for storage (safe area)
    mbed::FlashIAP flash;

    uint32_t calculateChecksum(const CalibrationStorage& data) {
        uint32_t sum = data.magic;
        sum ^= *reinterpret_cast<const uint32_t*>(&data.baselineX);
        sum ^= *reinterpret_cast<const uint32_t*>(&data.baselineY);
        sum ^= *reinterpret_cast<const uint32_t*>(&data.baselineZ);
        sum ^= *reinterpret_cast<const uint32_t*>(&data.ledOffsetAngle);
        return sum;
    }
}

/**
 * @brief Interpolate color between two given colors.
 */
uint32_t EffectUtils::interpolateColor(uint32_t color1, uint32_t color2, int step, int steps) {
    uint8_t r = map(step, 0, steps, static_cast<uint8_t>((color1 >> 16) & 0xFF), static_cast<uint8_t>((color2 >> 16) & 0xFF));
    uint8_t g = map(step, 0, steps, static_cast<uint8_t>((color1 >> 8) & 0xFF), static_cast<uint8_t>((color2 >> 8) & 0xFF));
    uint8_t b = map(step, 0, steps, static_cast<uint8_t>(color1 & 0xFF), static_cast<uint8_t>(color2 & 0xFF));
    return HulaHoopDotStar::Color(r, g, b);
}

/**
 * @brief Convert HSV color to RGB.
 */
uint32_t EffectUtils::HSVtoRGB(int hue, int saturation, int value) {
    int chroma = (value * saturation) / 255;
    int h = hue / 43; // sector 0 to 5
    int x = chroma * (1 - abs((h % 2) - 1));
    int r, g, b;

    switch (h) {
        case 0: r = chroma; g = x; b = 0; break;
        case 1: r = x; g = chroma; b = 0; break;
        case 2: r = 0; g = chroma; b = x; break;
        case 3: r = 0; g = x; b = chroma; break;
        case 4: r = x; g = 0; b = chroma; break;
        default: r = chroma; g = 0; b = x; break;
    }

    int m = value - chroma;
    r += m; g += m; b += m;

    return (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
}

/**
 * @brief Map a value from one range to another.
 */
int EffectUtils::mapRange(int value, int fromLow, int fromHigh, int toLow, int toHigh) {
    return map(constrain(value, fromLow, fromHigh), fromLow, fromHigh, toLow, toHigh);
}

/**
 * @brief Generate color based on wheel position for the rainbow effect.
 */
uint32_t EffectUtils::Wheel(byte wheelPos) {
    wheelPos = 255 - wheelPos;
    if (wheelPos < 85) {
        return ((255 - wheelPos * 3) << 16) | (wheelPos * 3);
    } else if (wheelPos < 170) {
        wheelPos -= 85;
        return (wheelPos * 3) | ((255 - wheelPos * 3) << 8);
    } else {
        wheelPos -= 170;
        return ((wheelPos * 3) << 16) | (255 - wheelPos * 3);
    }
}

/**
 * @brief Apply brightness to a given color.
 */
uint32_t EffectUtils::applyBrightness(uint32_t color, int brightness) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    r = (r * brightness) / 255;
    g = (g * brightness) / 255;
    b = (b * brightness) / 255;

    return HulaHoopDotStar::Color(r, g, b);
}

/**
 * @brief Static array to store audio sample data.
 */
short EffectUtils::sampleBuffer[256];

/**
 * @brief Static variable to track the number of samples read.
 */
volatile int EffectUtils::samplesRead;

/**
 * @brief Static IMU calibration data.
 */
IMUCalibration EffectUtils::calibration;

/**
 * @brief Calibrate IMU by capturing current orientation as baseline.
 */
void EffectUtils::calibrateIMU() {
    float x, y, z;

    // Average multiple readings for stability
    float sumX = 0, sumY = 0, sumZ = 0;
    const int samples = 10;

    for (int i = 0; i < samples; i++) {
        if (IMU.readAcceleration(x, y, z)) {
            sumX += x;
            sumY += y;
            sumZ += z;
        }
        delay(10);
    }

    calibration.baselineX = sumX / samples;
    calibration.baselineY = sumY / samples;
    calibration.baselineZ = sumZ / samples;
    calibration.isCalibrated = true;

    DEBUG_PRINTLN("=== IMU Calibrated ===");
    DEBUG_PRINT("Baseline - X: ");
    DEBUG_PRINT(calibration.baselineX);
    DEBUG_PRINT(" Y: ");
    DEBUG_PRINT(calibration.baselineY);
    DEBUG_PRINT(" Z: ");
    DEBUG_PRINTLN(calibration.baselineZ);
}

/**
 * @brief Check if IMU has been calibrated.
 */
bool EffectUtils::isIMUCalibrated() {
    return calibration.isCalibrated;
}

/**
 * @brief Get the calibration data.
 */
const IMUCalibration& EffectUtils::getCalibration() {
    return calibration;
}

/**
 * @brief Calibrate LED offset - call when hoop is vertical with Arduino at BOTTOM.
 * This captures the current gravity angle as the reference for "bottom" = Arduino position.
 */
void EffectUtils::calibrateLEDOffset() {
    float x, y, z;

    // Average multiple readings for stability
    float sumX = 0, sumY = 0, sumZ = 0;
    const int samples = 10;

    for (int i = 0; i < samples; i++) {
        if (IMU.readAcceleration(x, y, z)) {
            sumX += x;
            sumY += y;
            sumZ += z;
        }
        delay(10);
    }

    x = sumX / samples;
    y = sumY / samples;
    z = sumZ / samples;

    // Calculate the current gravity angle in the hoop plane
    float angle;
    float absBaseX = fabs(calibration.baselineX);
    float absBaseY = fabs(calibration.baselineY);
    float absBaseZ = fabs(calibration.baselineZ);

    if (absBaseY >= absBaseX && absBaseY >= absBaseZ) {
        angle = atan2(x, z) * RAD_TO_DEG;
    } else if (absBaseX >= absBaseY && absBaseX >= absBaseZ) {
        angle = atan2(y, z) * RAD_TO_DEG;
    } else {
        angle = atan2(x, y) * RAD_TO_DEG;
    }

    // Store this as the offset - when gravity points this direction, Arduino is at bottom
    calibration.ledOffsetAngle = angle;

    DEBUG_PRINTLN("=== LED Offset Calibrated ===");
    DEBUG_PRINT("Offset angle: ");
    DEBUG_PRINT(calibration.ledOffsetAngle);
    DEBUG_PRINTLN(" degrees");

    // Auto-save to flash after calibration
    saveCalibration();
}

/**
 * @brief Save calibration data to persistent flash storage.
 */
void EffectUtils::saveCalibration() {
    if (!calibration.isCalibrated) {
        DEBUG_PRINTLN("No calibration to save");
        return;
    }

    CalibrationStorage storage;
    storage.magic = CALIBRATION_MAGIC;
    storage.baselineX = calibration.baselineX;
    storage.baselineY = calibration.baselineY;
    storage.baselineZ = calibration.baselineZ;
    storage.ledOffsetAngle = calibration.ledOffsetAngle;
    storage.checksum = calculateChecksum(storage);

    flash.init();

    // Get flash geometry
    uint32_t flashSize = flash.get_flash_size();
    uint32_t sectorSize = flash.get_sector_size(flashSize - 1);

    // Use last sector for calibration storage
    uint32_t address = flashSize - sectorSize;

    // Erase the sector first
    flash.erase(address, sectorSize);

    // Write calibration data
    flash.program(&storage, address, sizeof(storage));

    flash.deinit();

    DEBUG_PRINTLN("Calibration saved to flash");
}

/**
 * @brief Load calibration data from persistent flash storage.
 */
bool EffectUtils::loadCalibration() {
    flash.init();

    // Get flash geometry
    uint32_t flashSize = flash.get_flash_size();
    uint32_t sectorSize = flash.get_sector_size(flashSize - 1);

    // Use last sector for calibration storage
    uint32_t address = flashSize - sectorSize;

    // Read calibration data
    CalibrationStorage storage;
    flash.read(&storage, address, sizeof(storage));

    flash.deinit();

    // Validate magic number and checksum
    if (storage.magic != CALIBRATION_MAGIC) {
        DEBUG_PRINTLN("No saved calibration found");
        return false;
    }

    if (storage.checksum != calculateChecksum(storage)) {
        DEBUG_PRINTLN("Calibration data corrupted");
        return false;
    }

    // Restore calibration
    calibration.baselineX = storage.baselineX;
    calibration.baselineY = storage.baselineY;
    calibration.baselineZ = storage.baselineZ;
    calibration.ledOffsetAngle = storage.ledOffsetAngle;
    calibration.isCalibrated = true;

    DEBUG_PRINTLN("=== Calibration Loaded from Flash ===");
    DEBUG_PRINT("Baseline - X: ");
    DEBUG_PRINT(calibration.baselineX);
    DEBUG_PRINT(" Y: ");
    DEBUG_PRINT(calibration.baselineY);
    DEBUG_PRINT(" Z: ");
    DEBUG_PRINTLN(calibration.baselineZ);
    DEBUG_PRINT("LED Offset: ");
    DEBUG_PRINT(calibration.ledOffsetAngle);
    DEBUG_PRINTLN(" degrees");

    return true;
}

/**
 * @brief Get the angle to the bottom of the hoop in LED index space.
 * @return Angle in degrees (0-360) where 0 = LED at Arduino position.
 */
float EffectUtils::getBottomAngle() {
    float x, y, z;

    if (!IMU.accelerationAvailable() || !IMU.readAcceleration(x, y, z)) {
        return 0.0f;
    }

    // Calculate current gravity angle in hoop plane
    float angle;
    float absBaseX = fabs(calibration.baselineX);
    float absBaseY = fabs(calibration.baselineY);
    float absBaseZ = fabs(calibration.baselineZ);

    if (calibration.isCalibrated) {
        if (absBaseY >= absBaseX && absBaseY >= absBaseZ) {
            angle = atan2(x, z) * RAD_TO_DEG;
        } else if (absBaseX >= absBaseY && absBaseX >= absBaseZ) {
            angle = atan2(y, z) * RAD_TO_DEG;
        } else {
            angle = atan2(x, y) * RAD_TO_DEG;
        }
    } else {
        // Fallback: assume Y is vertical
        angle = atan2(x, z) * RAD_TO_DEG;
    }

    // Subtract the offset to get angle relative to Arduino position
    float relativeAngle = angle - calibration.ledOffsetAngle;

    // Normalize to 0-360
    if (relativeAngle < 0) relativeAngle += 360.0f;
    if (relativeAngle >= 360.0f) relativeAngle -= 360.0f;

    return relativeAngle;
}

/**
 * @brief Callback function for PDM data.
 * Reads PDM data into the sample buffer.
 */
void EffectUtils::onPDMdata() {
    // Query the number of bytes available
    int bytesAvailable = PDM.available();

    // Read into the sample buffer
    PDM.read(sampleBuffer, bytesAvailable);

    // 16-bit, 2 bytes per sample
    samplesRead = bytesAvailable / 2;
}

/**
 * @brief Calculate the sound spectrum intensity.
 * @return The calculated sound intensity.
 */
int EffectUtils::calculateSoundSpectrum() {
    // Wait for samples to be read
    if (samplesRead) {
        DEBUG_PRINT("Samples Read: ");
        DEBUG_PRINTLN(samplesRead);

        int soundIntensity = 0;

        // Calculate the average intensity of the sound spectrum
        for (int i = 0; i < samplesRead; i++) {
            soundIntensity += abs(sampleBuffer[i]);
        }

        // Clear the read count
        samplesRead = 0;

        DEBUG_PRINT("Total Sound Intensity: ");
        DEBUG_PRINTLN(soundIntensity);

        // Map the sound intensity to a value between 1 and 10
        // Based on observed values: quiet ~50k, medium ~150k, loud ~300k+
        int mappedIntensity = mapRange(soundIntensity, 50000, 300000, 1, 10);

        DEBUG_PRINT("Mapped Sound Intensity: ");
        DEBUG_PRINTLN(mappedIntensity);

        return mappedIntensity;
    }

    DEBUG_PRINTLN("No Samples Read");
    return 0;  // Return 0 if no samples are read
}

/**
 * @brief Get the inclination angle based on accelerometer readings.
 * Uses calibration baseline if available for relative tilt calculation.
 * @return The inclination angle (0-360 degrees).
 */
float EffectUtils::getInclination() {
    float x, y, z;

    // Read accelerometer values (measures gravity for tilt detection)
    if (!IMU.readAcceleration(x, y, z)) {
        return 0.0f;  // Return 0 if read fails
    }

    float inclination;

    if (calibration.isCalibrated) {
        // Calculate tilt relative to baseline
        // The baseline captures gravity direction when "flat"
        // Subtract baseline to get relative change
        float relX = x - calibration.baselineX;
        float relY = y - calibration.baselineY;
        float relZ = z - calibration.baselineZ;

        // Find the two axes with smallest baseline values (horizontal plane)
        // and use them for tilt calculation
        float absBaseX = abs(calibration.baselineX);
        float absBaseY = abs(calibration.baselineY);
        float absBaseZ = abs(calibration.baselineZ);

        // The axis with largest baseline is gravity (vertical)
        // Use the other two for tilt angle
        if (absBaseY >= absBaseX && absBaseY >= absBaseZ) {
            // Y is vertical, use X and Z for tilt
            inclination = atan2(x, z) * RAD_TO_DEG;
        } else if (absBaseX >= absBaseY && absBaseX >= absBaseZ) {
            // X is vertical, use Y and Z for tilt
            inclination = atan2(y, z) * RAD_TO_DEG;
        } else {
            // Z is vertical, use X and Y for tilt
            inclination = atan2(x, y) * RAD_TO_DEG;
        }
    } else {
        // Fallback: assume Y is vertical (legacy behavior)
        inclination = atan2(x, z) * RAD_TO_DEG;
    }

    // Normalize to 0-360 range
    if (inclination < 0) {
        inclination += 360.0f;
    }

    return inclination;
}
