/**
 * @project OpenHoop
 * @file PatternEffect.cpp
 * @brief Implementation of configurable pattern effect.
 */

#include "../../include/effects/PatternEffect.h"
#include "../../include/Config.h"

PatternEffect::PatternEffect(const Config& cfg)
    : config(cfg), phase(0.0f) {}

void PatternEffect::start() {
    phase = 0.0f;
}

void PatternEffect::update() {
    if (config.colors.empty()) {
        hoop.fill(0);
        hoop.show();
        return;
    }

    int numLeds = hoop.getActivePixels();

    // Advance animation phase
    phase += config.speed * 0.01f;
    if (phase >= 1.0f) phase -= 1.0f;

    for (int i = 0; i < numLeds; i++) {
        // Calculate position in pattern (0-1)
        float pos = static_cast<float>(i) / numLeds;

        // Apply segments (repeat pattern)
        pos = pos * config.segments;
        pos = pos - floor(pos);  // Keep 0-1

        // Add animation offset
        pos = pos + phase;
        if (pos >= 1.0f) pos -= 1.0f;

        // Check if in gap region
        if (config.gapRatio > 0.0f) {
            // Each color gets equal space, gap comes after each
            float segmentSize = 1.0f / config.colors.size();
            float posInSegment = fmod(pos, segmentSize) / segmentSize;

            if (posInSegment > (1.0f - config.gapRatio)) {
                // In the gap - show black
                hoop.setPixelColor(i, 0, 0, 0);
                continue;
            }
            // Rescale position within the non-gap portion
            pos = fmod(pos, segmentSize) / segmentSize / (1.0f - config.gapRatio);
            pos = fmod(pos * config.colors.size() + floor(fmod(pos * config.colors.size(), 1.0f)), 1.0f);
            // Recalculate for correct color mapping
            float colorPos = fmod(phase + static_cast<float>(i) / numLeds * config.segments, 1.0f);
            pos = colorPos;
        }

        uint32_t color = getColorAt(pos);

        // Apply brightness
        uint8_t r = ((color >> 16) & 0xFF) * config.brightness;
        uint8_t g = ((color >> 8) & 0xFF) * config.brightness;
        uint8_t b = (color & 0xFF) * config.brightness;

        hoop.setPixelColor(i, r, g, b);
    }

    hoop.show();
}

void PatternEffect::stop() {
    hoop.fill(0);
    hoop.show();
}

uint32_t PatternEffect::getColorAt(float position) {
    if (config.colors.size() == 1) {
        return config.colors[0];
    }

    // Position 0-1 maps to color array
    float scaledPos = position * config.colors.size();
    int idx1 = static_cast<int>(scaledPos) % config.colors.size();
    int idx2 = (idx1 + 1) % config.colors.size();
    float t = scaledPos - floor(scaledPos);

    if (config.blend == BlendMode::SOLID) {
        // Hard edges - no interpolation
        return config.colors[idx1];
    }

    // Gradient - interpolate between colors
    return blendColors(config.colors[idx1], config.colors[idx2], t);
}

uint32_t PatternEffect::blendColors(uint32_t c1, uint32_t c2, float t) {
    uint8_t r1 = (c1 >> 16) & 0xFF;
    uint8_t g1 = (c1 >> 8) & 0xFF;
    uint8_t b1 = c1 & 0xFF;

    uint8_t r2 = (c2 >> 16) & 0xFF;
    uint8_t g2 = (c2 >> 8) & 0xFF;
    uint8_t b2 = c2 & 0xFF;

    uint8_t r = r1 + (r2 - r1) * t;
    uint8_t g = g1 + (g2 - g1) * t;
    uint8_t b = b1 + (b2 - b1) * t;

    return (r << 16) | (g << 8) | b;
}
