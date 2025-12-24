/**
 * @project OpenHoop
 * @file CompassEffect.cpp
 * @brief Implementation of the CompassEffect class.
 * @details Shows magnetic north direction on the hoop using magnetometer.
 */

#include "../../include/effects/CompassEffect.h"
#include "../../include/Config.h"
#include <Arduino_LSM9DS1.h>

CompassEffect::CompassEffect()
    : smoothedSin(0)
    , smoothedCos(1)
    , smoothedAngle(0)
    , pulsePhase(0) {}

void CompassEffect::start() {
    smoothedSin = 0;
    smoothedCos = 1;
    smoothedAngle = 0;
    pulsePhase = 0;
    DEBUG_PRINTLN("=== Compass Effect Started ===");
    DEBUG_PRINTLN("Red pointer shows magnetic north");
}

void CompassEffect::update() {
    float mx, my, mz;

    // Only read magnetometer when new data is available
    // Keep using last heading if no new data (don't flash error)
    if (IMU.magnetAvailable() && IMU.readMagnet(mx, my, mz)) {
        // Calculate heading from magnetometer (assuming hoop is roughly level)
        // atan2(my, mx) gives heading in radians
        float heading = atan2(my, mx);  // radians

        // Convert to degrees (0-360)
        float headingDeg = heading * RAD_TO_DEG;
        if (headingDeg < 0) headingDeg += 360.0f;

        // Circular interpolation using sin/cos to avoid discontinuity
        float targetRad = headingDeg * DEG_TO_RAD;
        float targetSin = sin(targetRad);
        float targetCos = cos(targetRad);

        smoothedSin += (targetSin - smoothedSin) * SMOOTHING_FACTOR;
        smoothedCos += (targetCos - smoothedCos) * SMOOTHING_FACTOR;
    }
    // If no new data, just keep using the current smoothed values

    smoothedAngle = atan2(smoothedSin, smoothedCos) * RAD_TO_DEG;
    if (smoothedAngle < 0) smoothedAngle += 360.0f;

    // Calculate which LED points north
    // Account for Arduino position offset
    float adjustedAngle = smoothedAngle - ARDUINO_LED_OFFSET_DEGREES;
    if (adjustedAngle < 0) adjustedAngle += 360.0f;
    if (adjustedAngle >= 360.0f) adjustedAngle -= 360.0f;

    int numLeds = hoop.getActivePixels();
    int northLed = static_cast<int>((adjustedAngle / LED_COVERAGE_DEGREES) * numLeds) % numLeds;
    int southLed = (northLed + numLeds / 2) % numLeds;

    // Pulse animation
    pulsePhase += 0.05f;
    if (pulsePhase > TWO_PI) pulsePhase -= TWO_PI;
    float pulse = (sin(pulsePhase) + 1.0f) * 0.5f;  // 0 to 1

    // Clear all LEDs
    hoop.fill(0);

    // Draw north pointer (red/orange gradient)
    int halfPointer = POINTER_WIDTH / 2;
    for (int i = -halfPointer; i <= halfPointer; i++) {
        int ledIndex = (northLed + i + numLeds) % numLeds;
        float dist = fabs(static_cast<float>(i)) / halfPointer;

        // Red at center, orange at edges, with pulse
        uint8_t brightness = static_cast<uint8_t>((1.0f - dist * 0.5f) * (180 + pulse * 75));
        uint8_t r = brightness;
        uint8_t g = static_cast<uint8_t>(dist * brightness * 0.3f);  // Slight orange at edges
        uint8_t b = 0;

        hoop.setPixelColor(ledIndex, r, g, b);
    }

    // Draw south tail (dim blue)
    int halfTail = TAIL_WIDTH / 2;
    for (int i = -halfTail; i <= halfTail; i++) {
        int ledIndex = (southLed + i + numLeds) % numLeds;
        float dist = fabs(static_cast<float>(i)) / halfTail;

        uint8_t brightness = static_cast<uint8_t>((1.0f - dist * 0.7f) * 60);
        hoop.setPixelColor(ledIndex, 0, 0, brightness);
    }

    // Small white dot at exact north for precision
    hoop.setPixelColor(northLed, 255, 200, 150);

    hoop.show();
}

void CompassEffect::stop() {
    hoop.fill(0);
    hoop.show();
    DEBUG_PRINTLN("=== Compass Effect Stopped ===");
}
