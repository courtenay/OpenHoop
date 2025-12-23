# OpenHoop Mobile App

Flutter app for controlling your OpenHoop LED hula hoop via Bluetooth Low Energy.

## Features

- Connect to OpenHoop via BLE
- Switch between effect patterns (Classic, Patterns, Sound Reactive, Special)
- Adjust brightness (10% - 100%)
- Battery level display

## Setup

1. Install Flutter SDK (3.0+)
2. Run `flutter pub get`
3. Connect your phone via USB

### Android
```bash
flutter run
```

### iOS (Sideloading)
1. Open `ios/Runner.xcworkspace` in Xcode
2. Sign with your Apple ID (free developer account works)
3. Trust the developer certificate on your iPhone:
   Settings > General > VPN & Device Management > Trust
4. Build and run from Xcode

## BLE Protocol

- Service UUID: `0x1815` (Automation IO)
- Effect Characteristic: `0x0A92` - Write effect ID (uint8)
- Color Characteristic: `0x0A93` - Write RGB hex string
- Brightness Characteristic: `0x0A95` - Write level 0-8 (0=100%, 8=10%)

## Effect IDs

### Classic (0-7)
- 1: Rainbow
- 2: Color Wave
- 3: Funky
- 4: Rastafari
- 5: Fire
- 6: Leopard
- 7: Mushroom

### Patterns (80-86)
- 80: Liquid Sugar
- 81: Froth
- 82: Rainbow Dash (sound reactive)
- 83: Fireball
- 84: Flamebow
- 85: OG Sparkles
- 86: Fruit Basket

### Sound Reactive (87-91)
- 87: Sound Pulse
- 88: Sound Strobe
- 89: Sound Fire
- 90: Sound Waves
- 91: Sound Rainbow

### Special (95-99)
- 95: Water
- 99: Spectrum
