import 'dart:async';
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
  static final energy = Guid('00000a95-0000-1000-8000-00805f9b34fb');
  static final batteryService = Guid('0000180f-0000-1000-8000-00805f9b34fb');
  static final batteryLevel = Guid('00002a19-0000-1000-8000-00805f9b34fb');
}

// Effect definitions matching EffectService.h
class EffectDef {
  final String name;
  final int id;
  final Color color;

  const EffectDef(this.name, this.id, this.color);
}

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

  bool _isConnected = false;
  bool _isScanning = false;
  int _batteryLevel = -1;
  int _brightnessLevel = 4; // Default 50%
  String _status = 'Disconnected';

  // Brightness levels: 0=100%, 1=88%, 2=75%, 3=63%, 4=50%, 5=38%, 6=25%, 7=15%, 8=10%
  static const List<int> brightnessPercents = [100, 88, 75, 63, 50, 38, 25, 15, 10];

  // Classic effects (0-7)
  static const classicEffects = [
    EffectDef('Rainbow', 1, Colors.red),
    EffectDef('Color Wave', 2, Colors.blue),
    EffectDef('Funky', 3, Colors.purple),
    EffectDef('Rastafari', 4, Colors.green),
    EffectDef('Fire', 5, Colors.orange),
    EffectDef('Leopard', 6, Colors.amber),
    EffectDef('Mushroom', 7, Colors.brown),
  ];

  // Pattern effects (80-86)
  static const patternEffects = [
    EffectDef('Liquid Sugar', 80, Colors.cyan),
    EffectDef('Froth', 81, Colors.lightBlue),
    EffectDef('Rainbow Dash', 82, Colors.pink),
    EffectDef('Fireball', 83, Colors.deepOrange),
    EffectDef('Flamebow', 84, Colors.redAccent),
    EffectDef('OG Sparkles', 85, Colors.indigo),
    EffectDef('Fruit Basket', 86, Colors.orangeAccent),
  ];

  // Sound-reactive effects (87-91)
  static const soundEffects = [
    EffectDef('Sound Pulse', 87, Colors.pinkAccent),
    EffectDef('Sound Strobe', 88, Colors.white),
    EffectDef('Sound Fire', 89, Colors.deepOrange),
    EffectDef('Sound Waves', 90, Colors.blueAccent),
    EffectDef('Sound Rainbow', 91, Colors.purple),
  ];

  // Special effects (95-99)
  static const specialEffects = [
    EffectDef('Water', 95, Colors.teal),
    EffectDef('Spectrum', 99, Colors.deepPurple),
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
      _status = 'Scanning...';
    });

    try {
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
          setState(() {
            _isConnected = false;
            _status = 'Disconnected';
            _effectChar = null;
            _colorChar = null;
            _energyChar = null;
            _batteryChar = null;
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

  Future<void> _disconnect() async {
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

  Widget _buildEffectButton(EffectDef effect) {
    return Padding(
      padding: const EdgeInsets.all(4),
      child: ElevatedButton(
        style: ElevatedButton.styleFrom(
          backgroundColor: effect.color.withOpacity(0.3),
          foregroundColor: Colors.white,
          padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
        ),
        onPressed: _isConnected ? () => _sendEffect(effect.id) : null,
        child: Text(effect.name, textAlign: TextAlign.center),
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
                    ElevatedButton.icon(
                      icon: Icon(_isConnected ? Icons.bluetooth_disabled : Icons.bluetooth_searching),
                      label: Text(_isConnected ? 'Disconnect' : (_isScanning ? 'Scanning...' : 'Connect')),
                      style: ElevatedButton.styleFrom(
                        backgroundColor: _isConnected ? Colors.red : Colors.blue,
                        padding: const EdgeInsets.symmetric(horizontal: 32, vertical: 12),
                      ),
                      onPressed: _isScanning ? null : (_isConnected ? _disconnect : _scanAndConnect),
                    ),
                  ],
                ),
              ),
            ),

            const SizedBox(height: 16),

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
    _device?.disconnect();
    super.dispose();
  }
}
