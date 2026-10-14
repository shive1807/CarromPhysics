#pragma once

#include "Vec2.h"

struct Board {
    Vec2  center;
    float size;

    [[nodiscard]] float left()   const { return center.x - size * 0.5f; }
    [[nodiscard]] float right()  const { return center.x + size * 0.5f; }
    [[nodiscard]] float bottom() const { return center.y - size * 0.5f; }
    [[nodiscard]] float top()    const { return center.y + size * 0.5f; }
};
