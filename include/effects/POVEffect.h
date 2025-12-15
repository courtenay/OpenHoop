/**
 * @project OpenHoop
 * @file POVEffect.h
 * @brief Header file for the POVEffect class.
 * @details Persistence of Vision effect that displays images while spinning.
 * @author OpenHoop Contributors
 * @date 2024-12-15
 * @license Open-source license.
 */

#ifndef OPENHOOP_POVEFFECT_H
#define OPENHOOP_POVEFFECT_H

#include "Effect.h"
#include "../images/LedImage.h"

/**
 * @brief Persistence of Vision effect that displays images when hoop spins.
 *
 * POV works by displaying different columns of an image based on rotation angle.
 * When spinning fast enough, the eye perceives the full image.
 */
class POVEffect : public Effect {
public:
    POVEffect();
    ~POVEffect();

    void start() override;
    void update() override;
    void stop() override;

private:
    // Image data: stored as columns (numColumns x numLEDs)
    // Each column represents what to display at that rotation angle
    static const int NUM_COLUMNS = 72;  // 72 columns = 5 degrees per column
    static const int NUM_ROWS = 288;    // Match LED count

    LedColor* imageData;  // Column-major: imageData[col * NUM_ROWS + row]

    float currentAngle;      // Current rotation angle (0-360)
    float angularVelocity;   // Degrees per second from gyroscope
    unsigned long lastUpdateTime;

    // Demo mode for testing without spinning
    bool demoMode;
    float demoSpeed;  // Degrees per frame in demo mode

    /**
     * @brief Initialize a simple test pattern (heart shape).
     */
    void initTestPattern();

    /**
     * @brief Initialize text pattern.
     * @param text Simple text to display (uppercase letters only).
     */
    void initTextPattern(const char* text);

    /**
     * @brief Set a pixel in the POV image buffer.
     * @param column Column index (0 to NUM_COLUMNS-1).
     * @param row Row index (0 to NUM_ROWS-1, maps to LED index).
     * @param color Color to set.
     */
    void setImagePixel(int column, int row, const LedColor& color);

    /**
     * @brief Get a pixel from the POV image buffer.
     */
    LedColor getImagePixel(int column, int row) const;

    /**
     * @brief Read gyroscope and update angular position.
     */
    void updateRotation();

    /**
     * @brief Display the current column based on rotation angle.
     */
    void displayCurrentColumn();
};

#endif //OPENHOOP_POVEFFECT_H
