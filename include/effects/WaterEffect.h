/**
 * @project OpenHoop
 * @file WaterEffect.h
 * @brief Header file for the WaterEffect class.
 * @details Simulates water flowing to the bottom of the hoop based on gravity.
 */

#ifndef WATEREFFECT_H
#define WATEREFFECT_H

#include "Effect.h"
#include <Arduino.h>

/**
 * @brief Simulates water in a hoop that always flows to the bottom.
 * Uses accelerometer to determine "down" and lights up the bottom 1/4 of the hoop.
 */
class WaterEffect : public Effect {
public:
    WaterEffect();

    void start() override;
    void update() override;
    void stop() override;

private:
    float smoothedAngle;       ///< Smoothed angle to "bottom" for less jitter
    float wavePhase;           ///< Phase for wave animation

    // Moving average buffer for ultra-smooth angle calculation
    static constexpr int ANGLE_BUFFER_SIZE = 10;  ///< Number of samples for moving average
    float angleBuffer[ANGLE_BUFFER_SIZE];         ///< Circular buffer for angle samples
    int bufferIndex;                               ///< Current position in buffer
    bool bufferFilled;                             ///< True once buffer has been filled at least once

    // Smoothed sin/cos components for circular interpolation (avoids 0/360 discontinuity)
    float smoothedSin;         ///< Smoothed sin component of angle
    float smoothedCos;         ///< Smoothed cos component of angle

    static constexpr float WATER_FILL = 0.25f;    ///< Fraction of hoop filled with "water" (1/4)
    static constexpr float SMOOTHING_FACTOR = 0.03f;  ///< Slower smoothing for calmer water (was 0.08)
};

#endif //WATEREFFECT_H
