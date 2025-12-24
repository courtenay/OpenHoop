import 'dart:async';
import 'dart:math';
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';

void main() {
  runApp(const OpenHoopApp());
}

class OpenHoopApp extends StatelessWidget {
  const OpenHoopApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'OpenHoop',
      theme: ThemeData(
        brightness: Brightness.dark,
        primarySwatch: Colors.purple,
        scaffoldBackgroundColor: const Color(0xFF1A1A2E),
      ),
      home: const HoopController(),
    );
  }
}

// BLE UUIDs - OpenHoop uses 16-bit UUIDs
class BleUuids {
  static final service = Guid('00001815-0000-1000-8000-00805f9b34fb');
  static final effect = Guid('00000a92-0000-1000-8000-00805f9b34fb');
  static final color = Guid('00000a93-0000-1000-8000-00805f9b34fb');
  static final imu = Guid('00000a94-0000-1000-8000-00805f9b34fb');
  static final energy = Guid('00000a95-0000-1000-8000-00805f9b34fb');
  static final batteryService = Guid('0000180f-0000-1000-8000-00805f9b34fb');
  static final batteryLevel = Guid('00002a19-0000-1000-8000-00805f9b34fb');
}

// Effect definitions with actual color palettes from PatternEffect.h
class EffectDef {
  final String name;
  final int id;
  final List<Color> colors; // Actual pattern colors

  const EffectDef(this.name, this.id, this.colors);
}

// Helper to convert hex to Color
Color hexColor(int hex) => Color(0xFF000000 | hex);

class HoopController extends StatefulWidget {
  const HoopController({super.key});

  @override
  State<HoopController> createState() => _HoopControllerState();
}

class _HoopControllerState extends State<HoopController> {
  BluetoothDevice? _device;
  BluetoothCharacteristic? _effectChar;
  BluetoothCharacteristic? _colorChar;
  BluetoothCharacteristic? _energyChar;
  BluetoothCharacteristic? _batteryChar;
  BluetoothCharacteristic? _imuChar;
  StreamSubscription? _imuSubscription;

  bool _isConnected = false;
  bool _isScanning = false;
  bool _isCalibrating = false;
  int _batteryLevel = -1;
  int _brightnessLevel = 4; // Default 50%
  String _status = 'Disconnected';

  // IMU data from Madgwick filter (already fused and stable)
  double _roll = 0, _pitch = 0, _yaw = 0; // Euler angles in degrees
  double _accelX = 0, _accelY = 0, _accelZ = 0; // Raw accel for effects
  // Smoothed values for visualization (light smoothing on already-filtered data)
  double _smoothRoll = 0, _smoothPitch = 0, _smoothYaw = 0;
  static const double _smoothing = 0.3; // Higher since Madgwick already filters

  // Brightness levels: 0=100%, 1=88%, 2=75%, 3=63%, 4=50%, 5=38%, 6=25%, 7=15%, 8=10%
  static const List<int> brightnessPercents = [100, 88, 75, 63, 50, 38, 25, 15, 10];

  // Classic effects (1-7) - simple color representations
  static final classicEffects = [
    EffectDef('Rainbow', 1, [hexColor(0xFF0000), hexColor(0xFF8800), hexColor(0xFFFF00), hexColor(0x00FF00), hexColor(0x00FFFF), hexColor(0x0000FF), hexColor(0xFF00FF)]),
    EffectDef('Color Wave', 2, [hexColor(0x0000FF), hexColor(0x00FFFF), hexColor(0x00FF00)]),
    EffectDef('Funky', 3, [hexColor(0xFF00FF), hexColor(0xFFFF00), hexColor(0x00FFFF)]),
    EffectDef('Rastafari', 4, [hexColor(0xFF0000), hexColor(0xFFFF00), hexColor(0x00FF00)]),
    EffectDef('Fire', 5, [hexColor(0xFF2200), hexColor(0xFF4400), hexColor(0xFF8800), hexColor(0xFFCC00)]),
    EffectDef('Leopard', 6, [hexColor(0xFFAA00), hexColor(0x000000), hexColor(0xFF8800)]),
    EffectDef('Mushroom', 7, [hexColor(0xFF0000), hexColor(0xFFFFFF), hexColor(0xFF4444)]),
  ];

  // Pattern effects (80-86) - actual colors from PatternEffect.h
  static final patternEffects = [
    EffectDef('Liquid Sugar', 80, [hexColor(0x00FFFF), hexColor(0x00FF80), hexColor(0x40FFA0)]),
    EffectDef('Froth', 81, [hexColor(0x0066FF), hexColor(0x00AAFF), hexColor(0x00FFFF)]),
    EffectDef('Rainbow Dash', 82, [hexColor(0x00FF00), hexColor(0x00FFFF), hexColor(0x0088FF), hexColor(0xFF00FF)]),
    EffectDef('Fireball', 83, [hexColor(0xFF4400), hexColor(0xFF8800), hexColor(0xFFFF00), hexColor(0xFFFFFF)]),
    EffectDef('Flamebow', 84, [hexColor(0xFF0000), hexColor(0xFF8800), hexColor(0xFFFF00), hexColor(0x00FF00), hexColor(0x00FFFF), hexColor(0x0000FF), hexColor(0xFF00FF)]),
    EffectDef('OG Sparkles', 85, [hexColor(0x0044AA), hexColor(0x0088FF), hexColor(0x00CCFF), hexColor(0xFFFFFF)]),
    EffectDef('Fruit Basket', 86, [hexColor(0xFF0066), hexColor(0xFF4400), hexColor(0xFF8800), hexColor(0xFFCC00)]),
  ];

  // Sound-reactive effects (87-91)
  static final soundEffects = [
    EffectDef('Sound Pulse', 87, [hexColor(0xFF0088), hexColor(0xFF00FF), hexColor(0x8800FF), hexColor(0x0088FF), hexColor(0x00FFFF)]),
    EffectDef('Sound Strobe', 88, [hexColor(0xFFFFFF), hexColor(0x000000)]),
    EffectDef('Sound Fire', 89, [hexColor(0xFF2200), hexColor(0xFF4400), hexColor(0xFF8800), hexColor(0xFFCC00), hexColor(0xFFFF88)]),
    EffectDef('Sound Waves', 90, [hexColor(0x001144), hexColor(0x003388), hexColor(0x0066CC), hexColor(0x00AAFF), hexColor(0x88DDFF)]),
    EffectDef('Sound Rainbow', 91, [hexColor(0xFF0000), hexColor(0xFF8800), hexColor(0xFFFF00), hexColor(0x00FF00), hexColor(0x00FFFF), hexColor(0x0000FF), hexColor(0xFF00FF)]),
  ];

  // Special effects (95-99)
  static final specialEffects = [
    EffectDef('Water', 95, [hexColor(0x0044AA), hexColor(0x0088FF), hexColor(0x00CCFF), hexColor(0xFFFFFF)]),
    EffectDef('Spectrum', 99, [hexColor(0xFF0000), hexColor(0x00FF00), hexColor(0x0000FF)]),
  ];

  @override
  void initState() {
    super.initState();
    _requestPermissions();
  }

  Future<void> _requestPermissions() async {
    await [
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.location,
    ].request();
  }

  Future<void> _scanAndConnect() async {
    if (_isScanning) return;

    setState(() {
      _isScanning = true;
      _status = 'Checking Bluetooth...';
    });

    try {
      // Request permissions first (triggers iOS dialog)
      await [
        Permission.bluetoothScan,
        Permission.bluetoothConnect,
      ].request();

      // Wait for Bluetooth adapter to be ready (skip unknown state)
      final adapterState = await FlutterBluePlus.adapterState
          .where((state) => state != BluetoothAdapterState.unknown)
          .first
          .timeout(const Duration(seconds: 5), onTimeout: () => BluetoothAdapterState.off);

      if (adapterState != BluetoothAdapterState.on) {
        setState(() {
          _status = 'Please turn on Bluetooth';
          _isScanning = false;
        });
        // Try to turn on Bluetooth (Android only)
        try { await FlutterBluePlus.turnOn(); } catch (_) {}
        return;
      }

      setState(() => _status = 'Scanning...');

      // Start scanning
      await FlutterBluePlus.startScan(timeout: const Duration(seconds: 10));

      // Listen for scan results
      await for (final results in FlutterBluePlus.scanResults) {
        for (final result in results) {
          final name = result.device.platformName;
          if (name.contains('OpenHoop') || name.contains('Hoop')) {
            await FlutterBluePlus.stopScan();
            await _connectToDevice(result.device);
            return;
          }
        }
      }

      setState(() {
        _status = 'No hoop found';
        _isScanning = false;
      });
    } catch (e) {
      setState(() {
        _status = 'Scan error: $e';
        _isScanning = false;
      });
    }
  }

  Future<void> _connectToDevice(BluetoothDevice device) async {
    setState(() => _status = 'Connecting...');

    try {
      await device.connect(timeout: const Duration(seconds: 10));
      _device = device;

      // Listen for disconnection
      device.connectionState.listen((state) {
        if (state == BluetoothConnectionState.disconnected) {
          _imuSubscription?.cancel();
          setState(() {
            _isConnected = false;
            _status = 'Disconnected';
            _effectChar = null;
            _colorChar = null;
            _energyChar = null;
            _batteryChar = null;
            _imuChar = null;
          });
        }
      });

      setState(() => _status = 'Discovering services...');
      final services = await device.discoverServices();

      // Find our characteristics
      for (final service in services) {
        if (service.uuid == BleUuids.service) {
          for (final char in service.characteristics) {
            if (char.uuid == BleUuids.effect) _effectChar = char;
            if (char.uuid == BleUuids.color) _colorChar = char;
            if (char.uuid == BleUuids.energy) _energyChar = char;
            if (char.uuid == BleUuids.imu) {
              _imuChar = char;
              // Subscribe to IMU notifications
              await char.setNotifyValue(true);
              _imuSubscription = char.onValueReceived.listen(_onImuData);
            }
          }
        }
        if (service.uuid == BleUuids.batteryService) {
          for (final char in service.characteristics) {
            if (char.uuid == BleUuids.batteryLevel) {
              _batteryChar = char;
              // Read initial battery level
              try {
                final value = await char.read();
                if (value.isNotEmpty) {
                  setState(() => _batteryLevel = value[0]);
                }
              } catch (_) {}
            }
          }
        }
      }

      if (_effectChar != null) {
        setState(() {
          _isConnected = true;
          _isScanning = false;
          _status = 'Connected to ${device.platformName}';
        });
      } else {
        setState(() {
          _status = 'OpenHoop service not found';
          _isScanning = false;
        });
        await device.disconnect();
      }
    } catch (e) {
      setState(() {
        _status = 'Connection error: $e';
        _isScanning = false;
      });
    }
  }

  void _onImuData(List<int> data) {
    if (data.length < 12) return;

    final bytes = ByteData.sublistView(Uint8List.fromList(data));
    setState(() {
      // Euler angles from Madgwick filter (already fused/filtered)
      _roll = bytes.getInt16(0, Endian.little) / 100.0;   // degrees
      _pitch = bytes.getInt16(2, Endian.little) / 100.0;  // degrees
      _yaw = bytes.getInt16(4, Endian.little) / 100.0;    // degrees

      // Raw accelerometer (for display and effects)
      _accelX = bytes.getInt16(6, Endian.little) / 1000.0;  // g
      _accelY = bytes.getInt16(8, Endian.little) / 1000.0;  // g
      _accelZ = bytes.getInt16(10, Endian.little) / 1000.0; // g

      // Light smoothing for visualization (Madgwick output is already stable)
      _smoothRoll += (_roll - _smoothRoll) * _smoothing;
      _smoothPitch += (_pitch - _smoothPitch) * _smoothing;
      _smoothYaw += (_yaw - _smoothYaw) * _smoothing;
    });
  }

  Future<void> _disconnect() async {
    _imuSubscription?.cancel();
    await _device?.disconnect();
    setState(() {
      _isConnected = false;
      _status = 'Disconnected';
    });
  }

  Future<void> _sendEffect(int effectId) async {
    if (_effectChar == null) return;
    try {
      await _effectChar!.write([effectId], withoutResponse: false);
    } catch (e) {
      setState(() => _status = 'Write error: $e');
    }
  }

  Future<void> _sendBrightness(int level) async {
    if (_energyChar == null) return;
    try {
      await _energyChar!.write([level], withoutResponse: false);
      setState(() => _brightnessLevel = level);
    } catch (e) {
      setState(() => _status = 'Write error: $e');
    }
  }

  void _startCalibration() {
    _sendEffect(96);
    setState(() => _isCalibrating = true);
    showModalBottomSheet(
      context: context,
      backgroundColor: const Color(0xFF1A1A2E),
      isScrollControlled: true,
      builder: (context) => _buildCalibrationSheet(),
    ).whenComplete(() {
      setState(() => _isCalibrating = false);
    });
  }

  Widget _buildCalibrationSheet() {
    return DraggableScrollableSheet(
      initialChildSize: 0.7,
      minChildSize: 0.5,
      maxChildSize: 0.9,
      expand: false,
      builder: (context, scrollController) => SingleChildScrollView(
        controller: scrollController,
        padding: const EdgeInsets.all(24),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Center(
              child: Container(
                width: 40,
                height: 4,
                margin: const EdgeInsets.only(bottom: 20),
                decoration: BoxDecoration(
                  color: Colors.white24,
                  borderRadius: BorderRadius.circular(2),
                ),
              ),
            ),
            const Text(
              'Calibration',
              style: TextStyle(fontSize: 24, fontWeight: FontWeight.bold, color: Colors.white),
            ),
            const SizedBox(height: 8),
            const Text(
              'Follow the LED colors on your hoop:',
              style: TextStyle(fontSize: 14, color: Colors.white70),
            ),
            const SizedBox(height: 24),
            _buildCalibrationStep(
              color: Colors.cyan,
              step: '1',
              title: 'Lay Flat',
              description: 'Place the hoop flat on the ground and hold still.',
            ),
            _buildCalibrationStep(
              color: Colors.yellow,
              step: '2',
              title: 'Pick It Up',
              description: 'Lift and tilt the hoop at least 45° from flat.',
            ),
            _buildCalibrationStep(
              color: Colors.purple,
              step: '3',
              title: 'Arduino at Bottom',
              description: 'Rotate so the Arduino/battery is at the bottom (6 o\'clock). Tilt at least 66° from horizontal.',
            ),
            _buildCalibrationStep(
              color: Colors.blue,
              step: '4',
              title: 'Arduino at Top',
              description: 'Flip the hoop so Arduino is at the top (12 o\'clock). Hold still.',
            ),
            _buildCalibrationStep(
              color: Colors.green,
              step: '✓',
              title: 'Done!',
              description: 'Calibration saved. Your hoop will return to normal effects.',
            ),
            const SizedBox(height: 24),
            SizedBox(
              width: double.infinity,
              child: ElevatedButton(
                onPressed: () => Navigator.pop(context),
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.white10,
                  padding: const EdgeInsets.symmetric(vertical: 16),
                ),
                child: const Text('Close'),
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildCalibrationStep({
    required Color color,
    required String step,
    required String title,
    required String description,
  }) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 16),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Container(
            width: 36,
            height: 36,
            decoration: BoxDecoration(
              color: color,
              borderRadius: BorderRadius.circular(18),
            ),
            child: Center(
              child: Text(
                step,
                style: TextStyle(
                  color: color.computeLuminance() > 0.5 ? Colors.black : Colors.white,
                  fontWeight: FontWeight.bold,
                ),
              ),
            ),
          ),
          const SizedBox(width: 12),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  title,
                  style: const TextStyle(
                    fontSize: 16,
                    fontWeight: FontWeight.bold,
                    color: Colors.white,
                  ),
                ),
                const SizedBox(height: 4),
                Text(
                  description,
                  style: const TextStyle(fontSize: 14, color: Colors.white70),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildColorPreview(List<Color> colors) {
    return Container(
      height: 8,
      margin: const EdgeInsets.only(top: 4),
      decoration: BoxDecoration(
        borderRadius: BorderRadius.circular(4),
        gradient: LinearGradient(colors: colors),
      ),
    );
  }

  Widget _buildEffectButton(EffectDef effect) {
    return Padding(
      padding: const EdgeInsets.all(4),
      child: SizedBox(
        width: 100,
        child: ElevatedButton(
          style: ElevatedButton.styleFrom(
            backgroundColor: Colors.white10,
            foregroundColor: Colors.white,
            padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 8),
            shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(8)),
          ),
          onPressed: _isConnected ? () => _sendEffect(effect.id) : null,
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              Text(effect.name, textAlign: TextAlign.center, style: const TextStyle(fontSize: 12)),
              _buildColorPreview(effect.colors),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildEffectSection(String title, List<EffectDef> effects) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Padding(
          padding: const EdgeInsets.symmetric(vertical: 8),
          child: Text(
            title,
            style: const TextStyle(
              fontSize: 16,
              fontWeight: FontWeight.bold,
              color: Colors.white70,
            ),
          ),
        ),
        Wrap(
          children: effects.map(_buildEffectButton).toList(),
        ),
      ],
    );
  }

  Widget _buildImuVisualization() {
    // Use Euler angles from Madgwick filter (converted to radians for painter)
    final pitchRad = _smoothPitch * pi / 180.0;
    final rollRad = _smoothRoll * pi / 180.0;

    return Card(
      color: Colors.white10,
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              'Orientation (Madgwick)',
              style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold, color: Colors.white70),
            ),
            const SizedBox(height: 12),
            Row(
              children: [
                // 3D-ish hoop visualization
                Expanded(
                  child: AspectRatio(
                    aspectRatio: 1,
                    child: CustomPaint(
                      painter: HoopPainter(pitch: pitchRad, roll: rollRad, yaw: _smoothYaw),
                    ),
                  ),
                ),
                const SizedBox(width: 16),
                // Data readout - Euler angles and raw accel
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      _imuRow('Roll', _roll, '°'),
                      _imuRow('Pitch', _pitch, '°'),
                      _imuRow('Yaw', _yaw, '°'),
                      const Divider(color: Colors.white24),
                      _imuRow('Accel X', _accelX, 'g'),
                      _imuRow('Accel Y', _accelY, 'g'),
                      _imuRow('Accel Z', _accelZ, 'g'),
                    ],
                  ),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }

  Widget _imuRow(String label, double value, String unit) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 2),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(label, style: const TextStyle(color: Colors.white54, fontSize: 12)),
          Text('${value.toStringAsFixed(2)} $unit', style: const TextStyle(color: Colors.white, fontSize: 12)),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('OpenHoop'),
        backgroundColor: Colors.transparent,
        elevation: 0,
        actions: [
          if (_batteryLevel >= 0)
            Padding(
              padding: const EdgeInsets.all(16),
              child: Row(
                children: [
                  Icon(
                    _batteryLevel > 20 ? Icons.battery_full : Icons.battery_alert,
                    color: _batteryLevel > 20 ? Colors.green : Colors.red,
                  ),
                  Text('$_batteryLevel%'),
                ],
              ),
            ),
        ],
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            // Connection status
            Card(
              color: Colors.white10,
              child: Padding(
                padding: const EdgeInsets.all(16),
                child: Column(
                  children: [
                    Text(
                      _status,
                      style: TextStyle(
                        fontSize: 18,
                        color: _isConnected ? Colors.greenAccent : Colors.white70,
                      ),
                    ),
                    const SizedBox(height: 12),
                    Row(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: [
                        ElevatedButton.icon(
                          icon: Icon(_isConnected ? Icons.bluetooth_disabled : Icons.bluetooth_searching),
                          label: Text(_isConnected ? 'Disconnect' : (_isScanning ? 'Scanning...' : 'Connect')),
                          style: ElevatedButton.styleFrom(
                            backgroundColor: _isConnected ? Colors.red : Colors.blue,
                            padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 12),
                          ),
                          onPressed: _isScanning ? null : (_isConnected ? _disconnect : _scanAndConnect),
                        ),
                        if (_isConnected) ...[
                          const SizedBox(width: 12),
                          ElevatedButton.icon(
                            icon: const Icon(Icons.power_settings_new),
                            label: const Text('Off'),
                            style: ElevatedButton.styleFrom(
                              backgroundColor: Colors.grey[800],
                              padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
                            ),
                            onPressed: () => _sendEffect(0),
                          ),
                          const SizedBox(width: 12),
                          ElevatedButton.icon(
                            icon: const Icon(Icons.tune),
                            label: const Text('Calibrate'),
                            style: ElevatedButton.styleFrom(
                              backgroundColor: Colors.teal[700],
                              padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
                            ),
                            onPressed: _startCalibration,
                          ),
                        ],
                      ],
                    ),
                  ],
                ),
              ),
            ),

            const SizedBox(height: 16),

            // IMU Visualization (only when connected)
            if (_isConnected) ...[
              _buildImuVisualization(),
              const SizedBox(height: 16),
            ],

            // Brightness slider
            Card(
              color: Colors.white10,
              child: Padding(
                padding: const EdgeInsets.all(16),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      'Brightness: ${brightnessPercents[_brightnessLevel]}%',
                      style: const TextStyle(fontSize: 16, color: Colors.white70),
                    ),
                    Slider(
                      value: _brightnessLevel.toDouble(),
                      min: 0,
                      max: 8,
                      divisions: 8,
                      label: '${brightnessPercents[_brightnessLevel]}%',
                      onChanged: _isConnected
                          ? (value) => _sendBrightness(value.round())
                          : null,
                    ),
                  ],
                ),
              ),
            ),

            const SizedBox(height: 16),

            // Effect buttons
            _buildEffectSection('Classic', classicEffects),
            _buildEffectSection('Patterns', patternEffects),
            _buildEffectSection('Sound Reactive', soundEffects),
            _buildEffectSection('Special', specialEffects),
          ],
        ),
      ),
    );
  }

  @override
  void dispose() {
    _imuSubscription?.cancel();
    _device?.disconnect();
    super.dispose();
  }
}

// Custom painter for hoop visualization with Madgwick-filtered orientation
class HoopPainter extends CustomPainter {
  final double pitch; // radians
  final double roll;  // radians
  final double yaw;   // degrees (for display)

  HoopPainter({required this.pitch, required this.roll, this.yaw = 0});

  @override
  void paint(Canvas canvas, Size size) {
    final center = Offset(size.width / 2, size.height / 2);
    final radius = size.width * 0.4;

    // Draw outer ring (the hoop) with gradient to show orientation
    final ringPaint = Paint()
      ..style = PaintingStyle.stroke
      ..strokeWidth = 8;

    // Apply perspective based on pitch/roll from Madgwick filter
    final scaleY = cos(pitch).abs().clamp(0.3, 1.0);
    final scaleX = cos(roll).abs().clamp(0.3, 1.0);

    canvas.save();
    canvas.translate(center.dx, center.dy);

    // Rotate based on yaw for heading reference
    canvas.rotate(yaw * pi / 180.0);
    canvas.scale(scaleX, scaleY);

    // Draw hoop with purple gradient
    ringPaint.color = Colors.purple;
    canvas.drawCircle(Offset.zero, radius, ringPaint);

    // Draw "down" indicator (where gravity points based on roll)
    final downAngle = roll;
    final downX = sin(downAngle) * radius * 0.7;
    final downY = cos(downAngle) * radius * 0.7;

    final downPaint = Paint()
      ..color = Colors.cyan
      ..style = PaintingStyle.fill;
    canvas.drawCircle(Offset(downX, downY), 10, downPaint);

    // Draw Arduino marker (fixed position on hoop - top)
    final markerPaint = Paint()
      ..color = Colors.green
      ..style = PaintingStyle.fill;
    canvas.drawCircle(Offset(0, -radius), 6, markerPaint);

    // Draw small ticks around the hoop for orientation reference
    final tickPaint = Paint()
      ..color = Colors.white24
      ..style = PaintingStyle.stroke
      ..strokeWidth = 2;
    for (int i = 0; i < 8; i++) {
      final angle = i * pi / 4;
      final innerR = radius * 0.85;
      final outerR = radius * 0.95;
      canvas.drawLine(
        Offset(cos(angle) * innerR, sin(angle) * innerR),
        Offset(cos(angle) * outerR, sin(angle) * outerR),
        tickPaint,
      );
    }

    canvas.restore();

    // Draw Euler angle labels
    final textStyle = const TextStyle(color: Colors.white54, fontSize: 10);
    final textPainter = TextPainter(textDirection: TextDirection.ltr);

    textPainter.text = TextSpan(text: 'P: ${(pitch * 180 / pi).toStringAsFixed(0)}°', style: textStyle);
    textPainter.layout();
    textPainter.paint(canvas, Offset(4, size.height - 36));

    textPainter.text = TextSpan(text: 'R: ${(roll * 180 / pi).toStringAsFixed(0)}°', style: textStyle);
    textPainter.layout();
    textPainter.paint(canvas, Offset(4, size.height - 24));

    textPainter.text = TextSpan(text: 'Y: ${yaw.toStringAsFixed(0)}°', style: textStyle);
    textPainter.layout();
    textPainter.paint(canvas, Offset(4, size.height - 12));
  }

  @override
  bool shouldRepaint(HoopPainter oldDelegate) {
    return oldDelegate.pitch != pitch || oldDelegate.roll != roll || oldDelegate.yaw != yaw;
  }
}
