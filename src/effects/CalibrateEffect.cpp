/**
 * @project OpenHoop
 * @file CalibrateEffect.cpp
 * @brief Four-phase IMU and LED calibration effect with verification.
 */

#include "../../include/effects/CalibrateEffect.h"
#include "../../include/Config.h"
#include "../../include/utils/EffectUtils.h"
#include "../../include/services/BleService.h"
#include <Arduino_LSM9DS1.h>

// Access the global BLE service to report calibration phase
extern BleService bleService;

CalibrateEffect::CalibrateEffect()
    : currentPhase(Phase::FLAT)
    , phaseStartTime(0)
    , lastPrintTime(0)
    , stableStartTime(0)
    , stableCycles(0)
    , smoothAx(0), smoothAy(0), smoothAz(0)
    , flatAx(0), flatAy(0), flatAz(0)
    , bottomAx(0), bottomAy(0), bottomAz(0) {}

void CalibrateEffect::start() {
    currentPhase = Phase::FLAT;
    phaseStartTime = millis();
    lastPrintTime = 0;
    stableStartTime = 0;
    stableCycles = 0;
    smoothAx = smoothAy = smoothAz = 0;

    // Report phase 1 to BLE
    bleService.calibrationCharacteristic.writeValue(1);

    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("==========================================");
    DEBUG_PRINTLN("=== FOUR-PHASE CALIBRATION STARTING ===");
    DEBUG_PRINTLN("==========================================");
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("PHASE 1/4: Place hoop FLAT on the ground");
    DEBUG_PRINTLN("          (CYAN pulsing = waiting for stable)");
    DEBUG_PRINTLN("");
}

void CalibrateEffect::resetForNextPhase() {
    stableCycles = 0;
    stableStartTime = millis();
}

float CalibrateEffect::calcCosAngle(float ax1, float ay1, float az1, float ax2, float ay2, float az2) {
    float dot = ax1 * ax2 + ay1 * ay2 + az1 * az2;
    float mag1 = sqrt(ax1 * ax1 + ay1 * ay1 + az1 * az1);
    float mag2 = sqrt(ax2 * ax2 + ay2 * ay2 + az2 * az2);
    if (mag1 < 0.1f || mag2 < 0.1f) return 1.0f;
    return dot / (mag1 * mag2);
}

void CalibrateEffect::update() {
    float ax, ay, az;

    if (!IMU.readAcceleration(ax, ay, az)) {
        hoop.fill(HulaHoopDotStar::Color(255, 0, 0));
        hoop.show();
        return;
    }

    // Smooth the accelerometer readings
    smoothAx = smoothAx * (1.0f - SMOOTHING) + ax * SMOOTHING;
    smoothAy = smoothAy * (1.0f - SMOOTHING) + ay * SMOOTHING;
    smoothAz = smoothAz * (1.0f - SMOOTHING) + az * SMOOTHING;

    unsigned long now = millis();
    float pulse = (sin(now * 0.005f) + 1.0f) * 0.5f;
    uint8_t brightness = static_cast<uint8_t>(50 + pulse * 150);

    // Check stability (smoothed values not changing much)
    static float prevSmoothAx = 0, prevSmoothAy = 0, prevSmoothAz = 0;
    float delta = fabs(smoothAx - prevSmoothAx) + fabs(smoothAy - prevSmoothAy) + fabs(smoothAz - prevSmoothAz);
    prevSmoothAx = smoothAx; prevSmoothAy = smoothAy; prevSmoothAz = smoothAz;

    bool isCurrentlyStable = delta < STABILITY_THRESHOLD;
    if (isCurrentlyStable) {
        stableCycles++;
    } else {
        stableCycles = 0;
    }

    bool stableEnough = stableCycles >= STABLE_CYCLES_REQUIRED;

    // Print status periodically
    if (now - lastPrintTime > 300) {
        lastPrintTime = now;
        DEBUG_PRINT("Accel: X=");
        DEBUG_PRINT(smoothAx, 2);
        DEBUG_PRINT(" Y=");
        DEBUG_PRINT(smoothAy, 2);
        DEBUG_PRINT(" Z=");
        DEBUG_PRINT(smoothAz, 2);
        DEBUG_PRINT(" | Stable cycles: ");
        DEBUG_PRINT(stableCycles);
        DEBUG_PRINT("/");
        DEBUG_PRINTLN(STABLE_CYCLES_REQUIRED);
    }

    switch (currentPhase) {
        case Phase::FLAT: {
            hoop.fill(HulaHoopDotStar::Color(0, brightness, brightness));  // CYAN

            if (stableEnough) {
                flatAx = smoothAx;
                flatAy = smoothAy;
                flatAz = smoothAz;

                EffectUtils::calibrateIMU();

                hoop.fill(HulaHoopDotStar::Color(0, 255, 0));
                hoop.show();
                delay(500);

                currentPhase = Phase::WAIT_MOVE;
                resetForNextPhase();
                bleService.calibrationCharacteristic.writeValue(2);

                DEBUG_PRINTLN("");
                DEBUG_PRINTLN("PHASE 1 COMPLETE - Flat orientation captured");
                DEBUG_PRINTLN("");
                DEBUG_PRINTLN("PHASE 2/4: Now PICK UP the hoop");
                DEBUG_PRINTLN("          (YELLOW = waiting for movement)");
                DEBUG_PRINTLN("");
            }
            break;
        }

        case Phase::WAIT_MOVE: {
            // Wait for the hoop to be moved (orientation changed from flat)
            float cosAngle = calcCosAngle(smoothAx, smoothAy, smoothAz, flatAx, flatAy, flatAz);
            bool hasMoved = cosAngle < 0.7f;  // At least ~45° change

            if (hasMoved) {
                hoop.fill(HulaHoopDotStar::Color(brightness, brightness / 2, 0));  // YELLOW
            } else {
                hoop.fill(HulaHoopDotStar::Color(brightness / 2, brightness / 4, 0));  // DIM YELLOW
            }

            if (now - lastPrintTime > 300) {
                float angle = acos(constrain(cosAngle, -1.0f, 1.0f)) * 180.0f / 3.14159f;
                DEBUG_PRINT("  Movement angle: ");
                DEBUG_PRINT(angle, 0);
                DEBUG_PRINTLN(hasMoved ? "° - MOVED!" : "° (need >45°)");
            }

            if (hasMoved) {
                currentPhase = Phase::BOTTOM;
                resetForNextPhase();
                bleService.calibrationCharacteristic.writeValue(3);

                DEBUG_PRINTLN("");
                DEBUG_PRINTLN("PHASE 2 COMPLETE - Movement detected");
                DEBUG_PRINTLN("");
                DEBUG_PRINTLN("PHASE 3/4: Hold VERTICAL, Arduino at BOTTOM");
                DEBUG_PRINTLN("          (MAGENTA = waiting for vertical + stable)");
                DEBUG_PRINTLN("");
            }
            break;
        }

        case Phase::BOTTOM: {
            float cosAngle = calcCosAngle(smoothAx, smoothAy, smoothAz, flatAx, flatAy, flatAz);
            bool isTilted = cosAngle < TILT_COS_THRESHOLD;  // At least ~66° from flat

            if (isTilted && stableEnough) {
                hoop.fill(HulaHoopDotStar::Color(brightness, 0, brightness));  // MAGENTA bright
            } else if (isTilted) {
                hoop.fill(HulaHoopDotStar::Color(brightness / 2, 0, brightness / 2));  // MAGENTA dim
            } else {
                hoop.fill(HulaHoopDotStar::Color(brightness, brightness / 2, 0));  // YELLOW
            }

            if (now - lastPrintTime > 300) {
                float angle = acos(constrain(cosAngle, -1.0f, 1.0f)) * 180.0f / 3.14159f;
                DEBUG_PRINT("  Tilt: ");
                DEBUG_PRINT(angle, 0);
                DEBUG_PRINT("° | Tilted: ");
                DEBUG_PRINT(isTilted ? "YES" : "no");
                DEBUG_PRINT(" | Stable: ");
                DEBUG_PRINTLN(stableEnough ? "YES" : "no");
            }

            if (stableEnough && isTilted) {
                bottomAx = smoothAx;
                bottomAy = smoothAy;
                bottomAz = smoothAz;

                EffectUtils::calibrateLEDOffset();

                hoop.fill(HulaHoopDotStar::Color(0, 255, 0));
                hoop.show();
                delay(500);

                currentPhase = Phase::TOP_VERIFY;
                resetForNextPhase();
                bleService.calibrationCharacteristic.writeValue(4);

                DEBUG_PRINTLN("");
                DEBUG_PRINTLN("PHASE 3 COMPLETE - Bottom position captured");
                DEBUG_PRINTLN("");
                DEBUG_PRINTLN("PHASE 4/4: Now flip - Arduino at TOP");
                DEBUG_PRINTLN("          (BLUE = waiting for opposite position)");
                DEBUG_PRINTLN("");
            }
            break;
        }

        case Phase::TOP_VERIFY: {
            // Verify that Arduino is now at top (opposite of bottom)
            float cosFromBottom = calcCosAngle(smoothAx, smoothAy, smoothAz, bottomAx, bottomAy, bottomAz);
            float cosFromFlat = calcCosAngle(smoothAx, smoothAy, smoothAz, flatAx, flatAy, flatAz);

            // Should be ~180° from bottom (cos ≈ -1) and still tilted from flat
            bool isOpposite = cosFromBottom < -0.5f;  // At least ~120° from bottom position
            bool stillTilted = cosFromFlat < TILT_COS_THRESHOLD;

            if (isOpposite && stillTilted && stableEnough) {
                hoop.fill(HulaHoopDotStar::Color(0, 0, brightness));  // BLUE bright
            } else if (isOpposite && stillTilted) {
                hoop.fill(HulaHoopDotStar::Color(0, 0, brightness / 2));  // BLUE dim
            } else {
                hoop.fill(HulaHoopDotStar::Color(brightness / 2, 0, brightness));  // PURPLE
            }

            if (now - lastPrintTime > 300) {
                float angleFromBottom = acos(constrain(cosFromBottom, -1.0f, 1.0f)) * 180.0f / 3.14159f;
                DEBUG_PRINT("  Angle from bottom: ");
                DEBUG_PRINT(angleFromBottom, 0);
                DEBUG_PRINT("° | Opposite: ");
                DEBUG_PRINT(isOpposite ? "YES" : "no");
                DEBUG_PRINT(" | Stable: ");
                DEBUG_PRINTLN(stableEnough ? "YES" : "no");
            }

            if (stableEnough && isOpposite && stillTilted) {
                currentPhase = Phase::DONE;
                bleService.calibrationCharacteristic.writeValue(5);

                DEBUG_PRINTLN("");
                DEBUG_PRINTLN("==========================================");
                DEBUG_PRINTLN("=== CALIBRATION COMPLETE & VERIFIED! ===");
                DEBUG_PRINTLN("==========================================");
                DEBUG_PRINTLN("");
                DEBUG_PRINTLN("Water effect should now align correctly.");
                DEBUG_PRINTLN("");
            }
            break;
        }

        case Phase::DONE: {
            hoop.fill(HulaHoopDotStar::Color(0, 150, 0));  // Solid GREEN
            break;
        }
    }

    hoop.show();
}

void CalibrateEffect::stop() {
    hoop.fill(0);
    hoop.show();
    bleService.calibrationCharacteristic.writeValue(0);  // Not calibrating
    DEBUG_PRINTLN("=== Calibration Mode Ended ===");
}
