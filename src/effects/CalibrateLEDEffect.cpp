/**
 * @project OpenHoop
 * @file CalibrateLEDEffect.cpp
 * @brief Calibrates LED offset angle for proper water/gravity effects.
 */

#include "../../include/effects/CalibrateLEDEffect.h"
#include "../../include/Config.h"
#include "../../include/utils/EffectUtils.h"

void CalibrateLEDEffect::start() {
    calibrated = false;
    startTime = millis();

    Serial.println("=== LED Offset Calibration ===");
    Serial.println("Hold hoop VERTICAL with Arduino at BOTTOM!");
    Serial.println("Calibrating in 2 seconds...");

    // Show a visual indicator - red pulsing
    hoop.fill(HulaHoopDotStar::Color(50, 0, 0));
    hoop.show();
}

void CalibrateLEDEffect::update() {
    unsigned long elapsed = millis() - startTime;

    if (!calibrated && elapsed > 2000) {
        // Perform LED offset calibration
        EffectUtils::calibrateLEDOffset();
        calibrated = true;

        // Show success - green
        hoop.fill(HulaHoopDotStar::Color(0, 100, 0));
        hoop.show();

        Serial.println("LED offset calibration complete!");
        Serial.println("Water effect should now align correctly.");
    }

    if (!calibrated) {
        // Pulsing red while waiting
        float pulse = (sin(elapsed * 0.005f) + 1.0f) * 0.5f;
        uint8_t brightness = static_cast<uint8_t>(20 + pulse * 80);
        hoop.fill(HulaHoopDotStar::Color(brightness, 0, 0));
        hoop.show();
    }
}

void CalibrateLEDEffect::stop() {
    hoop.fill(0);
    hoop.show();
}
