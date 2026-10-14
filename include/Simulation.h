#pragma once

#include "CarromModel.h"

class Simulation {
public:
    explicit Simulation(Board board, float friction = 0.985f);

    void addPiece(Circle2D piece);
    void shoot(Vec2 velocity);
    void step(float dt);

    [[nodiscard]] const CarromModel& model() const { return m_model; }

private:
    void resolveWalls(Circle2D& c);
    void resolveCircleCollisions();
    void checkPockets();

    static void                   resolveCirclePair(Circle2D& a, Circle2D& b);
    static Circle2D               defaultStriker(const Board& board);
    static std::array<Pocket, 4>  cornersOf(const Board& board, float pocketRadius);

    CarromModel m_model;
    float       m_friction;
};
