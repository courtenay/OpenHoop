/**
 * @project OpenHoop
 * @file POVEffect.h
 * @brief Header file for the POVEffect class.
 * @details Persistence of Vision effect that displays images while spinning.
 */

#ifndef OPENHOOP_POVEFFECT_H
#define OPENHOOP_POVEFFECT_H

#include "Effect.h"

/**
 * @brief Persistence of Vision effect that displays images when hoop spins.
 *
 * Uses procedural patterns (no RAM buffer) to minimize memory usage.
 * Patterns are computed on-the-fly based on rotation angle.
 */
class POVEffect : public Effect {
public:
    POVEffect();

    void start() override;
    void update() override;
    void stop() override;

private:
    float currentAngle;          // Current rotation angle (0-360)
    unsigned long lastUpdateTime;

    // Demo mode for testing without spinning
    bool demoMode;
    float demoSpeed;

    // Pattern selection
    int currentPattern;
    static const int NUM_PATTERNS = 3;

    /**
     * @brief Update rotation tracking using gyroscope or demo mode.
     */
    void updateRotation();

    /**
     * @brief Compute and display pattern based on current angle.
     */
    void displayPattern();

    /**
     * @brief Heart pattern - shows a heart when spinning.
     */
    void drawHeart(int column);

    /**
     * @brief Spiral pattern - colorful spiral.
     */
    void drawSpiral(int column);

    /**
     * @brief Text pattern - displays "HI".
     */
    void drawText(int column);

    /**
     * @brief Check if a point is inside the heart shape.
     */
    bool isInsideHeart(float x, float y);
};

#endif //OPENHOOP_POVEFFECT_H
