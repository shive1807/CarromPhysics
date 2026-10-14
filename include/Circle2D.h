#pragma once

#include "Vec2.h"
#include "PieceType.h"

struct Circle2D {
    Vec2      position;
    Vec2      velocity;
    float     radius = 1.0f;
    float     mass   = 1.0f;
    PieceType type   = PieceType::Black;
};
