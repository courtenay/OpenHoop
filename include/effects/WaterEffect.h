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
    static constexpr float WATER_FILL = 0.25f;  ///< Fraction of hoop filled with "water" (1/4)
};

#endif //WATEREFFECT_H
