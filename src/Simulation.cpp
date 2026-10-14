#include "Simulation.h"

#include <algorithm>

static constexpr float kPocketRadius = 25.0f;

Simulation::Simulation(Board board, float friction)
    : m_model{board, {}, cornersOf(board, kPocketRadius)}
    , m_friction(friction)
{}

std::array<Pocket, 4> Simulation::cornersOf(const Board& board, float r) {
    return {{
        {{board.left()  + r, board.top()    - r}, r},
        {{board.right() - r, board.top()    - r}, r},
        {{board.right() - r, board.bottom() + r}, r},
        {{board.left()  + r, board.bottom() + r}, r},
    }};
}

void Simulation::addPiece(Circle2D piece) {
    m_model.pieces.push_back(piece);
}

void Simulation::step(float dt) {
    for (auto& c : m_model.pieces) {
        c.position += c.velocity * dt;
        resolveWalls(c);
        c.velocity *= m_friction;
    }
    resolveCircleCollisions();
    checkPockets();
}

void Simulation::checkPockets() {
    m_model.pieces.erase(
        std::remove_if(m_model.pieces.begin(), m_model.pieces.end(), [&](const Circle2D& c) {
            for (const auto& p : m_model.pockets)
                if (p.contains(c.position)) return true;
            return false;
        }),
        m_model.pieces.end()
    );
}

void Simulation::resolveCircleCollisions() {
    auto& pieces = m_model.pieces;
    for (size_t i = 0; i < pieces.size(); ++i) {
        for (size_t j = i + 1; j < pieces.size(); ++j) {
            Circle2D& a = pieces[i];
            Circle2D& b = pieces[j];

            Vec2  delta   = b.position - a.position;
            float distSq  = delta.lengthSq();
            float minDist = a.radius + b.radius;

            if (distSq >= minDist * minDist)
                continue;

            float dist   = delta.length();
            Vec2  normal = dist > 0.0f ? delta * (1.0f / dist) : Vec2{1.0f, 0.0f};

            float overlap   = minDist - dist;
            float totalMass = a.mass + b.mass;
            a.position -= normal * (overlap * b.mass / totalMass);
            b.position += normal * (overlap * a.mass / totalMass);

            float relativeVel = (a.velocity - b.velocity).dot(normal);
            if (relativeVel >= 0.0f)
                continue;

            float impulse = 2.0f * relativeVel / totalMass;
            a.velocity   -= normal * (impulse * b.mass);
            b.velocity   += normal * (impulse * a.mass);
        }
    }
}

void Simulation::resolveWalls(Circle2D& c) {
    const Board& board = m_model.board;

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
