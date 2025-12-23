/**
 * @project OpenHoop
 * @file CalibrateLEDEffect.h
 * @brief Calibrates LED offset angle for proper water/gravity effects.
 */

#ifndef CALIBRATELEDEFFECT_H
#define CALIBRATELEDEFFECT_H

#include "Effect.h"

/**
 * @brief Calibrates the LED offset angle.
 * Hold the hoop VERTICAL with the Arduino/controller at the BOTTOM.
 * This captures the reference angle for mapping gravity to LED positions.
 */
class CalibrateLEDEffect : public Effect {
public:
    void start() override;
    void update() override;
    void stop() override;

private:
    bool calibrated;
    unsigned long startTime;
};

#endif //CALIBRATELEDEFFECT_H
