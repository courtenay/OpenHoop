/**
 * @project OpenHoop
 * @file POVEffect.cpp
 * @brief Implementation of the POVEffect class.
 * @details Procedural POV patterns - no RAM buffer, computed on-the-fly.
 */

#include "../../include/effects/POVEffect.h"
#include "../../include/Config.h"
#include <Arduino_LSM9DS1.h>

// Number of angular slices (columns) in the POV image
static const int NUM_COLUMNS = 72;  // 5 degrees per column

POVEffect::POVEffect()
    : currentAngle(0.0f),
      lastUpdateTime(0),
      demoMode(true),
      demoSpeed(4.0f),
      currentPattern(0) {
}

void POVEffect::start() {
    currentAngle = 0.0f;
    lastUpdateTime = millis();
    currentPattern = 0;  // Start with heart
    Serial.println("POV Effect started - demo mode");
    Serial.println("Patterns: 0=Heart, 1=Spiral, 2=Text");
}

void POVEffect::update() {
    updateRotation();
    displayPattern();
}

void POVEffect::stop() {
    hoop.fill(HulaHoopDotStar::Color(0, 0, 0));
    hoop.show();
}

void POVEffect::updateRotation() {
    unsigned long now = millis();
    float dt = (now - lastUpdateTime) / 1000.0f;
    lastUpdateTime = now;

    if (demoMode) {
        // Demo: auto-advance angle
        currentAngle += demoSpeed;
    } else {
        // Real mode: use gyroscope Y-axis (hoop spins around vertical Y axis)
        float gx, gy, gz;
        if (IMU.readGyroscope(gx, gy, gz)) {
            // Y axis is vertical (calibrated), so gy measures hoop spin
            // Integrate angular velocity to get position
            currentAngle += gy * dt;
        }
    }

    // Normalize to 0-360
    while (currentAngle >= 360.0f) currentAngle -= 360.0f;
    while (currentAngle < 0.0f) currentAngle += 360.0f;
}

void POVEffect::displayPattern() {
    // Map angle to column index
    int column = static_cast<int>((currentAngle / 360.0f) * NUM_COLUMNS) % NUM_COLUMNS;

    // Draw the selected pattern
    switch (currentPattern) {
        case 0:
            drawHeart(column);
            break;
        case 1:
            drawSpiral(column);
            break;
        case 2:
            drawText(column);
            break;
        default:
            drawHeart(column);
            break;
    }

    hoop.show();
}

bool POVEffect::isInsideHeart(float x, float y) {
    // Heart equation: (x^2 + y^2 - 1)^3 - x^2 * y^3 < 0
    // Scaled and positioned for our coordinate system
    float x2 = x * x;
    float y2 = y * y;
    float expr = (x2 + y2 - 1.0f);
    return (expr * expr * expr - x2 * y * y * y) < 0.0f;
}

void POVEffect::drawHeart(int column) {
    int numLeds = hoop.getActivePixels();

    // Column angle in radians (0 to 2*PI)
    float angle = (column / static_cast<float>(NUM_COLUMNS)) * 2.0f * PI;

    // Heart is visible in roughly half the rotation (front-facing)
    // Map column to x-coordinate (-1.5 to 1.5)
    float x = 1.5f * cos(angle);

    for (int led = 0; led < numLeds; led++) {
        // Map LED position to y-coordinate
        // Center of strip = heart center, edges = top/bottom
        float y = 1.5f * (1.0f - 2.0f * led / static_cast<float>(numLeds - 1));

        if (isInsideHeart(x * 0.8f, y)) {
            // Red/pink heart
            hoop.setPixelColor(led, 255, 20, 60);
        } else {
            hoop.setPixelColor(led, 0, 0, 0);
        }
    }
}

void POVEffect::drawSpiral(int column) {
    int numLeds = hoop.getActivePixels();
    float angle = (column / static_cast<float>(NUM_COLUMNS)) * 360.0f;

    for (int led = 0; led < numLeds; led++) {
        // Spiral: color based on angle + position
        float ledPos = static_cast<float>(led) / numLeds;
        float hue = fmod(angle + ledPos * 180.0f, 360.0f);

        // Convert HSV to RGB (simplified)
        float h = hue / 60.0f;
        int i = static_cast<int>(h) % 6;
        float f = h - static_cast<int>(h);

        uint8_t r, g, b;
        uint8_t v = 255;
        uint8_t p = 0;
        uint8_t q = static_cast<uint8_t>(255 * (1.0f - f));
        uint8_t t = static_cast<uint8_t>(255 * f);

        switch (i) {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            default: r = v; g = p; b = q; break;
        }

        hoop.setPixelColor(led, r, g, b);
    }
}

void POVEffect::drawText(int column) {
    // Simple 5x7 font bitmap for "HI" stored in PROGMEM-compatible format
    // Each byte is a column, bits are rows
    static const uint8_t fontH[] = {0x7F, 0x08, 0x08, 0x08, 0x7F};
    static const uint8_t fontI[] = {0x00, 0x41, 0x7F, 0x41, 0x00};

    int numLeds = hoop.getActivePixels();
    int scale = numLeds / 14;  // Scale font to fit
    int centerOffset = (numLeds - 7 * scale) / 2;

    // Map column to text position
    // Text spans about 1/3 of rotation (120 degrees)
    int textColumns = NUM_COLUMNS / 3;
    int textStart = NUM_COLUMNS / 3;  // Start at 120 degrees

    // Clear all LEDs first
    for (int led = 0; led < numLeds; led++) {
        hoop.setPixelColor(led, 0, 0, 0);
    }

    // Check if we're in the text display region
    if (column >= textStart && column < textStart + textColumns) {
        int textCol = column - textStart;
        int charWidth = textColumns / 12;  // Space for 2 chars + gap

        const uint8_t* fontData = nullptr;
        int fontCol = -1;

        if (textCol < 5 * charWidth) {
            // "H"
            fontData = fontH;
            fontCol = textCol / charWidth;
        } else if (textCol >= 6 * charWidth && textCol < 11 * charWidth) {
            // "I"
            fontData = fontI;
            fontCol = (textCol - 6 * charWidth) / charWidth;
        }

        if (fontData && fontCol >= 0 && fontCol < 5) {
            uint8_t columnBits = fontData[fontCol];

            for (int bit = 0; bit < 7; bit++) {
                if (columnBits & (1 << bit)) {
                    // Light up LEDs for this bit
                    int ledStart = centerOffset + bit * scale;
                    for (int s = 0; s < scale && ledStart + s < numLeds; s++) {
                        hoop.setPixelColor(ledStart + s, 0, 255, 100);  // Green text
                    }
                }
            }
        }
    }
}
