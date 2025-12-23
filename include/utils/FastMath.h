/**
 * @project OpenHoop
 * @file FastMath.h
 * @brief Fast math utilities using lookup tables.
 * @details Optimized for 8-bit microcontrollers without FPU.
 */

#ifndef OPENHOOP_FASTMATH_H
#define OPENHOOP_FASTMATH_H

#include <Arduino.h>

namespace FastMath {

// 256-entry sine table (0-255 maps to 0-2*PI, output 0-255 maps to -1 to +1)
// Generated with: round(127.5 + 127.5 * sin(i * 2 * PI / 256))
const uint8_t SIN_TABLE[256] PROGMEM = {
    128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173,
    176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215,
    218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244,
    245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255,
    255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246,
    245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220,
    218, 215, 213, 211, 208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179,
    176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131,
    128, 124, 121, 118, 115, 112, 109, 106, 103, 100,  97,  93,  90,  88,  85,  82,
     79,  76,  73,  70,  67,  65,  62,  59,  57,  54,  52,  49,  47,  44,  42,  40,
     37,  35,  33,  31,  29,  27,  25,  23,  21,  20,  18,  17,  15,  14,  12,  11,
     10,   9,   7,   6,   5,   5,   4,   3,   2,   2,   1,   1,   1,   0,   0,   0,
      0,   0,   0,   0,   1,   1,   1,   2,   2,   3,   4,   5,   5,   6,   7,   9,
     10,  11,  12,  14,  15,  17,  18,  20,  21,  23,  25,  27,  29,  31,  33,  35,
     37,  40,  42,  44,  47,  49,  52,  54,  57,  59,  62,  65,  67,  70,  73,  76,
     79,  82,  85,  88,  90,  93,  97, 100, 103, 106, 109, 112, 115, 118, 121, 124
};

/**
 * @brief Fast sine using lookup table.
 * @param angle 0-255 representing 0 to 2*PI
 * @return 0-255 representing -1 to +1 (128 = 0)
 */
inline uint8_t sin8(uint8_t angle) {
    return pgm_read_byte(&SIN_TABLE[angle]);
}

/**
 * @brief Fast cosine using lookup table.
 * @param angle 0-255 representing 0 to 2*PI
 * @return 0-255 representing -1 to +1 (128 = 0)
 */
inline uint8_t cos8(uint8_t angle) {
    return pgm_read_byte(&SIN_TABLE[(uint8_t)(angle + 64)]);
}

/**
 * @brief Fast sine returning signed value.
 * @param angle 0-255 representing 0 to 2*PI
 * @return -128 to +127
 */
inline int8_t sin8_signed(uint8_t angle) {
    return (int8_t)(pgm_read_byte(&SIN_TABLE[angle]) - 128);
}

/**
 * @brief Scale a value by a sine wave.
 * @param value Value to scale (0-255)
 * @param angle Sine angle (0-255)
 * @return Scaled value (0-255)
 */
inline uint8_t scale8_sin(uint8_t value, uint8_t angle) {
    uint8_t s = sin8(angle);
    return (uint16_t)value * s / 255;
}

/**
 * @brief Linear interpolation between two 8-bit values.
 * @param a Start value
 * @param b End value
 * @param t Interpolation factor 0-255 (0=a, 255=b)
 * @return Interpolated value
 */
inline uint8_t lerp8(uint8_t a, uint8_t b, uint8_t t) {
    return a + (((int16_t)(b - a) * t) >> 8);
}

/**
 * @brief Blend two colors.
 * @param c1 First color (RGB packed)
 * @param c2 Second color (RGB packed)
 * @param t Blend factor 0-255 (0=c1, 255=c2)
 * @return Blended color
 */
inline uint32_t blendColor8(uint32_t c1, uint32_t c2, uint8_t t) {
    uint8_t r = lerp8((c1 >> 16) & 0xFF, (c2 >> 16) & 0xFF, t);
    uint8_t g = lerp8((c1 >> 8) & 0xFF, (c2 >> 8) & 0xFF, t);
    uint8_t b = lerp8(c1 & 0xFF, c2 & 0xFF, t);
    return (r << 16) | (g << 8) | b;
}

/**
 * @brief Scale a color's brightness.
 * @param color RGB packed color
 * @param scale Brightness 0-255
 * @return Scaled color
 */
inline uint32_t scaleColor8(uint32_t color, uint8_t scale) {
    uint8_t r = ((color >> 16) & 0xFF) * scale / 255;
    uint8_t g = ((color >> 8) & 0xFF) * scale / 255;
    uint8_t b = (color & 0xFF) * scale / 255;
    return (r << 16) | (g << 8) | b;
}

}  // namespace FastMath

#endif //OPENHOOP_FASTMATH_H
