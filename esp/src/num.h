/**
 * [num.h] Provides numeric type definitions and common
 * math functions.
 */
#pragma once

#include <stdint.h>
#include <stddef.h>

typedef size_t usize;
// 8-bit
typedef uint8_t u8;
typedef int8_t i8;
// 16-bit
typedef uint16_t u16;
typedef int16_t i16;
// 32-bit
typedef uint32_t u32;
typedef int32_t i32;
// 64-bit
typedef uint64_t u64;
typedef int64_t i64;
// Floating-point
typedef float f32;
typedef double f64;

// A 3-dimensional vector
struct vec3 {
    f64 x;
    f64 y;
    f64 z;

    vec3& operator=(const vec3& v) {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
        return *this;
    }
    vec3& operator+=(const vec3& v) {
        this->x += v.x;
        this->y += v.y;
        this->z += v.z;
        return *this;
    }
    vec3& operator-=(const vec3& v) {
        this->x -= v.x;
        this->y -= v.y;
        this->z -= v.z;
        return *this;
    }
    vec3& operator*=(const f32& s) {
        this->x *= s;
        this->y *= s;
        this->z *= s;
        return *this;
    }
    vec3& operator/=(const f32& s) {
        this->x /= s;
        this->y /= s;
        this->z /= s;
        return *this;
    }
    vec3 operator+(const vec3& v) const {
        return {
            .x = this->x + v.x,
            .y = this->y + v.y,
            .z = this->z + v.z,
        };
    }
    vec3 operator-(const vec3& v) const {
        return {
            .x = this->x - v.x,
            .y = this->y - v.y,
            .z = this->z - v.z,
        };
    }
    vec3 operator*(const f64& s) const {
        return {
            .x = this->x * s,
            .y = this->y * s,
            .z = this->z * s,
        };
    }
    vec3 operator/(const f64& s) const {
        return {
            .x = this->x / s,
            .y = this->y / s,
            .z = this->z / s,
        };
    }
};