/**
 * @project OpenHoop
 * @file CalibrateEffect.cpp
 * @brief Two-phase IMU and LED calibration effect.
 */

#include "../../include/effects/CalibrateEffect.h"
#include "../../include/Config.h"
#include "../../include/utils/EffectUtils.h"
#include <Arduino_LSM9DS1.h>

CalibrateEffect::CalibrateEffect()
    : currentPhase(Phase::FLAT)
    , phaseStartTime(0)
    , lastPrintTime(0)
    , stableStartTime(0)
    , isStable(false) {}

void CalibrateEffect::start() {
    currentPhase = Phase::FLAT;
    phaseStartTime = millis();
    lastPrintTime = 0;
    stableStartTime = 0;
    isStable = false;

    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTLN("=== TWO-PHASE CALIBRATION STARTING ===");
    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("PHASE 1: Place hoop FLAT on the ground");
    DEBUG_PRINTLN("         (CYAN pulsing = waiting)");
    DEBUG_PRINTLN("");
}

void CalibrateEffect::update() {
    float ax, ay, az;
    static float lastAx = 0, lastAy = 0, lastAz = 0;

    if (!IMU.readAcceleration(ax, ay, az)) {
        hoop.fill(HulaHoopDotStar::Color(255, 0, 0));
        hoop.show();
        return;
    }

    unsigned long now = millis();
    float pulse = (sin(now * 0.005f) + 1.0f) * 0.5f;
    uint8_t brightness = static_cast<uint8_t>(50 + pulse * 150);

    // Check stability (acceleration not changing much)
    float delta = fabs(ax - lastAx) + fabs(ay - lastAy) + fabs(az - lastAz);
    lastAx = ax; lastAy = ay; lastAz = az;

    if (delta < STABILITY_THRESHOLD) {
        if (!isStable) {
            isStable = true;
            stableStartTime = now;
        }
    } else {
        isStable = false;
        stableStartTime = 0;
    }

    bool stableEnough = isStable && (now - stableStartTime > STABLE_DURATION_MS);

    // Print status periodically
    if (now - lastPrintTime > 500) {
        lastPrintTime = now;
        DEBUG_PRINT("Accel: X=");
        DEBUG_PRINT(ax);
        DEBUG_PRINT(" Y=");
        DEBUG_PRINT(ay);
        DEBUG_PRINT(" Z=");
        DEBUG_PRINT(az);
        DEBUG_PRINT(" | Stable: ");
        DEBUG_PRINTLN(stableEnough ? "YES" : "no");
    }

    switch (currentPhase) {
        case Phase::FLAT: {
            // Show pulsing CYAN
            hoop.fill(HulaHoopDotStar::Color(0, brightness, brightness));

            // Check if stable (we don't really need it to be perfectly flat)
            // Just need it to be still so we can capture a baseline
            bool isFlat = stableEnough;  // Any stable position works for baseline

            if (isFlat) {
                // Capture flat baseline
                EffectUtils::calibrateIMU();

                // Flash green briefly
                hoop.fill(HulaHoopDotStar::Color(0, 255, 0));
                hoop.show();
                delay(500);

                // Move to phase 2
                currentPhase = Phase::LED_OFFSET;
                phaseStartTime = now;
                isStable = false;
                stableStartTime = 0;

                DEBUG_PRINTLN("");
                DEBUG_PRINTLN("========================================");
                DEBUG_PRINTLN("PHASE 1 COMPLETE!");
                DEBUG_PRINTLN("========================================");
                DEBUG_PRINTLN("");
                DEBUG_PRINTLN("PHASE 2: Hold hoop VERTICAL");
                DEBUG_PRINTLN("         Arduino should be at the BOTTOM");
                DEBUG_PRINTLN("         (MAGENTA pulsing = waiting)");
                DEBUG_PRINTLN("");
            }
            break;
        }

        case Phase::LED_OFFSET: {
            // Show pulsing MAGENTA
            hoop.fill(HulaHoopDotStar::Color(brightness, 0, brightness));

            // Just need stability - user positions Arduino at bottom
            if (stableEnough) {
                // Capture LED offset
                EffectUtils::calibrateLEDOffset();

                // Move to done
                currentPhase = Phase::DONE;
                phaseStartTime = now;

                DEBUG_PRINTLN("");
                DEBUG_PRINTLN("========================================");
                DEBUG_PRINTLN("CALIBRATION COMPLETE!");
                DEBUG_PRINTLN("========================================");
                DEBUG_PRINTLN("");
                DEBUG_PRINTLN("Water effect should now align correctly.");
                DEBUG_PRINTLN("You can now select another effect.");
                DEBUG_PRINTLN("");
            }
            break;
        }

        case Phase::DONE: {
            // Solid green - all done!
            hoop.fill(HulaHoopDotStar::Color(0, 150, 0));
            break;
        }
    }

    hoop.show();
}

void CalibrateEffect::stop() {
    hoop.fill(0);
    hoop.show();
    DEBUG_PRINTLN("=== Calibration Mode Ended ===");
}
