/**
 * @project OpenHoop
 * @file WaterEffect.cpp
 * @brief Implementation of the WaterEffect class.
 * @details Simulates water flowing to the bottom of the hoop based on gravity.
 */

#include "../../include/effects/WaterEffect.h"
#include "../../include/Config.h"
#include "../../include/utils/EffectUtils.h"

WaterEffect::WaterEffect() : smoothedAngle(0), wavePhase(0) {}

void WaterEffect::start() {
    smoothedAngle = 0;
    wavePhase = 0;
    DEBUG_PRINTLN("=== Water Effect Started ===");
    DEBUG_PRINTLN("Simulating water that flows to bottom of hoop");
}

void WaterEffect::update() {
    // Get the angle to the bottom of the hoop (handles calibration offset)
    float targetAngle = EffectUtils::getBottomAngle();

    // Smooth the angle to reduce jitter
    float angleDiff = targetAngle - smoothedAngle;
    // Handle wrap-around at 180/-180
    if (angleDiff > 180) angleDiff -= 360;
    if (angleDiff < -180) angleDiff += 360;
    smoothedAngle += angleDiff * 0.05f;  // Slower smoothing for calmer motion

    // Normalize to 0-360
    float normalizedAngle = smoothedAngle;
    if (normalizedAngle < 0) normalizedAngle += 360;
    if (normalizedAngle >= 360) normalizedAngle -= 360;

    // Apply Arduino-to-LED offset and scale for LED coverage
    float adjustedAngle = normalizedAngle - ARDUINO_LED_OFFSET_DEGREES;
    if (adjustedAngle < 0) adjustedAngle += 360;
    if (adjustedAngle >= 360) adjustedAngle -= 360;

    int numLeds = hoop.getActivePixels();
    int centerLed = static_cast<int>((adjustedAngle / LED_COVERAGE_DEGREES) * numLeds) % numLeds;

    // Water fills bottom 1/4 of the hoop
    int waterLeds = static_cast<int>(numLeds * WATER_FILL);
    int halfWater = waterLeds / 2;

    // Wave animation - gentle and slow
    wavePhase += 0.03f;
    if (wavePhase > TWO_PI) wavePhase -= TWO_PI;

    // Clear all LEDs first
    hoop.fill(0);

    // Light up the water region
    for (int i = -halfWater; i <= halfWater; i++) {
        int ledIndex = (centerLed + i + numLeds) % numLeds;

        // Calculate distance from center for gradient
        float distFromCenter = fabs(static_cast<float>(i)) / halfWater;

        // Wave effect - subtle variation in brightness
        float wave = sin(wavePhase + distFromCenter * 2.0f) * 0.08f + 0.92f;

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

        // Subtle highlight near edges (no sparkles, just gentle glow)
        if (distFromCenter > 0.8f) {
            float edgeGlow = (distFromCenter - 0.8f) * 5.0f;  // 0 to 1 at edge
            r = min(255, static_cast<int>(r + edgeGlow * 30));
            g = min(255, static_cast<int>(g + edgeGlow * 40));
            b = min(255, static_cast<int>(b + edgeGlow * 20));
        }

        hoop.setPixelColor(ledIndex, r, g, b);
    }

    hoop.show();
}

void WaterEffect::stop() {
    hoop.fill(0);
    hoop.show();
}
