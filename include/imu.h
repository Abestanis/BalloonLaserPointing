/**
 * IMU related functionality.
 */

#pragma once

/**
 * A 3 dimensional double vector.
 */
typedef struct {
    double x, y, z;
} Vec3D;


/**
 * Initialize the IMU.
 */
extern void initImu();

/**
 * Get the rotations in rpm from the gyroscope.
 *
 * @param rotations A vector that will be filled with the rotations in rpm.
 */
extern void getIMUGyro(Vec3D& rotations);
