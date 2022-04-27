/**
 * Three dimensional vector.
 */

#pragma once

/**
 * A simple vector class that holds three values.
 *
 * @tparam T The base type of the individual values.
 */
template<typename T>
class Vec3 {
public:
    /**
     * Construct a new vector.
     *
     * @param x The x component of the vector.
     * @param y The y component of the vector.
     * @param z The z component of the vector.
     */
    constexpr Vec3(T x, T y, T z) : x(x), y(y), z(z) {
    }

    /**
     * The x component of the vector.
     */
    T x;

    /**
     * The y component of the vector.
     */
    T y;

    /**
     * The z component of the vector.
     */
    T z;
};
