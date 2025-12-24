# OpenHoop Architecture

## File Index

### Core Files

| File | Purpose |
|------|---------|
| `src/main.cpp` | Entry point: `setup()`, `loop()` |
| `include/Config.h` | Hardware config, pin definitions, BLE UUIDs |

### Services (`src/services/`, `include/services/`)

| File | Class | Key Functions |
|------|-------|---------------|
| `BleService.cpp/.h` | `BleService` | `init()`, `updateIMUData()`, `handleEffectWrite()` |
| `EffectService.cpp/.h` | `EffectService` | `setEffect()`, `dispatchEffectCommand()`, `update()` |

### Effects (`src/effects/`, `include/effects/`)

| File | Class | Effect ID |
|------|-------|-----------|
| `RainbowEffect.cpp/.h` | `RainbowEffect` | 1 |
| `ColorWaveEffect.cpp/.h` | `ColorWaveEffect` | 2 |
| `FunkyEffect.cpp/.h` | `FunkyEffect` | 3 |
| `RastafaraiFlagEffect.cpp/.h` | `RastafaraiFlagEffect` | 4 |
| `FireEffect.cpp/.h` | `FireEffect` | 5 |
| `RainbowLeopardEffect.cpp/.h` | `RainbowLeopardEffect` | 6 |
| `MushroomImageEffect.cpp/.h` | `MushroomImageEffect` | 7 |
| `PatternEffect.cpp/.h` | `PatternEffect` | 80-91 (configurable) |
| `WaterEffect.cpp/.h` | `WaterEffect` | 95 |
| `CalibrateEffect.cpp/.h` | `CalibrateEffect` | 96 |
| `POVEffect.cpp/.h` | `POVEffect` | 97 |
| `PulseEffect.cpp/.h` | `PulseEffect` | 98 |
| `SpectrumEffect.cpp/.h` | `SpectrumEffect` | 99 |
| `SolidColorFillEffect.cpp/.h` | `SolidColorFillEffect` | (via color char) |

### Images (`src/images/`, `include/images/`)

| File | Class | Description |
|------|-------|-------------|
| `LedImage.cpp/.h` | `LedImage` | Base class for POV images |
| `MushroomImage.cpp/.h` | `MushroomImage` | Mushroom sprite |
| `LeopardPrintImage.cpp/.h` | `LeopardPrintImage` | Leopard pattern |
| `FlowerImage.cpp/.h` | `FlowerImage` | Flower sprite |
| `CupcakeImage.cpp/.h` | `CupcakeImage` | Cupcake sprite |

### Utilities (`src/utils/`, `include/utils/`)

| File | Purpose |
|------|---------|
| `EffectUtils.cpp/.h` | Color math, HSV conversion |
| `HulaHoopDotStar.cpp/.h` | LED strip driver wrapper |

---

## Effect Interface

All effects implement:

```cpp
class Effect {
    virtual void start();   // Called when effect activates
    virtual void update();  // Called each frame (~60fps)
    virtual void stop();    // Called when effect deactivates
};
```

---

## Data Flow

```
BLE Write (0A92) → BleService → EffectService::dispatchEffectCommand()
                                      ↓
                              setEffect(new XxxEffect())
                                      ↓
                              Effect::update() called in loop()
                                      ↓
                              LED strip updated
```

---

## IMU Data Flow

```
IMU::read() → BleService::updateIMUData() → BLE Notify (0A94)
                    ↓
              Effects (Water, Calibrate) use IMU directly
```

---

## Pattern Configuration

`PatternEffect` is highly configurable via `PatternEffect::Config`:

```cpp
struct Config {
    vector<uint32_t> colors;   // Color palette (RGB hex)
    uint8_t segments;          // Repetitions around hoop
    BlendMode blend;           // GRADIENT or SOLID
    float speed;               // Animation speed
    float gapRatio;            // Black space between colors
    bool soundReactive;        // Enable mic input
    float soundBrightness;     // Sound → brightness mapping
    float soundSpeed;          // Sound → speed mapping
};
```

Presets in `Patterns::` namespace: `liquidSugar()`, `froth()`, `rainbowDash()`, etc.
