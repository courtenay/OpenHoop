/**
 * @project OpenHoop
 * @file WaterEffect.cpp
 * @brief Implementation of the WaterEffect class.
 * @details Simulates water flowing to the bottom of the hoop based on gravity.
 */

#include "../../include/effects/WaterEffect.h"
#include "../../include/Config.h"
#include "../../include/utils/EffectUtils.h"
#include <Arduino_LSM9DS1.h>

WaterEffect::WaterEffect() : smoothedAngle(0), wavePhase(0) {}

void WaterEffect::start() {
    smoothedAngle = 0;
    wavePhase = 0;
    Serial.println("=== Water Effect Started ===");
    Serial.println("Simulating water that flows to bottom of hoop");
}

void WaterEffect::update() {
    float ax, ay, az;

    if (!IMU.accelerationAvailable() || !IMU.readAcceleration(ax, ay, az)) {
        return;
    }

    // Get calibration to determine which axis is vertical
    const auto& cal = EffectUtils::getCalibration();

    // Determine the angle around the hoop where "down" is
    // The hoop lies in a plane. We need to find where gravity points
    // relative to the Arduino's position on the hoop.
    //
    // If calibrated: use baseline to determine which plane the hoop is in
    // The "bottom" is where the gravity vector intersects the hoop circle

    float targetAngle;

    if (cal.isCalibrated) {
        // Find which axis had max baseline (vertical when calibrated)
        float absBaseX = fabs(cal.baselineX);
        float absBaseY = fabs(cal.baselineY);
        float absBaseZ = fabs(cal.baselineZ);

        if (absBaseY >= absBaseX && absBaseY >= absBaseZ) {
            // Y was vertical at calibration, hoop plane is X-Z
            targetAngle = atan2(ax, az) * RAD_TO_DEG;
        } else if (absBaseX >= absBaseY && absBaseX >= absBaseZ) {
            // X was vertical at calibration, hoop plane is Y-Z
            targetAngle = atan2(ay, az) * RAD_TO_DEG;
        } else {
            // Z was vertical at calibration, hoop plane is X-Y
            targetAngle = atan2(ax, ay) * RAD_TO_DEG;
        }
    } else {
        // No calibration - assume Y is vertical (common orientation)
        targetAngle = atan2(ax, az) * RAD_TO_DEG;
    }

    // Smooth the angle to reduce jitter
    float angleDiff = targetAngle - smoothedAngle;
    // Handle wrap-around at 180/-180
    if (angleDiff > 180) angleDiff -= 360;
    if (angleDiff < -180) angleDiff += 360;
    smoothedAngle += angleDiff * 0.15f;  // Smoothing factor

    // Normalize to 0-360
    float normalizedAngle = smoothedAngle;
    if (normalizedAngle < 0) normalizedAngle += 360;

    // Convert angle to LED position
    // Angle 0 = Arduino position, so we need to offset by 180 to find "bottom"
    // when Arduino is at top
    float bottomAngle = normalizedAngle + 180;
    if (bottomAngle >= 360) bottomAngle -= 360;

    int numLeds = hoop.getActivePixels();
    int centerLed = static_cast<int>((bottomAngle / 360.0f) * numLeds) % numLeds;

    // Water fills bottom 1/4 of the hoop
    int waterLeds = static_cast<int>(numLeds * WATER_FILL);
    int halfWater = waterLeds / 2;

    // Wave animation
    wavePhase += 0.1f;
    if (wavePhase > TWO_PI) wavePhase -= TWO_PI;

    // Clear all LEDs first
    hoop.fill(0);

    // Light up the water region
    for (int i = -halfWater; i <= halfWater; i++) {
        int ledIndex = (centerLed + i + numLeds) % numLeds;

        // Calculate distance from center for gradient
        float distFromCenter = fabs(static_cast<float>(i)) / halfWater;

        // Wave effect - slight variation in brightness
        float wave = sin(wavePhase + distFromCenter * 3.0f) * 0.15f + 0.85f;

        // Color gradient: deeper blue in center, lighter at edges
        // Center: deep blue (0, 50, 150)
        // Edge: cyan-ish (50, 150, 200)
        uint8_t r = static_cast<uint8_t>(0 + distFromCenter * 50);
        uint8_t g = static_cast<uint8_t>(50 + distFromCenter * 100);
        uint8_t b = static_cast<uint8_t>(150 + distFromCenter * 50);

        // Apply wave brightness
        r = static_cast<uint8_t>(r * wave);
        g = static_cast<uint8_t>(g * wave);
        b = static_cast<uint8_t>(b * wave);

        // Add white sparkles near surface
        uint8_t w = 0;
        if (distFromCenter > 0.7f) {
            float sparkle = sin(wavePhase * 3 + ledIndex * 0.5f);
            if (sparkle > 0.8f) {
                w = static_cast<uint8_t>((sparkle - 0.8f) * 5 * 100);
            }
        }

        hoop.setPixelColor(ledIndex, r, g, b, w);
    }

    hoop.show();
}

void WaterEffect::stop() {
    hoop.fill(0);
    hoop.show();
}
