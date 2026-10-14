#pragma once

#include "CarromModel.h"

class CarromView {
public:
    CarromView(int windowSize, float boardSize);

    void drawFrame(const CarromModel& model) const;

private:
    void drawBoard(const Board& board)                      const;
    void drawPockets(const std::array<Pocket, 4>& pockets)  const;
    void drawPieces(const std::vector<Circle2D>& pieces)    const;

    float m_windowSize;
    float m_boardSize;
    float m_padding;

    float m_centerCircleR;
    float m_pieceRingInner;
    float m_pieceRingOuter;
};
