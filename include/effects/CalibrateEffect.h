/**
 * @project OpenHoop
 * @file CalibrateEffect.h
 * @brief IMU calibration/diagnostic effect.
 * @details Visualizes IMU data to help understand sensor orientation.
 */

#ifndef OPENHOOP_CALIBRATEEFFECT_H
#define OPENHOOP_CALIBRATEEFFECT_H

#include "Effect.h"

/**
 * @brief Diagnostic effect that visualizes IMU sensor data.
 *
 * Divides the LED strip into sections showing:
 * - Accelerometer X, Y, Z (for tilt/gravity direction)
 * - Gyroscope magnitude (for spin detection)
 *
 * This helps identify which physical axis corresponds to which sensor axis.
 */
class CalibrateEffect : public Effect {
public:
    CalibrateEffect();

    void start() override;
    void update() override;
    void stop() override;

private:
    unsigned long lastPrintTime;
};

#endif //OPENHOOP_CALIBRATEEFFECT_H
