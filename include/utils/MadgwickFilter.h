/**
 * @project OpenHoop
 * @file MadgwickFilter.h
 * @brief Madgwick AHRS (Attitude and Heading Reference System) filter.
 *
 * Fuses accelerometer and gyroscope data to produce stable orientation.
 * Optionally can include magnetometer for absolute heading.
 *
 * Based on Sebastian Madgwick's algorithm:
 * https://x-io.co.uk/open-source-imu-and-ahrs-algorithms/
 */

#ifndef OPENHOOP_MADGWICKFILTER_H
#define OPENHOOP_MADGWICKFILTER_H

#include <Arduino.h>
#include <math.h>

class MadgwickFilter {
public:
    /**
     * @brief Initialize the filter with default beta (0.1).
     * @param sampleFreq Expected update frequency in Hz (default 100)
     */
    MadgwickFilter(float sampleFreq = 100.0f);

    /**
     * @brief Set filter gain (beta).
     * Higher = faster convergence but more noise.
     * Lower = smoother but slower response.
     * Typical values: 0.01 (very smooth) to 0.5 (responsive)
     * @param beta Filter gain
     */
    void setBeta(float beta);

    /**
     * @brief Set sample frequency for proper integration.
     * @param freq Sample frequency in Hz
     */
    void setSampleFreq(float freq);

    /**
     * @brief Update filter with accelerometer and gyroscope data.
     * Call this at your sample rate (e.g., 100Hz).
     *
     * @param gx Gyroscope X in rad/s
     * @param gy Gyroscope Y in rad/s
     * @param gz Gyroscope Z in rad/s
     * @param ax Accelerometer X (any unit, will be normalized)
     * @param ay Accelerometer Y
     * @param az Accelerometer Z
     */
    void update(float gx, float gy, float gz, float ax, float ay, float az);

    /**
     * @brief Update filter with accelerometer, gyroscope, AND magnetometer.
     * Provides absolute heading reference.
     *
     * @param gx Gyroscope X in rad/s
     * @param gy Gyroscope Y in rad/s
     * @param gz Gyroscope Z in rad/s
     * @param ax Accelerometer X
     * @param ay Accelerometer Y
     * @param az Accelerometer Z
     * @param mx Magnetometer X
     * @param my Magnetometer Y
     * @param mz Magnetometer Z
     */
    void update(float gx, float gy, float gz,
                float ax, float ay, float az,
                float mx, float my, float mz);

    /**
     * @brief Get quaternion components.
     */
    float getQ0() const { return q0; }
    float getQ1() const { return q1; }
    float getQ2() const { return q2; }
    float getQ3() const { return q3; }

    /**
     * @brief Get roll angle (rotation around X axis).
     * @return Roll in degrees (-180 to 180)
     */
    float getRoll() const;

    /**
     * @brief Get pitch angle (rotation around Y axis).
     * @return Pitch in degrees (-90 to 90)
     */
    float getPitch() const;

    /**
     * @brief Get yaw angle (rotation around Z axis).
     * Note: Without magnetometer, yaw will drift over time.
     * @return Yaw in degrees (-180 to 180)
     */
    float getYaw() const;

    /**
     * @brief Get the gravity vector in body frame.
     * Useful for determining "down" direction relative to the hoop.
     * @param gx Output gravity X component
     * @param gy Output gravity Y component
     * @param gz Output gravity Z component
     */
    void getGravityVector(float& gx, float& gy, float& gz) const;

    /**
     * @brief Get angle to "down" in the hoop plane.
     * This is what Water effect needs - angle around the hoop where gravity points.
     * @return Angle in degrees (0-360)
     */
    float getDownAngle() const;

    /**
     * @brief Reset filter to initial state.
     */
    void reset();

private:
    // Quaternion of sensor frame relative to earth frame
    float q0, q1, q2, q3;

    // Filter parameters
    float beta;           // Algorithm gain
    float sampleFreq;     // Sample frequency in Hz

    // Fast inverse square root (Quake III algorithm)
    static float invSqrt(float x);
};

#endif // OPENHOOP_MADGWICKFILTER_H
