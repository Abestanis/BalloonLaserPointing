/**
 * IMU related functionality.
 */

#pragma once

#include "types.h"


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
