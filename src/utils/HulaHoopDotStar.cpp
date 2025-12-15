/**
 * @project OpenHoop
 * @file HulaHoopDotStar.cpp
 * @brief Implementation file for the HulaHoopDotStar class, providing functionality for managing LED strips with
 * energy-saving modes.
 * @details This class extends either Adafruit_DotStar or Adafruit_NeoPixel library to include energy-saving modes
 * and dynamic pixel management. The specific LED type is selected via Config.h.
 * @author github.com/angelcamelot
 * @date 2024-04-07
 * @license Open-source license.
 */

// Include Config.h which defines USE_DOTSTAR/USE_NEOPIXEL before including HulaHoopDotStar.h
#include "../../include/Config.h"

#ifdef USE_DOTSTAR
/**
 * @brief Constructor for the HulaHoopDotStar class using DotStar bit-bang SPI.
 * @param n Number of LEDs.
 * @param dataPin Data pin.
 * @param clockPin Clock pin.
 * @param order Color order.
 */
HulaHoopDotStar::HulaHoopDotStar(uint16_t n, uint8_t dataPin, uint8_t clockPin, uint8_t order)
    : Adafruit_DotStar(n, dataPin, clockPin, order), brightnessLevel(255), activePixels(n), energySavingModeLevel(0) {
    LED_BASE_CLASS::setBrightness(brightnessLevel);
}

/**
 * @brief Constructor for the HulaHoopDotStar class using DotStar hardware SPI.
 * @param n Number of LEDs.
 * @param order Color order.
 */
HulaHoopDotStar::HulaHoopDotStar(uint16_t n, uint8_t order)
    : Adafruit_DotStar(n, order), brightnessLevel(255), activePixels(n), energySavingModeLevel(0) {
    LED_BASE_CLASS::setBrightness(brightnessLevel);
}
#elif defined(USE_NEOPIXEL)
/**
 * @brief Constructor for the HulaHoopDotStar class using NeoPixel.
 * @param n Number of LEDs.
 * @param dataPin Data pin.
 * @param type LED type and color order.
 */
HulaHoopDotStar::HulaHoopDotStar(uint16_t n, uint8_t dataPin, neoPixelType type)
    : Adafruit_NeoPixel(n, dataPin, type), brightnessLevel(255), activePixels(n), energySavingModeLevel(0) {
    LED_BASE_CLASS::setBrightness(brightnessLevel);
}
#endif

/**
 * @brief Applies energy-saving mode to the specified level.
 * @param level Energy-saving mode level (0-8).
 */
void HulaHoopDotStar::applyEnergySavingMode(uint8_t level) {
    switch (level) {
        case 0: // No energy saving mode
            energySavingModeLevel = 0;
            activePixels = numPixels();
            brightnessLevel = 255;
            break;
        case 1: // All pixels at 70% brightness
            energySavingModeLevel = 1;
            activePixels = numPixels();
            brightnessLevel = 178;
            break;
        case 2: // All pixels at 40% brightness
            energySavingModeLevel = 2;
            activePixels = numPixels();
            brightnessLevel = 102;
            break;
        case 3: // Half of pixels at 100% brightness
            energySavingModeLevel = 3;
            activePixels = numPixels() / 2;
            brightnessLevel = 255;
            break;
        case 4: // Half of pixels at 70% brightness
            energySavingModeLevel = 4;
            activePixels = numPixels() / 2;
            brightnessLevel = 178;
            break;
        case 5: // Half of pixels at 40% brightness
            energySavingModeLevel = 5;
            activePixels = numPixels() / 2;
            brightnessLevel = 102;
            break;
        case 6: // Third of pixels at 100% brightness
            energySavingModeLevel = 6;
            activePixels = numPixels() / 3;
            brightnessLevel = 255;
            break;
        case 7: // Third of pixels at 70% brightness
            energySavingModeLevel = 7;
            activePixels = numPixels() / 3;
            brightnessLevel = 178;
            break;
        case 8: // Third of pixels at 40% brightness
            energySavingModeLevel = 8;
            activePixels = numPixels() / 3;
            brightnessLevel = 102;
            break;
        default:
            return; // Invalid mode
    }

    LED_BASE_CLASS::setBrightness(brightnessLevel);

    // Clear any unused physical pixels when reducing active pixel count to avoid ghosting.
    if (energySavingModeLevel >= 3) {
        LED_BASE_CLASS::fill(0, 0, numPixels());
    }
}

/**
 * @brief Override of setPixelColor to apply energy saving mode.
 * @param n Pixel number.
 * @param r Red color value.
 * @param g Green color value.
 * @param b Blue color value.
 */
void HulaHoopDotStar::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
    if (n >= activePixels) {
        return;
    }

    uint16_t step = 1;
    if (energySavingModeLevel >= 6) {
        step = 3;
    } else if (energySavingModeLevel >= 3) {
        step = 2;
    }

    uint16_t mappedIndex = n * step;
    if (mappedIndex >= numPixels()) {
        return;
    }

    LED_BASE_CLASS::setPixelColor(mappedIndex, r, g, b);

    // Ensure non-addressed neighbors remain dark for persistence of vision timing.
    for (uint16_t offset = 1; offset < step; ++offset) {
        uint16_t neighbor = mappedIndex + offset;
        if (neighbor < numPixels()) {
            LED_BASE_CLASS::setPixelColor(neighbor, 0, 0, 0);
        }
    }
}

/**
 * @brief Fills a range of pixels while respecting the energy saving mode.
 * @param color Color value.
 * @param first First pixel to fill.
 * @param count Number of pixels to fill.
 */
void HulaHoopDotStar::fill(uint32_t color, uint16_t first, uint16_t count) {
    if (first >= activePixels) {
        return;
    }

    if (count == 0 || first + count > activePixels) {
        count = activePixels - first;
    }

    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    for (uint16_t i = 0; i < count; ++i) {
        setPixelColor(first + i, r, g, b);
    }

    // Clear any remaining logical pixels in the selected range.
    uint16_t endLogical = first + count;
    for (uint16_t logical = endLogical; logical < activePixels; ++logical) {
        setPixelColor(logical, 0, 0, 0);
    }
}

/**
 * @brief Sets the energy-saving mode level.
 * @param level Energy-saving mode level (0-8).
 */
void HulaHoopDotStar::setEnergySavingMode(uint8_t level) {
    applyEnergySavingMode(level);
}

/**
 * @brief Gets the number of active pixels.
 * @return Number of active pixels.
 */
uint16_t HulaHoopDotStar::getActivePixels() const {
    return activePixels;
}

void HulaHoopDotStar::setDirectBrightness(uint8_t brightness) {
    brightnessLevel = brightness;
    LED_BASE_CLASS::setBrightness(brightnessLevel);
}

uint8_t HulaHoopDotStar::getBrightnessLevel() const {
    return brightnessLevel;
}

uint8_t HulaHoopDotStar::getEnergySavingModeLevel() const {
    return energySavingModeLevel;
}
