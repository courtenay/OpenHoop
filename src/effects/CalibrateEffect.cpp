/**
 * @project OpenHoop
 * @file CalibrateEffect.cpp
 * @brief IMU calibration/diagnostic effect implementation.
 */

#include "../../include/effects/CalibrateEffect.h"
#include "../../include/Config.h"
#include "Arduino_BMI270_BMM150.h"

CalibrateEffect::CalibrateEffect() : lastPrintTime(0) {}

void CalibrateEffect::start() {
    lastPrintTime = 0;
    Serial.println("=== IMU Calibration Mode ===");
    Serial.println("Place hoop FLAT on ground for baseline.");
    Serial.println("LED sections show:");
    Serial.println("  RED section   = Accelerometer X");
    Serial.println("  GREEN section = Accelerometer Y");
    Serial.println("  BLUE section  = Accelerometer Z (gravity when flat)");
    Serial.println("  WHITE section = Gyroscope spin magnitude");
    Serial.println("");
}

void CalibrateEffect::update() {
    float ax, ay, az;  // Accelerometer (g-force, ~1.0 = gravity)
    float gx, gy, gz;  // Gyroscope (degrees/second)

    // Read sensors
    bool accelOk = IMU.readAcceleration(ax, ay, az);
    bool gyroOk = IMU.readGyroscope(gx, gy, gz);

    if (!accelOk || !gyroOk) {
        // Flash red if IMU read fails
        hoop.fill(HulaHoopDotStar::Color(255, 0, 0));
        hoop.show();
        return;
    }

    // Calculate spin magnitude from gyroscope
    float spinMagnitude = sqrt(gx * gx + gy * gy + gz * gz);

    // Print to serial every 500ms for debugging
    unsigned long now = millis();
    if (now - lastPrintTime > 500) {
        lastPrintTime = now;
        Serial.print("Accel: X=");
        Serial.print(ax, 2);
        Serial.print(" Y=");
        Serial.print(ay, 2);
        Serial.print(" Z=");
        Serial.print(az, 2);
        Serial.print(" | Gyro: X=");
        Serial.print(gx, 1);
        Serial.print(" Y=");
        Serial.print(gy, 1);
        Serial.print(" Z=");
        Serial.print(gz, 1);
        Serial.print(" | Spin=");
        Serial.println(spinMagnitude, 1);
    }

    // Divide LEDs into 4 equal sections
    int numLeds = hoop.getActivePixels();
    int sectionSize = numLeds / 4;

    // Map accelerometer values (-1 to +1 g) to brightness (0-255)
    // abs() because we care about magnitude, sign shows in which section is brightest
    uint8_t brightnessX = static_cast<uint8_t>(min(255.0f, abs(ax) * 255.0f));
    uint8_t brightnessY = static_cast<uint8_t>(min(255.0f, abs(ay) * 255.0f));
    uint8_t brightnessZ = static_cast<uint8_t>(min(255.0f, abs(az) * 255.0f));

    // Gyro: map 0-500 deg/s to 0-255 brightness
    uint8_t brightnessSpin = static_cast<uint8_t>(min(255.0f, spinMagnitude / 2.0f));

    // Section 1: RED = X axis
    for (int i = 0; i < sectionSize; i++) {
        // Add direction indicator: bright at one end if positive, other end if negative
        float posInSection = static_cast<float>(i) / sectionSize;
        uint8_t dirBrightness = (ax > 0) ?
            static_cast<uint8_t>(posInSection * brightnessX) :
            static_cast<uint8_t>((1.0f - posInSection) * brightnessX);
        hoop.setPixelColor(i, dirBrightness, 0, 0);
    }

    // Section 2: GREEN = Y axis
    for (int i = sectionSize; i < sectionSize * 2; i++) {
        float posInSection = static_cast<float>(i - sectionSize) / sectionSize;
        uint8_t dirBrightness = (ay > 0) ?
            static_cast<uint8_t>(posInSection * brightnessY) :
            static_cast<uint8_t>((1.0f - posInSection) * brightnessY);
        hoop.setPixelColor(i, 0, dirBrightness, 0);
    }

    // Section 3: BLUE = Z axis
    for (int i = sectionSize * 2; i < sectionSize * 3; i++) {
        float posInSection = static_cast<float>(i - sectionSize * 2) / sectionSize;
        uint8_t dirBrightness = (az > 0) ?
            static_cast<uint8_t>(posInSection * brightnessZ) :
            static_cast<uint8_t>((1.0f - posInSection) * brightnessZ);
        hoop.setPixelColor(i, 0, 0, dirBrightness);
    }

    // Section 4: WHITE = Spin magnitude (gyroscope)
    for (int i = sectionSize * 3; i < numLeds; i++) {
        hoop.setPixelColor(i, brightnessSpin, brightnessSpin, brightnessSpin);
    }

    hoop.show();
}

void CalibrateEffect::stop() {
    hoop.fill(HulaHoopDotStar::Color(0, 0, 0));
    Serial.println("=== Calibration Mode Ended ===");
}
