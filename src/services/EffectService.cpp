/**
 * @project OpenHoop
 * @file EffectService.cpp
 * @brief Implementation file for the EffectService class, responsible for managing LED effects.
 * @details Implements the methods defined in the EffectService class for setting, updating, and dispatching BLE commands for LED effects.
 * @author github.com/angelcamelot
 * @date 2024-03-17
 * @license Open-source license.
 */

#include "../../include/services/EffectService.h"
#include "../../include/effects/SolidColorFillEffect.h"
#include "../../include/effects/RainbowEffect.h"
#include "../../include/effects/FireEffect.h"
#include "../../include/effects/PulseEffect.h"
#include "../../include/effects/ColorWaveEffect.h"
#include "../../include/effects/FunkyEffect.h"
#include "../../include/effects/RastafaraiFlagEffect.h"
#include "../../include/effects/SpectrumEffect.h"
#include "../../include/effects/RainbowLeopardEffect.h"
#include "../../include/effects/MushroomImageEffect.h"
#include "../../include/effects/POVEffect.h"
#include "../../include/effects/PatternEffect.h"

#ifdef FEATURE_IMU
#include "../../include/effects/CalibrateEffect.h"
#include "../../include/effects/WaterEffect.h"
#endif
#include "../../include/Config.h"

/**
 * @brief Set a new effect, stopping and deleting the current one if it exists.
 */
void EffectService::setEffect(std::unique_ptr<Effect> newEffect) {
    // The unique_ptr will automatically handle the deletion of the currentEffect
    // when it goes out of scope or gets replaced.
    if (currentEffect) {
        currentEffect->stop();
    }

    currentEffect = std::move(newEffect);

    // Start the new effect
    if (currentEffect) {
        currentEffect->start();
    }
}

/**
 * @brief Update the current effect if it exists.
 */
void EffectService::update() const {
    // Update the current effect
    if (currentEffect) {
        currentEffect->update();
    }
}

/**
 * @brief Dispatch an effect command based on the provided gesture.
 * @param gesture The type of effect to dispatch.
 */
void EffectService::dispatchEffectCommand(EffectType gesture) {
    switch (gesture) {
        case EffectType::NO_EFFECT:
            setEffect(std::make_unique<SolidColorFillEffect>(HulaHoopDotStar::Color(0, 0, 0)));
            break;
        case EffectType::RAINBOW:
            setEffect(std::make_unique<RainbowEffect>());
            break;
        case EffectType::FIRE:
            setEffect(std::make_unique<FireEffect>());
            break;
        case EffectType::PULSE:
            setEffect(std::make_unique<PulseEffect>());
            break;
        case EffectType::COLORWAVE:
            setEffect(std::make_unique<ColorWaveEffect>());
            break;
        case EffectType::FUNKY:
            setEffect(std::make_unique<FunkyEffect>());
            break;
        case EffectType::RASTAFARAIFLAG:
            setEffect(std::make_unique<RastafaraiFlagEffect>());
            break;
        case EffectType::SPECTRUM:
            setEffect(std::make_unique<SpectrumEffect>());
            break;
        case EffectType::MUSHROOM:
            setEffect(std::make_unique<MushroomImageEffect>());
            break;
        case EffectType::LEOPARDRAINBOW:
            setEffect(std::make_unique<RainbowLeopardEffect>());
            break;
        case EffectType::POV:
            setEffect(std::make_unique<POVEffect>());
            break;
#ifdef FEATURE_IMU
        case EffectType::CALIBRATE:
            setEffect(std::make_unique<CalibrateEffect>());
            break;
        case EffectType::WATER:
            setEffect(std::make_unique<WaterEffect>());
            break;
#endif
        // Pattern presets
        case EffectType::PATTERN_LIQUID_SUGAR:
            setEffect(std::make_unique<PatternEffect>(Patterns::liquidSugar()));
            break;
        case EffectType::PATTERN_FROTH:
            setEffect(std::make_unique<PatternEffect>(Patterns::froth()));
            break;
        case EffectType::PATTERN_RAINBOW_DASH:
            setEffect(std::make_unique<PatternEffect>(Patterns::rainbowDash()));
            break;
        case EffectType::PATTERN_FIREBALL:
            setEffect(std::make_unique<PatternEffect>(Patterns::fireball()));
            break;
        case EffectType::PATTERN_FLAMEBOW:
            setEffect(std::make_unique<PatternEffect>(Patterns::flamebow()));
            break;
        case EffectType::PATTERN_OG_SPARKLES:
            setEffect(std::make_unique<PatternEffect>(Patterns::ogSparkles()));
            break;
        case EffectType::PATTERN_FRUIT_BASKET:
            setEffect(std::make_unique<PatternEffect>(Patterns::fruitBasket()));
            break;
        // Sound-reactive patterns
        case EffectType::SOUND_PULSE:
            setEffect(std::make_unique<PatternEffect>(Patterns::soundPulse()));
            break;
        case EffectType::SOUND_STROBE:
            setEffect(std::make_unique<PatternEffect>(Patterns::soundStrobe()));
            break;
        case EffectType::SOUND_FIRE:
            setEffect(std::make_unique<PatternEffect>(Patterns::soundFire()));
            break;
        case EffectType::SOUND_WAVES:
            setEffect(std::make_unique<PatternEffect>(Patterns::soundWaves()));
            break;
        case EffectType::SOUND_RAINBOW:
            setEffect(std::make_unique<PatternEffect>(Patterns::soundRainbow()));
            break;
        default:
            // Stop the current effect if no matching gesture is found
            setEffect(nullptr);
            break;
    }
}

/**
 * @brief Dispatch a solid color command based on the provided color string.
 * @param colorString The string representing the RGB color code.
 */
void EffectService::dispatchSolidColorCommand(const String& colorString) {
    // Handle potential errors gracefully:
    if (colorString.length() != 6) {
        DEBUG_PRINTLN("Invalid color code length (must be 6 characters)");
        setEffect(std::make_unique<SolidColorFillEffect>(HulaHoopDotStar::Color(0, 0, 0))); // Set default color in case of error
        return;
    }

    // Extract color components using `strtol` (ensure exceptions are enabled):
    uint8_t red = strtol(colorString.substring(0, 2).c_str(), nullptr, 16);
    uint8_t green = strtol(colorString.substring(2, 4).c_str(), nullptr, 16);
    uint8_t blue = strtol(colorString.substring(4, 6).c_str(), nullptr, 16);

    // Log the extracted RGB values (for debugging purposes):
    DEBUG_PRINT("Extracted RGB values: ");
    DEBUG_PRINT("Red: "); DEBUG_PRINT(red); DEBUG_PRINT(", ");
    DEBUG_PRINT("Green: "); DEBUG_PRINT(green); DEBUG_PRINT(", ");
    DEBUG_PRINT("Blue: "); DEBUG_PRINTLN(blue);

    // Create and set the solid color effect:
    setEffect(std::make_unique<SolidColorFillEffect>(HulaHoopDotStar::Color(red, green, blue)));
}

EffectService::~EffectService() = default;
