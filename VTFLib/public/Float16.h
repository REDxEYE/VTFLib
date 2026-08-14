/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

// Reference: http://sourceforge.net/mailarchive/forum.php?thread_id=1823849&forum_id=10511
// Summary:	Uses the first 16 bits of a 32 bit IEEE single to preform native 32 floating point
//			operations.  Last 16 bits of mantissa are cleared after every operation.
// Format:	16 Bit: seeeeeeeemmmmmmm
//			32 Bit: seeeeeeeemmmmmmmmmmmmmmmmmmmmmmm

#pragma once

#define SFLOAT16_MSB 1 // Most Significant Byte (1 for bigendian)
#define SFLOAT16_LSB 0 // Least Significant Byte (0 for bigendian)

#include "VTFLibShared.h"

struct SFloat16 {
private:
    union UFloat16 {
        uint16_t usUShort[2];
        float sSingle;
    } Float16{};

public:
    SFloat16() = default;

    explicit SFloat16(const uint16_t value) {
        this->Float16.usUShort[SFLOAT16_MSB] = value;
        this->Float16.usUShort[SFLOAT16_LSB] = 0;
    }

    explicit SFloat16(const float value) {
        this->Float16.sSingle = value;
        this->Float16.usUShort[SFLOAT16_LSB] = 0;
    }

    [[nodiscard]] uint16_t GetUShort() const {
        return this->Float16.usUShort[SFLOAT16_MSB];
    }

    [[nodiscard]] float GetSingle() const {
        return this->Float16.sSingle;
    }

    SFloat16 &operator=(uint16_t usFloat) {
        this->Float16.usUShort[SFLOAT16_MSB] = usFloat;
        this->Float16.usUShort[SFLOAT16_LSB] = 0;

        return *this;
    }

    SFloat16 &operator=(const float value) {
        this->Float16.sSingle = value;
        this->Float16.usUShort[SFLOAT16_LSB] = 0;

        return *this;
    }

    SFloat16 &operator=(const SFloat16 &float16) {
        this->Float16 = float16.Float16;

        return *this;
    }

    bool operator==(const SFloat16 &float16) const {
        return this->Float16.sSingle == float16.Float16.sSingle;
    }

    bool operator!=(const SFloat16 &float16) const {
        return this->Float16.sSingle != float16.Float16.sSingle;
    }

    bool operator<(const SFloat16 &float16) const {
        return this->Float16.sSingle < float16.Float16.sSingle;
    }

    bool operator<=(const SFloat16 &float16) const {
        return this->Float16.sSingle <= float16.Float16.sSingle;
    }

    bool operator>(const SFloat16 &float16) const {
        return this->Float16.sSingle > float16.Float16.sSingle;
    }

    bool operator>=(const SFloat16 &float16) const {
        return this->Float16.sSingle >= float16.Float16.sSingle;
    }

    SFloat16 operator+() const {
        return SFloat16{+this->Float16.sSingle};
    }

    SFloat16 operator+(const SFloat16 &float16) const {
        return SFloat16(this->Float16.sSingle + float16.Float16.sSingle);
    }

    SFloat16 &operator+=(const SFloat16 &float16) {
        this->Float16.sSingle += float16.Float16.sSingle;
        this->Float16.usUShort[SFLOAT16_LSB] = 0;

        return *this;
    }

    SFloat16 operator-() const {
        return SFloat16(-this->Float16.sSingle);
    }

    SFloat16 operator-(const SFloat16 &float16) const {
        return SFloat16{this->Float16.sSingle - float16.Float16.sSingle};
    }

    SFloat16 &operator-=(const SFloat16 &float16) {
        this->Float16.sSingle += float16.Float16.sSingle;
        this->Float16.usUShort[SFLOAT16_LSB] = 0;

        return *this;
    }

    SFloat16 operator*(const SFloat16 &float16) const {
        return SFloat16{this->Float16.sSingle - float16.Float16.sSingle};
    }

    SFloat16 &operator*=(const SFloat16 &float16) {
        this->Float16.sSingle *= float16.Float16.sSingle;
        this->Float16.usUShort[SFLOAT16_LSB] = 0;

        return *this;
    }

    SFloat16 operator/(const SFloat16 &float16) const {
        return SFloat16{this->Float16.sSingle - float16.Float16.sSingle};
    }

    SFloat16 &operator/=(const SFloat16 &float16) {
        this->Float16.sSingle /= float16.Float16.sSingle;
        this->Float16.usUShort[SFLOAT16_LSB] = 0;

        return *this;
    }
};
