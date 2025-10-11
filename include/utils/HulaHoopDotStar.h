/**
 * @project OpenHoop
 * @file HulaHoopDotStar.h
 * @brief Header file for the HulaHoopDotStar class, providing functionality for managing DotStar LEDs with energy-saving
 * modes.
 * @details This class extends the Adafruit_DotStar library to include energy-saving modes and dynamic pixel
 * management.
 * @author github.com/angelcamelot
 * @date 2024-04-07
 * @license Open-source license.
 */

#ifndef OPENHOOP_HULAHOOPDOTSTAR_H
#define OPENHOOP_HULAHOOPDOTSTAR_H

#include <Adafruit_DotStar.h>

class HulaHoopDotStar : public Adafruit_DotStar {
private:
    /**
     * @brief Applies energy-saving mode to the specified level.
     */
    void applyEnergySavingMode(uint8_t level);

    uint8_t brightnessLevel; ///< Brightness level of the DotStar LEDs.
    uint16_t activePixels; ///< Number of active pixels.
    uint16_t energySavingModeLevel; ///< Energy-saving mode level (0-8).

public:
    /**
     * @brief Constructor for the HulaHoopDotStar class using bit-bang SPI.
     */
    HulaHoopDotStar(uint16_t n, uint8_t dataPin, uint8_t clockPin, uint8_t order = DOTSTAR_BGR);

    /**
     * @brief Constructor for the HulaHoopDotStar class using hardware SPI.
     */
    explicit HulaHoopDotStar(uint16_t n, uint8_t order = DOTSTAR_BGR);

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
};

#endif //OPENHOOP_HULAHOOPDOTSTAR_H
