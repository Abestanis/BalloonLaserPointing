/**
 * This wraps the Arduino header and works around some problems when building in C++.
 * It should always be used instead of <Arduino.h>.
 *
 * @see https://github.com/kekyo/gcc-toolchain/issues/3
 */

#pragma once

#include <Arduino.h>

#undef max
#undef min
