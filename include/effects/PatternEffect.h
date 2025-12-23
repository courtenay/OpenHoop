/**
 * @project OpenHoop
 * @file PatternEffect.h
 * @brief Configurable pattern effect with color palette and blend modes.
 */

#ifndef OPENHOOP_PATTERNEFFECT_H
#define OPENHOOP_PATTERNEFFECT_H

#include "Effect.h"
#include <vector>

/**
 * @brief Flexible pattern effect that renders color palettes.
 *
 * Can create many different looks:
 * - Smooth gradients (liquid sugar, froth)
 * - Distinct color blobs with gaps (rainbow dash, fireball)
 * - Scrolling or static patterns
 */
class PatternEffect : public Effect {
public:
    enum class BlendMode {
        GRADIENT,   // Smooth interpolation between colors
        SOLID       // Hard edges, distinct segments
    };

    struct Config {
        std::vector<uint32_t> colors;  // RGB colors in palette
        uint8_t segments = 1;          // How many times to repeat pattern
        BlendMode blend = BlendMode::GRADIENT;
        float speed = 1.0f;            // Animation speed (0 = static)
        float gapRatio = 0.0f;         // 0-1, fraction of black space between segments
        float brightness = 1.0f;       // Overall brightness multiplier
    };

    PatternEffect(const Config& config);

    void start() override;
    void update() override;
    void stop() override;

private:
    Config config;
    float phase;  // Animation phase 0-1

    uint32_t getColorAt(float position);  // Position 0-1 in the palette
    uint32_t blendColors(uint32_t c1, uint32_t c2, float t);
};

// ============ PRESET PATTERNS ============

namespace Patterns {

// Smooth cyan-green gradient
inline PatternEffect::Config liquidSugar() {
    return {
        .colors = {0x00FFFF, 0x00FF80, 0x40FFA0, 0x00FFFF},
        .segments = 1,
        .blend = PatternEffect::BlendMode::GRADIENT,
        .speed = 0.3f,
        .gapRatio = 0.0f
    };
}

// Smooth blue-cyan
inline PatternEffect::Config froth() {
    return {
        .colors = {0x0066FF, 0x00AAFF, 0x00FFFF, 0x00AAFF, 0x0066FF},
        .segments = 1,
        .blend = PatternEffect::BlendMode::GRADIENT,
        .speed = 0.2f,
        .gapRatio = 0.0f
    };
}

// Distinct colored blobs with gaps
inline PatternEffect::Config rainbowDash() {
    return {
        .colors = {0x00FF00, 0x00FFFF, 0x0088FF, 0xFF00FF},
        .segments = 1,
        .blend = PatternEffect::BlendMode::SOLID,
        .speed = 0.5f,
        .gapRatio = 0.3f
    };
}

// Orange-red-white fire blobs
inline PatternEffect::Config fireball() {
    return {
        .colors = {0xFF4400, 0xFF8800, 0xFFFF00, 0xFFFFFF, 0xFFFF00, 0xFF8800, 0xFF4400},
        .segments = 2,
        .blend = PatternEffect::BlendMode::GRADIENT,
        .speed = 0.4f,
        .gapRatio = 0.2f
    };
}

// Full rainbow gradient
inline PatternEffect::Config flamebow() {
    return {
        .colors = {0xFF0000, 0xFF8800, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0xFF00FF},
        .segments = 1,
        .blend = PatternEffect::BlendMode::GRADIENT,
        .speed = 0.5f,
        .gapRatio = 0.0f
    };
}

// Rainbow with distinct segments
inline PatternEffect::Config rainbowBridge() {
    return {
        .colors = {0xFF0000, 0xFF8800, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0xFF00FF},
        .segments = 1,
        .blend = PatternEffect::BlendMode::SOLID,
        .speed = 0.3f,
        .gapRatio = 0.15f
    };
}

// Blue with sparkle-like variation
inline PatternEffect::Config ogSparkles() {
    return {
        .colors = {0x0044AA, 0x0088FF, 0x00CCFF, 0xFFFFFF, 0x00CCFF, 0x0088FF},
        .segments = 3,
        .blend = PatternEffect::BlendMode::GRADIENT,
        .speed = 0.6f,
        .gapRatio = 0.0f
    };
}

// Warm sunset colors
inline PatternEffect::Config fruitBasket() {
    return {
        .colors = {0xFF0066, 0xFF4400, 0xFF8800, 0xFFCC00, 0xFF8800, 0xFF4400, 0xFF0066},
        .segments = 1,
        .blend = PatternEffect::BlendMode::GRADIENT,
        .speed = 0.3f,
        .gapRatio = 0.0f
    };
}

}  // namespace Patterns

#endif //OPENHOOP_PATTERNEFFECT_H
