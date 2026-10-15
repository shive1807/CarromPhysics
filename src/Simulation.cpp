#include "Simulation.h"

#include <algorithm>

static constexpr float kPocketRadius    = 25.0f;
static constexpr float kStrikerRadius   = 24.0f;
static constexpr float kStrikerMass     = 1.5f;
static constexpr float kStrikerEdgeOff  = 50.0f;  // matches CarromView kEdgeOffset
static constexpr float kStrikerLineGap  = 20.0f;  // matches CarromView kLineGap

Simulation::Simulation(Board board, float friction, float staticThreshold)
    : m_model{board, {}, cornersOf(board, kPocketRadius), defaultStriker(board)}
    , m_friction(friction)
    , m_staticThreshold(staticThreshold)
{}

Circle2D Simulation::defaultStriker(const Board& board) {
    return {
        {board.center.x, board.bottom() + kStrikerEdgeOff + kStrikerLineGap * 0.5f},
        {},
        kStrikerRadius,
        kStrikerMass,
        PieceType::Striker,
    };
}

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

void Simulation::shoot(Vec2 velocity) {
    m_model.striker.velocity = velocity;
}

void Simulation::resetStriker(bool topSide) {
    Circle2D s = defaultStriker(m_model.board);
    if (topSide)
        s.position.y = m_model.board.top() - kStrikerEdgeOff - kStrikerLineGap * 0.5f;
    m_model.striker = s;
}

void Simulation::setStrikerX(float x) {
    const float halfSpan = m_model.board.size * 0.5f - 90.0f;  // half-board minus corner inset
    m_model.striker.position.x = x < -halfSpan ? -halfSpan : (x > halfSpan ? halfSpan : x);
    m_model.striker.velocity   = {};
}

static constexpr int kCollisionIterations = 8;

void Simulation::step(float dt) {
    // 1. Integrate positions
    for (auto& c : m_model.pieces)
        c.position += c.velocity * dt;
    m_model.striker.position += m_model.striker.velocity * dt;

    // 2. Resolve walls
    for (auto& c : m_model.pieces)
        resolveWalls(c);
    resolveWalls(m_model.striker);

    // 3. Resolve circle collisions — multiple passes prevent sticking in dense clusters
    for (int i = 0; i < kCollisionIterations; ++i)
        resolveCircleCollisions();

    // 4. Apply friction after all constraints are resolved
    for (auto& c : m_model.pieces)
        applyFriction(c);
    applyFriction(m_model.striker);

    checkPockets();
}

void Simulation::resolveCircleCollisions() {
    auto& pieces  = m_model.pieces;
    auto& striker = m_model.striker;

    for (auto& piece : pieces)
        resolveCirclePair(striker, piece);

    for (size_t i = 0; i < pieces.size(); ++i)
        for (size_t j = i + 1; j < pieces.size(); ++j)
            resolveCirclePair(pieces[i], pieces[j]);
}

void Simulation::resolveCirclePair(Circle2D& a, Circle2D& b) {  // NOLINT(readability-make-member-function-const)
    Vec2  delta   = b.position - a.position;
    float distSq  = delta.lengthSq();
    float minDist = a.radius + b.radius;

    if (distSq >= minDist * minDist)
        return;

    float dist   = delta.length();
    Vec2  normal = dist > 0.0f ? delta * (1.0f / dist) : Vec2{1.0f, 0.0f};

    float overlap   = minDist - dist;
    float totalMass = a.mass + b.mass;
    a.position -= normal * (overlap * b.mass / totalMass);
    b.position += normal * (overlap * a.mass / totalMass);

    // normal points a→b, so relativeVel > 0 means approaching — apply impulse
    // relativeVel < 0 means already separating — skip
    float relativeVel = (a.velocity - b.velocity).dot(normal);
    if (relativeVel <= 0.0f)
        return;

    float impulse = 2.0f * relativeVel / totalMass;
    a.velocity   -= normal * (impulse * b.mass);
    b.velocity   += normal * (impulse * a.mass);

    static constexpr float kMinEventImpulse = 5.0f;
    if (impulse > kMinEventImpulse) {
        Vec2 mid = (a.position + b.position) * 0.5f;
        m_collisionEvents.push_back({mid, impulse});
    }
}

void Simulation::checkPockets() {
    auto& pieces = m_model.pieces;
    auto  it     = pieces.begin();
    while (it != pieces.end()) {
        bool potted = false;
        for (const auto& p : m_model.pockets) {
            if (p.contains(it->position)) {
                m_pottedThisTurn.push_back(*it);
                potted = true;
                break;
            }
        }
        it = potted ? pieces.erase(it) : std::next(it);
    }
}

void Simulation::clearPottedThisTurn() {
    m_pottedThisTurn.clear();
}

void Simulation::clearCollisionEvents() {
    m_collisionEvents.clear();
}

void Simulation::returnPieceToCenter(PieceType type) {
    static constexpr float kQueenRadius = 15.0f;
    static constexpr float kPieceRadius = 18.0f;
    float r = (type == PieceType::Queen) ? kQueenRadius : kPieceRadius;
    m_model.pieces.push_back({{0.0f, 0.0f}, {}, r, 1.0f, type});
}

void Simulation::applyFriction(Circle2D& c) const {
    c.velocity *= m_friction;
    if (c.velocity.lengthSq() < m_staticThreshold * m_staticThreshold)
        c.velocity = {};
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
