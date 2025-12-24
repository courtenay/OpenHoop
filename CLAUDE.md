# OpenHoop Project

An open-source LED smart hoop with BLE control, IMU-based effects, and sound reactivity.

## Project Structure

```
openhoop/
├── src/                    # C++ source files (Arduino/ESP32)
│   ├── main.cpp           # Entry point, setup() and loop()
│   └── services/          # BLE, LED, Effect, Audio services
├── include/               # Header files
│   ├── Config.h           # Hardware pins, LED count, BLE UUIDs
│   ├── effects/           # Effect implementations
│   └── services/          # Service interfaces
├── webapp/                # Web-based BLE controller (vanilla JS + Three.js)
├── openhoop_app/          # Flutter mobile app
├── lib/                   # External Arduino libraries
└── test/                  # Unit tests
```

## Hardware

- **MCU**: Arduino Nano 33 BLE (nRF52840)
- **LEDs**: WS2812B (144 LEDs default, configurable in Config.h)
- **IMU**: LSM6DS3 (built into Nano 33 BLE)
- **Audio**: MAX4466 electret mic on A0

## BLE Protocol

Service UUID: `1815` (Automation IO)

| Characteristic | UUID   | Description |
|---------------|--------|-------------|
| Effect        | `0A92` | Write 1 byte effect ID |
| Color         | `0A93` | Write 3 bytes RGB for solid color |
| Energy        | `0A95` | Write brightness level 0-8 |
| IMU           | `0A94` | Notify 12 bytes (6x int16: ax,ay,az,gx,gy,gz) |
| Battery       | `2A19` | Read battery % (standard BLE) |

## Effect IDs

- **0**: Off
- **1-7**: Classic (Rainbow, Color Wave, Funky, Rastafari, Fire, Leopard, Mushroom)
- **80-86**: Patterns (Liquid Sugar, Froth, Rainbow Dash, Fireball, Flamebow, OG Sparkles, Fruit Basket)
- **87-91**: Sound-reactive (Pulse, Strobe, Fire, Waves, Rainbow)
- **95-99**: Special (Water, Calibrate, POV, Pulse, Spectrum)
- **255**: Idle/default

## Building

```bash
# Firmware (requires PlatformIO)
pio run -t upload

# Flutter app
cd openhoop_app && flutter run

# Web app - just open webapp/index.html
```

## Key Files

- `include/Config.h` - All hardware configuration
- `include/effects/PatternEffect.h` - Pattern color definitions
- `include/services/EffectService.h` - Effect type enum
- `src/services/BleService.cpp` - BLE implementation
- `webapp/index.html` - Web controller with 3D IMU visualization
