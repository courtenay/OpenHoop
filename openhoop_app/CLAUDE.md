# OpenHoop Flutter App

Mobile controller for OpenHoop LED hoops via BLE.

## Structure

```
lib/
└── main.dart          # Single-file app (BLE, UI, effects)
```

## Dependencies

- `flutter_blue_plus` - BLE communication
- `permission_handler` - Runtime permissions for BLE

## BLE UUIDs

```dart
service:        00001815-0000-1000-8000-00805f9b34fb
effect:         00000a92-0000-1000-8000-00805f9b34fb  // Write effect ID
color:          00000a93-0000-1000-8000-00805f9b34fb  // Write RGB
energy:         00000a95-0000-1000-8000-00805f9b34fb  // Write brightness 0-8
imu:            00000a94-0000-1000-8000-00805f9b34fb  // Notify IMU data
batteryService: 0000180f-0000-1000-8000-00805f9b34fb
batteryLevel:   00002a19-0000-1000-8000-00805f9b34fb
```

## Effect Categories

| Category | IDs | Examples |
|----------|-----|----------|
| Classic | 1-7 | Rainbow, Fire, Mushroom |
| Pattern | 80-86 | Liquid Sugar, Rainbow Dash |
| Sound | 87-91 | Sound Pulse, Sound Fire |
| Special | 95-99 | Water, Spectrum |

## Building

```bash
flutter pub get
flutter run -d <device>        # Debug
flutter run --release          # Release (faster, no hot reload)
```

## iOS Setup

Requires in `ios/Runner/Info.plist`:
- `NSBluetoothAlwaysUsageDescription`
- `NSBluetoothPeripheralUsageDescription`
- `UIBackgroundModes` with `bluetooth-central`

## Pattern Colors (from PatternEffect.h)

These are the actual hex colors used by each pattern:

| Pattern | Colors |
|---------|--------|
| Liquid Sugar | `#00FFFF, #00FF80, #40FFA0` (cyan-green) |
| Froth | `#0066FF, #00AAFF, #00FFFF` (blue-cyan) |
| Rainbow Dash | `#00FF00, #00FFFF, #0088FF, #FF00FF` (distinct blobs) |
| Fireball | `#FF4400, #FF8800, #FFFF00, #FFFFFF` (fire) |
| Flamebow | `#FF0000 → #FF00FF` (full rainbow gradient) |
| OG Sparkles | `#0044AA, #0088FF, #00CCFF, #FFFFFF` (blue sparkle) |
| Fruit Basket | `#FF0066, #FF4400, #FF8800, #FFCC00` (sunset) |

## IMU Data Format

12 bytes from IMU characteristic:
- Bytes 0-1: Accel X (int16, divide by 1000 for g)
- Bytes 2-3: Accel Y
- Bytes 4-5: Accel Z
- Bytes 6-7: Gyro X (int16, divide by 10 for deg/s)
- Bytes 8-9: Gyro Y
- Bytes 10-11: Gyro Z
