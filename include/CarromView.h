#pragma once

#include "CarromModel.h"

class CarromView {
public:
    CarromView(int windowSize, float boardSize);

    void drawFrame(const CarromModel& model) const;
    void drawAimLine(Vec2 from, Vec2 to) const;

private:
    void drawBoard(const Board& board)                      const;
    void drawStrikerLines(const Board& board)               const;
    void drawPockets(const std::array<Pocket, 4>& pockets)  const;
    void drawPieces(const std::vector<Circle2D>& pieces)    const;
    void drawStriker(const Circle2D& striker)               const;

    float m_windowSize;
    float m_boardSize;
    float m_padding;

    float m_centerCircleR;
    float m_pieceRingInner;
    float m_pieceRingOuter;
};
