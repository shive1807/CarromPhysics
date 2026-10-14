#pragma once

#include "Vec2.h"

struct Pocket {
    Vec2  position;
    float radius = 25.0f;

    [[nodiscard]] bool contains(Vec2 point) const {
        return (point - position).lengthSq() <= radius * radius;
    }
};
