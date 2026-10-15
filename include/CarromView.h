#pragma once

#include "CarromModel.h"
#include "ParticleSystem.h"

class CarromView {
public:
    CarromView(int windowSize, float boardSize);

    // Layout constants — exposed so the presenter can do hit-testing without duplication
    static constexpr float kRulesBtnCX = 672.0f;
    static constexpr float kRulesBtnCY = 22.0f;
    static constexpr float kRulesBtnR  = 14.0f;

    static constexpr float kPanelX     = 70.0f;
    static constexpr float kPanelY     = 55.0f;
    static constexpr float kPanelW     = 560.0f;
    static constexpr float kPanelH     = 575.0f;
    // Close "×" button sits at top-right corner of the panel
    static constexpr float kCloseBtnCX = kPanelX + kPanelW - 22.0f;
    static constexpr float kCloseBtnCY = kPanelY + 22.0f;
    static constexpr float kCloseBtnR  = 13.0f;

    void beginFrame()                                                      const;
    void endFrame()                                                        const;
    void drawBackground(const CarromModel& model)                          const;
    void drawForeground(const CarromModel& model, bool sliderActive = true) const;
    void drawParticles(const std::vector<Particle>& particles)             const;
    void drawAimLine(Vec2 from, Vec2 to, float powerRadius)                const;
    void drawRulesButton()                                                 const;
    void drawRulesPanel()                                                  const;

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
