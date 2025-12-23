/**
 * @project OpenHoop
 * @file CalibrateEffect.h
 * @brief Two-phase IMU and LED calibration effect.
 */

#ifndef OPENHOOP_CALIBRATEEFFECT_H
#define OPENHOOP_CALIBRATEEFFECT_H

#include "Effect.h"

/**
 * @brief Two-phase calibration effect.
 *
 * Phase 1 (CYAN pulsing): Place hoop FLAT on ground
 *   - Captures which axis is gravity (vertical)
 *   - Auto-detects when stable and flat
 *
 * Phase 2 (MAGENTA pulsing): Hold hoop VERTICAL with Arduino at BOTTOM
 *   - Captures LED offset angle for water/gravity effects
 *   - Auto-detects when tilted and stable
 *
 * GREEN flash = phase complete, SOLID GREEN = all done
 */
class CalibrateEffect : public Effect {
public:
    CalibrateEffect();

    void start() override;
    void update() override;
    void stop() override;

private:
    enum class Phase { FLAT, LED_OFFSET, DONE };
    Phase currentPhase;
    unsigned long phaseStartTime;
    unsigned long lastPrintTime;
    unsigned long stableStartTime;
    bool isStable;

    static constexpr float STABILITY_THRESHOLD = 0.05f;  // Max allowed acceleration change
    static constexpr unsigned long STABLE_DURATION_MS = 1000;  // Must be stable for 1 second
    static constexpr float TILT_THRESHOLD = 0.7f;  // Min tilt to detect vertical position
};

#endif //OPENHOOP_CALIBRATEEFFECT_H
