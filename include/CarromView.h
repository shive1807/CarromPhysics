#pragma once

#include "CarromModel.h"

class CarromView {
public:
    CarromView(int windowSize, float boardSize);

    void beginFrame()                                                     const;
    void endFrame()                                                       const;
    void drawScene(const CarromModel& model, bool sliderActive = true)    const;
    void drawAimLine(Vec2 from, Vec2 to, float powerRadius)               const;

private:
    void drawBoard(const Board& board)                      const;
    void drawStrikerLines(const Board& board)               const;
    void drawPockets(const std::array<Pocket, 4>& pockets)  const;
    void drawPieces(const std::vector<Circle2D>& pieces)    const;
    void drawStriker(const Circle2D& striker)               const;
    void drawSlider(const Circle2D& striker, bool active)   const;

    float m_windowSize;
    float m_boardSize;
    float m_padding;

    float m_centerCircleR;
    float m_pieceRingInner;
    float m_pieceRingOuter;
};
