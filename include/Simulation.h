#pragma once

#include "Board.h"
#include "Circle2D.h"

#include <vector>

class Simulation {
public:
    explicit Simulation(Board board, float friction = 0.985f);

    void addPiece(Circle2D piece);
    void step(float dt);

    [[nodiscard]] const Board&                  board()  const { return m_board; }
    [[nodiscard]] const std::vector<Circle2D>&  pieces() const { return m_pieces; }

private:
    void resolveWalls(Circle2D& c) const;

    Board                 m_board;
    std::vector<Circle2D> m_pieces;
    float                 m_friction;
};
