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
    enum class Phase { FLAT, WAIT_MOVE, BOTTOM, TOP_VERIFY, DONE };
    Phase currentPhase;
    unsigned long phaseStartTime;
    unsigned long lastPrintTime;
    unsigned long stableStartTime;
    int stableCycles;  // Count of consecutive stable readings

    // Smoothed accelerometer values
    float smoothAx, smoothAy, smoothAz;

    // Captured orientations for comparison
    float flatAx, flatAy, flatAz;
    float bottomAx, bottomAy, bottomAz;

    static constexpr float STABILITY_THRESHOLD = 0.05f;  // Max allowed acceleration change per reading
    static constexpr int STABLE_CYCLES_REQUIRED = 30;    // Need 30 stable readings (~1s at 30fps)
    static constexpr int FLAT_STABLE_CYCLES = 60;        // Need 60 stable readings (~2s) for FLAT phase
    static constexpr unsigned long FLAT_GRACE_PERIOD_MS = 3000;  // 3 second grace period to lay hoop down
    static constexpr float SMOOTHING = 0.3f;             // Smoothing factor for accel
    static constexpr float TILT_COS_THRESHOLD = 0.4f;    // cos(66°) - must tilt at least 66°

    float calcCosAngle(float ax1, float ay1, float az1, float ax2, float ay2, float az2);
    void resetForNextPhase();
};

#endif //OPENHOOP_CALIBRATEEFFECT_H
