/**
 * @project OpenHoop
 * @file CompassEffect.h
 * @brief Header file for the CompassEffect class.
 * @details Shows magnetic north direction on the hoop using magnetometer.
 */

#ifndef COMPASSEFFECT_H
#define COMPASSEFFECT_H

#include "Effect.h"
#include <Arduino.h>

/**
 * @brief Compass effect that points toward magnetic north.
 * Uses magnetometer to determine heading and lights up LEDs pointing north.
 */
class CompassEffect : public Effect {
public:
    CompassEffect();

    void start() override;
    void update() override;
    void stop() override;

private:
    // Smoothed sin/cos components for circular interpolation (avoids discontinuity)
    float smoothedSin;
    float smoothedCos;
    float smoothedAngle;

    // Pulse animation
    float pulsePhase;

    static constexpr float SMOOTHING_FACTOR = 0.08f;  ///< Smoothing for heading
    static constexpr int POINTER_WIDTH = 12;          ///< LEDs for the north pointer
    static constexpr int TAIL_WIDTH = 6;              ///< LEDs for the south tail
};

#endif //COMPASSEFFECT_H
