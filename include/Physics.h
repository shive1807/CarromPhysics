#pragma once

#include "Circle2D.h"
#include "Board.h"

inline void resolveWallCollisions(Circle2D& c, const Board& board) {
    if (c.position.x - c.radius < board.left()) {
        c.position.x = board.left() + c.radius;
        c.velocity.x = -c.velocity.x;
    } else if (c.position.x + c.radius > board.right()) {
        c.position.x = board.right() - c.radius;
        c.velocity.x = -c.velocity.x;
    }

    if (c.position.y - c.radius < board.bottom()) {
        c.position.y = board.bottom() + c.radius;
        c.velocity.y = -c.velocity.y;
    } else if (c.position.y + c.radius > board.top()) {
        c.position.y = board.top() - c.radius;
        c.velocity.y = -c.velocity.y;
    }
}

inline void stepCircle(Circle2D& c, const Board& board, float dt, float friction = 0.98f) {
    c.position = c.position + c.velocity * dt;
    resolveWallCollisions(c, board);
    c.velocity = c.velocity * friction;
}
