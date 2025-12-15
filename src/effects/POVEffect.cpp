/**
 * @project OpenHoop
 * @file POVEffect.cpp
 * @brief Implementation of the POVEffect class.
 * @details Persistence of Vision effect using gyroscope for rotation tracking.
 * @author OpenHoop Contributors
 * @date 2024-12-15
 * @license Open-source license.
 */

#include "../../include/effects/POVEffect.h"
#include "../../include/Config.h"
#include "Arduino_BMI270_BMM150.h"

/**
 * @brief Constructor for POVEffect.
 */
POVEffect::POVEffect()
    : imageData(nullptr),
      currentAngle(0.0f),
      angularVelocity(0.0f),
      lastUpdateTime(0),
      demoMode(true),      // Start in demo mode for testing
      demoSpeed(3.0f) {    // 3 degrees per frame = ~120 frames per revolution
}

/**
 * @brief Destructor.
 */
POVEffect::~POVEffect() {
    if (imageData) {
        delete[] imageData;
        imageData = nullptr;
    }
}

/**
 * @brief Initialize the effect.
 */
void POVEffect::start() {
    // Allocate image buffer
    imageData = new LedColor[NUM_COLUMNS * NUM_ROWS];

    // Clear to black
    for (int i = 0; i < NUM_COLUMNS * NUM_ROWS; i++) {
        imageData[i] = LedColor(0, 0, 0, 0);
    }

    // Initialize with a test pattern
    initTestPattern();
    // Or use text: initTextPattern("HI");

    currentAngle = 0.0f;
    lastUpdateTime = millis();
}

/**
 * @brief Update the effect each frame.
 */
void POVEffect::update() {
    updateRotation();
    displayCurrentColumn();
}

/**
 * @brief Stop the effect.
 */
void POVEffect::stop() {
    if (imageData) {
        delete[] imageData;
        imageData = nullptr;
    }
    hoop.fill(HulaHoopDotStar::Color(0, 0, 0));
}

/**
 * @brief Set a pixel in the image buffer.
 */
void POVEffect::setImagePixel(int column, int row, const LedColor& color) {
    if (column >= 0 && column < NUM_COLUMNS && row >= 0 && row < NUM_ROWS) {
        imageData[column * NUM_ROWS + row] = color;
    }
}

/**
 * @brief Get a pixel from the image buffer.
 */
LedColor POVEffect::getImagePixel(int column, int row) const {
    if (column >= 0 && column < NUM_COLUMNS && row >= 0 && row < NUM_ROWS) {
        return imageData[column * NUM_ROWS + row];
    }
    return LedColor(0, 0, 0, 0);
}

/**
 * @brief Update rotation tracking using gyroscope or demo mode.
 */
void POVEffect::updateRotation() {
    unsigned long now = millis();
    float dt = (now - lastUpdateTime) / 1000.0f;  // Delta time in seconds
    lastUpdateTime = now;

    if (demoMode) {
        // Demo mode: auto-advance for testing without spinning
        currentAngle += demoSpeed;
    } else {
        // Read gyroscope Z-axis (rotation around hoop's axis)
        float gx, gy, gz;
        if (IMU.readGyroscope(gx, gy, gz)) {
            // gz is in degrees/second around Z-axis
            angularVelocity = gz;
            currentAngle += angularVelocity * dt;
        }
    }

    // Normalize angle to 0-360
    while (currentAngle >= 360.0f) currentAngle -= 360.0f;
    while (currentAngle < 0.0f) currentAngle += 360.0f;
}

/**
 * @brief Display the column corresponding to current rotation angle.
 */
void POVEffect::displayCurrentColumn() {
    // Map angle (0-360) to column index (0 to NUM_COLUMNS-1)
    int column = static_cast<int>((currentAngle / 360.0f) * NUM_COLUMNS) % NUM_COLUMNS;

    // Display this column on the LED strip
    for (int row = 0; row < hoop.getActivePixels() && row < NUM_ROWS; row++) {
        LedColor pixel = getImagePixel(column, row);

        if (pixel.alpha > 0) {
            hoop.setPixelColor(row, pixel.red, pixel.green, pixel.blue);
        } else {
            hoop.setPixelColor(row, 0, 0, 0);
        }
    }

    hoop.show();
}

/**
 * @brief Initialize a heart pattern for testing.
 * The heart spans multiple columns so it appears when spinning.
 */
void POVEffect::initTestPattern() {
    // Create a simple heart pattern
    // Heart is about 24 columns wide (120 degrees of the rotation)
    // and spans the middle section of LEDs

    const int heartWidth = 24;
    const int heartHeight = 40;
    const int startCol = (NUM_COLUMNS - heartWidth) / 2;
    const int startRow = (NUM_ROWS - heartHeight) / 2;

    // Heart shape defined as a bitmap (1 = filled)
    // Each row of this array is one vertical slice of the heart
    const uint8_t heart[24][40] = {
        // Columns 0-23, each has 40 rows
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    };

    // Red/pink color for heart
    LedColor heartColor(255, 20, 60, 255);

    for (int col = 0; col < heartWidth; col++) {
        for (int row = 0; row < heartHeight; row++) {
            if (heart[col][row]) {
                setImagePixel(startCol + col, startRow + row, heartColor);
            }
        }
    }
}

/**
 * @brief Initialize a simple text pattern.
 * Basic 5x7 pixel font for uppercase letters.
 */
void POVEffect::initTextPattern(const char* text) {
    // Simple 5-wide font definitions for basic letters
    // Each letter is 5 columns x 7 rows
    static const uint8_t fontH[5] = {0x7F, 0x08, 0x08, 0x08, 0x7F};  // H
    static const uint8_t fontI[5] = {0x00, 0x41, 0x7F, 0x41, 0x00};  // I
    static const uint8_t fontO[5] = {0x3E, 0x41, 0x41, 0x41, 0x3E};  // O
    static const uint8_t fontP[5] = {0x7F, 0x09, 0x09, 0x09, 0x06};  // P
    static const uint8_t fontV[5] = {0x1F, 0x20, 0x40, 0x20, 0x1F};  // V

    const uint8_t* fonts[] = {fontH, fontI, fontO, fontP, fontV};
    const char chars[] = "HIOPV";

    LedColor textColor(0, 255, 100, 255);  // Green text
    int startCol = 10;
    int startRow = (NUM_ROWS / 2) - 20;  // Center vertically
    int scale = 6;  // Scale up the font

    for (int i = 0; text[i] != '\0'; i++) {
        const uint8_t* fontData = nullptr;

        // Find the font for this character
        for (int j = 0; chars[j] != '\0'; j++) {
            if (text[i] == chars[j]) {
                fontData = fonts[j];
                break;
            }
        }

        if (fontData) {
            // Draw the character
            for (int col = 0; col < 5; col++) {
                uint8_t columnBits = fontData[col];
                for (int row = 0; row < 7; row++) {
                    if (columnBits & (1 << row)) {
                        // Scale up the pixel
                        for (int sx = 0; sx < scale; sx++) {
                            for (int sy = 0; sy < scale; sy++) {
                                setImagePixel(
                                    startCol + col * scale + sx,
                                    startRow + row * scale + sy,
                                    textColor
                                );
                            }
                        }
                    }
                }
            }
        }

        startCol += 6 * scale;  // Move to next character position
    }
}
