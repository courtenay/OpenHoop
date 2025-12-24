/**
 * @project OpenHoop
 * @file BleService.cpp
 * @brief Implementation of the BleService class for handling BLE communication.
 * @details This file contains the definitions of methods for initializing BLE services, advertising the HulaHoop device,
 * and updating battery level characteristics.
 * @author github.com/angelcamelot
 * @date 2024-03-17
 * @license Open-source license.
 */

#include "../../include/services/BleService.h"
#include "../../include/Config.h"

/**
 * @class BleService
 * @brief Handles BLE communication for the HulaHoop device.
 * @details This class initializes and configures BLE services and characteristics for the HulaHoop, including battery level,
 * effects control, solid color settings, energy-saving mode, and device information.
 */

/**
 * @brief Constructor for BleService class.
 * Initializes BLE services and characteristics.
 */
BleService::BleService():
        batteryService("180F"),
        batteryLevelCharacteristic("2A19", BLERead | BLENotify),
        effectCharacteristic(EFFECT_SERVICE_UUID, BLERead | BLEWrite),
        solidColorCharacteristic(SCOLOR_SERVICE_UUID, BLERead | BLEWrite, SOLID_COLOR_VALUE_SIZE),
        energySavingModeCharacteristic(ENERGY_SERVICE_UUID, BLERead | BLEWrite),
        imuCharacteristic(IMU_SERVICE_UUID, BLERead | BLENotify, 12),
        hulaHoopService("1812"),
        reportDescriptor("2908", "04 0B 00 0B 00 03 00 00 00 00 00 00 00 00 00 00 00 00 00 00"),
        reportMapCharacteristic("2A4B", BLERead | BLENotify),
        deviceInformationService("180A"),
        pnpIdCharacteristic("2A50", BLERead, 7),
        manufacturerCharacteristic("2A29", BLERead, 30),
        modelCharacteristic("2A24", BLERead, 20),
        serialNumberCharacteristic("2A25", BLERead, 15),
        firmwareCharacteristic("2A26", BLERead, 10),
        hulaHoopControlService("1815") {}

/**
 * @brief Begin BLE communication and advertise the HulaHoop device.
 */
bool BleService::beginAndAdvertise() {
    if (!BLE.begin()) {
        return false;
    }

    BLE.setDeviceName(MANUFACTURER_CHARACTERISTIC);
    BLE.setLocalName(MODEL_CHARACTERISTIC);
    BLE.setAppearance(BLE_DEVICE_APPEARANCE);
    BLE.setAdvertisedService(hulaHoopService);

    // Configure Report Map
    reportMapCharacteristic.addDescriptor(reportDescriptor);
    uint8_t reportMap[] = {
            0x05, 0x0C,       // Usage Page (Consumer Devices)
            0x09, 0x01,       // Usage (Battery Strength)
            0x15, 0x00,       // Logical Minimum (0)
            0x26, 0xFF, 0x00, // Logical Maximum (255)
            0x75, 0x08,       // Report Size (8 bits)
            0x95, 0x01,       // Report Count (1)
            0x81, 0x02        // Input (Data, Variable, Absolute)
    };
    for (uint8_t value : reportMap) {
        reportMapCharacteristic.writeValue(value);
    }
    hulaHoopService.addCharacteristic(reportMapCharacteristic);
    hulaHoopControlService.addCharacteristic(effectCharacteristic);
    effectCharacteristic.writeValue(DEFAULT_EFFECT_VALUE);
    hulaHoopControlService.addCharacteristic(solidColorCharacteristic);
    solidColorCharacteristic.writeValue(SOLID_COLOR_DEFAULT_VALUE);
    hulaHoopControlService.addCharacteristic(energySavingModeCharacteristic);
    energySavingModeCharacteristic.writeValue(DEFAULT_ENERGY_SAVING_MODE);
    hulaHoopControlService.addCharacteristic(imuCharacteristic);
    deviceInformationService.addCharacteristic(pnpIdCharacteristic);
    deviceInformationService.addCharacteristic(manufacturerCharacteristic);
    deviceInformationService.addCharacteristic(modelCharacteristic);
    deviceInformationService.addCharacteristic(serialNumberCharacteristic);
    deviceInformationService.addCharacteristic(firmwareCharacteristic);
    batteryLevelCharacteristic.writeValue(0);
    batteryService.addCharacteristic(batteryLevelCharacteristic);

    uint8_t pnpIdData[] = {
            0x01,             // Vendor ID (1)
            0x0D,             // Product ID (13)
            0x00, 0x00,       // Version Number (0.0)
            0x11, 0x22, 0x33  // Serial Number
    };
    pnpIdCharacteristic.writeValue(pnpIdData, sizeof(pnpIdData));

    manufacturerCharacteristic.writeValue(MANUFACTURER_CHARACTERISTIC);
    modelCharacteristic.writeValue(MODEL_CHARACTERISTIC);
    serialNumberCharacteristic.writeValue(SERIAL_NUMBER_CHARACTERISTIC);
    firmwareCharacteristic.writeValue(FIRMWARE_CHARACTERISTIC);

    BLE.addService(hulaHoopService);
    BLE.addService(hulaHoopControlService);
    BLE.addService(batteryService);
    BLE.addService(deviceInformationService);

    BLE.setAdvertisingInterval(BLE_ADVERTISING_INTERVAL);
    return BLE.advertise();
}

/**
 * @brief Update the battery level characteristic based on the provided voltage.
 * @param voltage Voltage level of the battery.
 */
void BleService::updateBatteryLevel(float voltage) {
    // Calculate the battery level and update the BLE characteristic
    const float clampedVoltage = voltage < BATTERY_MIN_VOLTAGE ? BATTERY_MIN_VOLTAGE : (voltage > BATTERY_MAX_VOLTAGE ? BATTERY_MAX_VOLTAGE : voltage);
    const float percent = (clampedVoltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE);
    const float boundedPercent = percent < 0.0f ? 0.0f : (percent > 1.0f ? 1.0f : percent);
    const uint8_t batteryLevel = static_cast<uint8_t>((boundedPercent * 100.0f) + 0.5f);

    static uint8_t lastReportedLevel = 0xFF;
    if (batteryLevel != lastReportedLevel) {
        batteryLevelCharacteristic.writeValue(batteryLevel);
        lastReportedLevel = batteryLevel;
    }
}

void BleService::resetControlCharacteristics() {
    effectCharacteristic.writeValue(DEFAULT_EFFECT_VALUE);
    solidColorCharacteristic.writeValue(SOLID_COLOR_DEFAULT_VALUE);
    energySavingModeCharacteristic.writeValue(DEFAULT_ENERGY_SAVING_MODE);
}

void BleService::updateIMUData(float roll, float pitch, float yaw, float ax, float ay, float az) {
    // Pack filtered orientation + raw accel as 6 x int16 (12 bytes total)
    // Orientation: scaled by 100 (180.0° = 18000)
    // Accelerometer: scaled by 1000 (1.0g = 1000)
    int16_t data[6];
    data[0] = static_cast<int16_t>(roll * 100.0f);   // Roll in degrees * 100
    data[1] = static_cast<int16_t>(pitch * 100.0f);  // Pitch in degrees * 100
    data[2] = static_cast<int16_t>(yaw * 100.0f);    // Yaw in degrees * 100
    data[3] = static_cast<int16_t>(ax * 1000.0f);    // Accel X in g * 1000
    data[4] = static_cast<int16_t>(ay * 1000.0f);    // Accel Y in g * 1000
    data[5] = static_cast<int16_t>(az * 1000.0f);    // Accel Z in g * 1000

    imuCharacteristic.writeValue(reinterpret_cast<uint8_t*>(data), 12);
}
