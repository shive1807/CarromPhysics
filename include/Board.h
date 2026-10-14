#pragma once

#include "Circle2D.h"

struct Board {
    Vec2  center;
    float size;   // side length

    float left()   const { return center.x - size * 0.5f; }
    float right()  const { return center.x + size * 0.5f; }
    float bottom() const { return center.y - size * 0.5f; }
    float top()    const { return center.y + size * 0.5f; }
};
