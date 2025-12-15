/**
 * @project OpenHoop
 * @file PulseEffect.cpp
 * @brief Implementation file for the PulseEffect class.
 * @details Defines the methods for the PulseEffect class, responsible for creating a pulsing color effect on the LED display based on gyroscope inclination.
 * @author github.com/angelcamelot
 * @date 2024-03-17
 * @license Open-source license.
 */

#include "../../include/effects/PulseEffect.h"
#include "../../include/Config.h"
#include "../../include/utils/EffectUtils.h"

/**
 * @brief Constructor for PulseEffect.
 */
PulseEffect::PulseEffect() : pulseSpeed(3), pulseBrightness(128), pulseIncrement(5), currentColorIndex(0), pulsePhase(0), pulseDirection(true) {
    colors[0] = HulaHoopDotStar::Color(34, 87, 122);   // Deep blue reminiscent of tranquil ocean waves
    colors[1] = HulaHoopDotStar::Color(56, 163, 165);  // Subtle greenish-blue representing serene depths
    colors[2] = HulaHoopDotStar::Color(87, 204, 153);  // Vibrant emerald portraying energy and vitality
    colors[3] = HulaHoopDotStar::Color(128, 237, 153); // Fresh light green symbolizing renewal and growth
    colors[4] = HulaHoopDotStar::Color(199, 249, 204); // Soft tea green evoking a sense of calm and balance
}

/**
 * @brief Initializes the Effect.
 */
void PulseEffect::start() {
    pulsePhase = 0;
    pulseBrightness = 128;
    pulseDirection = true;
}

/**
 * @brief Updates the Effect.
 */
void PulseEffect::update() {
    // Auto-pulsing animation: brightness oscillates between 30 and 255
    // Using sine wave for smooth pulsing
    pulsePhase += pulseSpeed;
    if (pulsePhase >= 360.0f) {
        pulsePhase -= 360.0f;
    }

    // Sine wave gives smooth pulse: range 30-255 (never fully off)
    float sineValue = sin(radians(pulsePhase));
    pulseBrightness = static_cast<int>(30 + (sineValue + 1.0f) * 112.5f);  // 30 to 255

    // Get inclination to influence color selection (optional enhancement)
    float inclination = EffectUtils::getInclination();

    // Map inclination to color palette index (0-4)
    // This makes tilting change the color
    currentColorIndex = static_cast<int>(inclination / 72.0f) % 5;

    // Apply the pulse to all LEDs
    for (int i = 0; i < hoop.getActivePixels(); i++) {
        // Interpolate between current and next color for smooth gradient
        uint32_t color = EffectUtils::interpolateColor(
            colors[currentColorIndex],
            colors[(currentColorIndex + 1) % 5],
            i,
            hoop.getActivePixels()
        );
        color = EffectUtils::applyBrightness(color, pulseBrightness);

        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        hoop.setPixelColor(i, r, g, b);
    }

    hoop.show();
}

/**
 * @brief Stops the Effect.
 * Turns off all LEDs on the display.
 */
void PulseEffect::stop() {
    hoop.fill(HulaHoopDotStar::Color(0, 0, 0));
}
