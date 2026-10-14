#pragma once

#include "Simulation.h"

class CarromView {
public:
    CarromView(int windowSize, float boardSize);

    void drawFrame(const Simulation& sim) const;

private:
    void drawBoard(const Board& board)                      const;
    void drawPockets(const std::array<Pocket, 4>& pockets)  const;
    void drawPieces(const std::vector<Circle2D>& pieces)    const;

    [[nodiscard]] Vector2 toScreen(Vec2 p) const;
    [[nodiscard]] static Color colorOf(PieceType type);

    float m_windowSize;
    float m_boardSize;
    float m_padding;

    // Board theme
    float m_centerCircleR;
    float m_pieceRingInner;
    float m_pieceRingOuter;
};
