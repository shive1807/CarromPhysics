#pragma once

#include "CarromModel.h"
#include <vector>

struct CollisionEvent {
    Vec2  position;
    float impulse;
};

class Simulation {
public:
    explicit Simulation(Board board,
                        float friction        = 0.985f,
                        float staticThreshold = 10.0f);

    void addPiece(Circle2D piece);
    void shoot(Vec2 velocity);
    void setStrikerX(float x);
    void resetStriker(bool topSide = false);
    void returnPieceToCenter(PieceType type);
    void clearPottedThisTurn();
    void clearCollisionEvents();
    void step(float dt);

    [[nodiscard]] const CarromModel&              model()            const { return m_model; }
    [[nodiscard]] const std::vector<Circle2D>&    pottedThisTurn()   const { return m_pottedThisTurn; }
    [[nodiscard]] const std::vector<CollisionEvent>& collisionEvents() const { return m_collisionEvents; }

private:
    void resolveWalls(Circle2D& c);
    void resolveCircleCollisions();
    void applyFriction(Circle2D& c) const;
    void checkPockets();

    void                          resolveCirclePair(Circle2D& a, Circle2D& b);
    static Circle2D               defaultStriker(const Board& board);
    static std::array<Pocket, 4>  cornersOf(const Board& board, float pocketRadius);

    CarromModel              m_model;
    float                    m_friction;
    float                    m_staticThreshold;
    std::vector<Circle2D>    m_pottedThisTurn;
    std::vector<CollisionEvent> m_collisionEvents;
};
