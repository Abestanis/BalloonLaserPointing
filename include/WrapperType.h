#pragma once

/**
 * A wrapper around a base type.
 *
 * @tparam T The base type that this wrapper wraps.
 * @tparam W The actual wrapper type.
 */
template<typename T, class W>
class WrapperType {
public:
    /**
     * Create a new wrapper wrapping the given base value.
     *
     * @param value The actual value in the base type.
     */
    explicit constexpr WrapperType(T value) : value(value) {
    }

    /**
     * The wrapped value.
     */
    T value;

    constexpr W operator+(T other) const {
        return W(value + other);
    }

    constexpr W operator-(T other) const {
        return W(value - other);
    }

    constexpr W operator*(T other) const {
        return W(value * other);
    }

    constexpr W operator/(T other) const {
        return W(value / other);
    }

    constexpr W operator-() const {
        return W(-value);
    }

    constexpr bool operator<(T other) const {
        return value < other;
    }

    constexpr bool operator<=(T other) const {
        return value <= other;
    }

    constexpr bool operator>(T other) const {
        return value > other;
    }

    constexpr bool operator>=(T other) const {
        return value >= other;
    }

    constexpr bool operator==(T other) const {
        return value == other;
    }

    constexpr bool operator!=(T other) const {
        return value != other;
    }

    W& operator+=(T other) {
        value += other;
        return static_cast<W&>(*this);
    }

    W& operator-=(T other) {
        value -= other;
        return static_cast<W&>(*this);
    }

    W& operator*=(T other) {
        value *= other;
        return static_cast<W&>(*this);
    }

    W& operator/=(T other) {
        value /= other;
        return static_cast<W&>(*this);
    }
};
