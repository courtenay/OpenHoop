/**
 * @project OpenHoop
 * @file HulaHoopDotStar.h
 * @brief Header file for the HulaHoopDotStar class, providing functionality for managing LED strips with energy-saving
 * modes.
 * @details This class extends either Adafruit_DotStar or Adafruit_NeoPixel library to include energy-saving modes
 * and dynamic pixel management. The specific LED type is selected via Config.h.
 * @author github.com/angelcamelot
 * @date 2024-04-07
 * @license Open-source license.
 */

#ifndef OPENHOOP_HULAHOOPDOTSTAR_H
#define OPENHOOP_HULAHOOPDOTSTAR_H

// Include the appropriate library based on configuration
#ifdef USE_DOTSTAR
  #include <Adafruit_DotStar.h>
  #define LED_BASE_CLASS Adafruit_DotStar
#elif defined(USE_NEOPIXEL)
  #include <Adafruit_NeoPixel.h>
  #define LED_BASE_CLASS Adafruit_NeoPixel
#else
  #error "Must define USE_DOTSTAR or USE_NEOPIXEL in Config.h"
#endif

class HulaHoopDotStar : public LED_BASE_CLASS {
private:
    /**
     * @brief Applies energy-saving mode to the specified level.
     */
    void applyEnergySavingMode(uint8_t level);

    uint8_t brightnessLevel; ///< Brightness level of the DotStar LEDs.
    uint16_t activePixels; ///< Number of active pixels.
    uint8_t energySavingModeLevel; ///< Energy-saving mode level (0-8).

public:
#ifdef USE_DOTSTAR
    /**
     * @brief Constructor for DotStar using bit-bang SPI.
     * @param n Number of LEDs.
     * @param dataPin Data pin.
     * @param clockPin Clock pin.
     * @param order Color order (default: DOTSTAR_BGR).
     */
    HulaHoopDotStar(uint16_t n, uint8_t dataPin, uint8_t clockPin, uint8_t order = DOTSTAR_BGR);

    /**
     * @brief Constructor for DotStar using hardware SPI.
     * @param n Number of LEDs.
     * @param order Color order (default: DOTSTAR_BGR).
     */
    explicit HulaHoopDotStar(uint16_t n, uint8_t order = DOTSTAR_BGR);
#elif defined(USE_NEOPIXEL)
    /**
     * @brief Constructor for NeoPixel.
     * @param n Number of LEDs.
     * @param dataPin Data pin.
     * @param type LED type and color order (default: NEO_GRB + NEO_KHZ800).
     */
    HulaHoopDotStar(uint16_t n, uint8_t dataPin, neoPixelType type = NEO_GRB + NEO_KHZ800);
#endif

    /**
     * @brief Override of setPixelColor to apply energy saving mode.
     */
    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Fills a range of pixels while respecting the energy saving mode.
     */
    void fill(uint32_t color, uint16_t first = 0, uint16_t count = 0);

    /**
     * @brief Sets the energy-saving mode level.
     */
    void setEnergySavingMode(uint8_t level);

    /**
     * @brief Gets the number of active pixels.
     */
    uint16_t getActivePixels() const;

    /**
     * @brief Overrides the current brightness level regardless of energy-saving mode.
     */
    void setDirectBrightness(uint8_t brightness);

    /**
     * @brief Gets the currently applied brightness level.
     */
    uint8_t getBrightnessLevel() const;

    /**
     * @brief Gets the currently applied energy-saving mode level.
     */
    uint8_t getEnergySavingModeLevel() const;
};

#endif //OPENHOOP_HULAHOOPDOTSTAR_H
