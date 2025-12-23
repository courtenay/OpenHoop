/**
 * @project OpenHoop
 * @file PatternEffect.h
 * @brief Configurable pattern effect with color palette, blend modes, and sound reactivity.
 */

#ifndef OPENHOOP_PATTERNEFFECT_H
#define OPENHOOP_PATTERNEFFECT_H

#include <Arduino.h>
#include "Effect.h"
#include "../Config.h"
#include <vector>

/**
 * @brief Flexible pattern effect that renders color palettes.
 *
 * Can create many different looks:
 * - Smooth gradients (liquid sugar, froth)
 * - Distinct color blobs with gaps (rainbow dash, fireball)
 * - Scrolling or static patterns
 * - Sound-reactive brightness and speed modulation
 */
class PatternEffect : public Effect {
public:
    enum class BlendMode {
        GRADIENT,   // Smooth interpolation between colors
        SOLID       // Hard edges, distinct segments
    };

    struct Config {
        std::vector<uint32_t> colors;  // RGB colors in palette
        uint8_t segments;              // How many times to repeat pattern
        BlendMode blend;
        float speed;                   // Base animation speed (0 = static)
        float gapRatio;                // 0-1, fraction of black space between segments
        float brightness;              // Base brightness multiplier

        // Sound reactivity settings
        bool soundReactive;            // Enable sound reactivity
        float soundBrightness;         // 0-1, how much sound affects brightness
        float soundSpeed;              // 0-1, how much sound affects speed
        float soundSmoothing;          // 0-1, smoothing factor (lower = more responsive)

        Config() : segments(1), blend(BlendMode::GRADIENT), speed(1.0f),
                   gapRatio(0.0f), brightness(1.0f),
                   soundReactive(false), soundBrightness(0.0f),
                   soundSpeed(0.0f), soundSmoothing(0.3f) {}
    };

    PatternEffect(const Config& config);

    void start() override;
    void update() override;
    void stop() override;

private:
    Config config;
    float phase;              // Animation phase 0-1
    float smoothedIntensity;  // Smoothed sound intensity for animation

    uint32_t getColorAt(float position);  // Position 0-1 in the palette
    uint32_t blendColors(uint32_t c1, uint32_t c2, float t);
};

// ============ PRESET PATTERNS ============

namespace Patterns {

// Smooth cyan-green gradient
inline PatternEffect::Config liquidSugar() {
    PatternEffect::Config cfg;
    cfg.colors = {0x00FFFF, 0x00FF80, 0x40FFA0, 0x00FFFF};
    cfg.segments = 1;
    cfg.blend = PatternEffect::BlendMode::GRADIENT;
    cfg.speed = 0.3f;
    cfg.gapRatio = 0.0f;
    return cfg;
}

// Smooth blue-cyan
inline PatternEffect::Config froth() {
    PatternEffect::Config cfg;
    cfg.colors = {0x0066FF, 0x00AAFF, 0x00FFFF, 0x00AAFF, 0x0066FF};
    cfg.segments = 1;
    cfg.blend = PatternEffect::BlendMode::GRADIENT;
    cfg.speed = 0.2f;
    cfg.gapRatio = 0.0f;
    return cfg;
}

// Distinct colored blobs with gaps
inline PatternEffect::Config rainbowDash() {
    PatternEffect::Config cfg;
    cfg.colors = {0x00FF00, 0x00FFFF, 0x0088FF, 0xFF00FF};
    cfg.segments = 1;
    cfg.blend = PatternEffect::BlendMode::SOLID;
    cfg.speed = 0.5f;
    cfg.gapRatio = 0.3f;
    return cfg;
}

// Orange-red-white fire blobs
inline PatternEffect::Config fireball() {
    PatternEffect::Config cfg;
    cfg.colors = {0xFF4400, 0xFF8800, 0xFFFF00, 0xFFFFFF, 0xFFFF00, 0xFF8800, 0xFF4400};
    cfg.segments = 2;
    cfg.blend = PatternEffect::BlendMode::GRADIENT;
    cfg.speed = 0.4f;
    cfg.gapRatio = 0.2f;
    return cfg;
}

// Full rainbow gradient
inline PatternEffect::Config flamebow() {
    PatternEffect::Config cfg;
    cfg.colors = {0xFF0000, 0xFF8800, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0xFF00FF};
    cfg.segments = 1;
    cfg.blend = PatternEffect::BlendMode::GRADIENT;
    cfg.speed = 0.5f;
    cfg.gapRatio = 0.0f;
    return cfg;
}

// Rainbow with distinct segments
inline PatternEffect::Config rainbowBridge() {
    PatternEffect::Config cfg;
    cfg.colors = {0xFF0000, 0xFF8800, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0xFF00FF};
    cfg.segments = 1;
    cfg.blend = PatternEffect::BlendMode::SOLID;
    cfg.speed = 0.3f;
    cfg.gapRatio = 0.15f;
    return cfg;
}

// Blue with sparkle-like variation
inline PatternEffect::Config ogSparkles() {
    PatternEffect::Config cfg;
    cfg.colors = {0x0044AA, 0x0088FF, 0x00CCFF, 0xFFFFFF, 0x00CCFF, 0x0088FF};
    cfg.segments = 3;
    cfg.blend = PatternEffect::BlendMode::GRADIENT;
    cfg.speed = 0.6f;
    cfg.gapRatio = 0.0f;
    return cfg;
}

// Warm sunset colors
inline PatternEffect::Config fruitBasket() {
    PatternEffect::Config cfg;
    cfg.colors = {0xFF0066, 0xFF4400, 0xFF8800, 0xFFCC00, 0xFF8800, 0xFF4400, 0xFF0066};
    cfg.segments = 1;
    cfg.blend = PatternEffect::BlendMode::GRADIENT;
    cfg.speed = 0.3f;
    cfg.gapRatio = 0.0f;
    return cfg;
}

// ============ SOUND-REACTIVE PATTERNS ============

// Sound-reactive pulse - brightness follows music
inline PatternEffect::Config soundPulse() {
    PatternEffect::Config cfg;
    cfg.colors = {0xFF0088, 0xFF00FF, 0x8800FF, 0x0088FF, 0x00FFFF};
    cfg.segments = 1;
    cfg.blend = PatternEffect::BlendMode::GRADIENT;
    cfg.speed = 0.2f;
    cfg.gapRatio = 0.0f;
    cfg.brightness = 0.3f;  // Base brightness low
    cfg.soundReactive = true;
    cfg.soundBrightness = 0.7f;  // Sound adds up to 70% brightness
    cfg.soundSpeed = 0.0f;
    cfg.soundSmoothing = 0.2f;  // Responsive
    return cfg;
}

// Sound-reactive strobe - speed follows beats
inline PatternEffect::Config soundStrobe() {
    PatternEffect::Config cfg;
    cfg.colors = {0xFFFFFF, 0x000000};  // White and black
    cfg.segments = 4;
    cfg.blend = PatternEffect::BlendMode::SOLID;
    cfg.speed = 0.1f;  // Base speed low
    cfg.gapRatio = 0.0f;
    cfg.soundReactive = true;
    cfg.soundBrightness = 0.3f;
    cfg.soundSpeed = 0.8f;  // Sound speeds up animation
    cfg.soundSmoothing = 0.15f;  // Very responsive
    return cfg;
}

// Sound-reactive fire - intensity with music
inline PatternEffect::Config soundFire() {
    PatternEffect::Config cfg;
    cfg.colors = {0xFF2200, 0xFF4400, 0xFF8800, 0xFFCC00, 0xFFFF88};
    cfg.segments = 2;
    cfg.blend = PatternEffect::BlendMode::GRADIENT;
    cfg.speed = 0.3f;
    cfg.gapRatio = 0.1f;
    cfg.brightness = 0.4f;
    cfg.soundReactive = true;
    cfg.soundBrightness = 0.6f;
    cfg.soundSpeed = 0.3f;
    cfg.soundSmoothing = 0.25f;
    return cfg;
}

// Sound-reactive ocean waves
inline PatternEffect::Config soundWaves() {
    PatternEffect::Config cfg;
    cfg.colors = {0x001144, 0x003388, 0x0066CC, 0x00AAFF, 0x88DDFF};
    cfg.segments = 1;
    cfg.blend = PatternEffect::BlendMode::GRADIENT;
    cfg.speed = 0.15f;
    cfg.gapRatio = 0.0f;
    cfg.brightness = 0.5f;
    cfg.soundReactive = true;
    cfg.soundBrightness = 0.5f;
    cfg.soundSpeed = 0.4f;
    cfg.soundSmoothing = 0.4f;  // Smoother for waves
    return cfg;
}

// Sound-reactive rainbow - full spectrum response
inline PatternEffect::Config soundRainbow() {
    PatternEffect::Config cfg;
    cfg.colors = {0xFF0000, 0xFF8800, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0xFF00FF};
    cfg.segments = 1;
    cfg.blend = PatternEffect::BlendMode::GRADIENT;
    cfg.speed = 0.2f;
    cfg.gapRatio = 0.0f;
    cfg.brightness = 0.4f;
    cfg.soundReactive = true;
    cfg.soundBrightness = 0.6f;
    cfg.soundSpeed = 0.5f;
    cfg.soundSmoothing = 0.3f;
    return cfg;
}

}  // namespace Patterns

#endif //OPENHOOP_PATTERNEFFECT_H
