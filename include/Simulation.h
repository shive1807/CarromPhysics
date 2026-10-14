#pragma once

#include "Board.h"
#include "Circle2D.h"
#include "Pocket.h"

#include <array>
#include <vector>

class Simulation {
public:
    explicit Simulation(Board board, float friction = 0.985f);

    void addPiece(Circle2D piece);
    void step(float dt);

    [[nodiscard]] const Board&                  board()   const { return m_board; }
    [[nodiscard]] const std::vector<Circle2D>&  pieces()  const { return m_pieces; }
    [[nodiscard]] const std::array<Pocket, 4>&  pockets() const { return m_pockets; }

private:
    void resolveWalls(Circle2D& c) const;
    void resolveCircleCollisions();
    void checkPockets();

    static std::array<Pocket, 4> cornersOf(const Board& board, float pocketRadius);

    Board                 m_board;
    std::vector<Circle2D> m_pieces;
    std::array<Pocket, 4> m_pockets;
    float                 m_friction;
};
