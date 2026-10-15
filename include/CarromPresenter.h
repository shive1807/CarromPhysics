#pragma once

#include "CarromAI.h"
#include "CarromView.h"
#include "Simulation.h"

class CarromPresenter {
public:
    enum class Turn { Human, AI };

    CarromPresenter(Simulation& sim, CarromView& view, int windowSize, float boardSize,
                    Turn firstTurn = Turn::Human);

    void update(float dt);

    [[nodiscard]] Turn currentTurn() const { return m_turn; }

private:
    enum class State { Idle, AIThinking, AIMoveStriker, Aiming, DraggingSlider, Moving, GameOver };

    void handleInput();
    void handleAIThink(float dt);
    void handleAIMoveStriker(float dt);
    void resolveTurnEnd();
    void draw() const;

    Vec2 mouseToSim() const;
    bool isOnStriker(Vec2 simPos) const;
    bool isOnSliderHandle(float screenX, float screenY) const;
    bool allStopped() const;
    void switchTurn();

    Simulation& m_sim;
    CarromView& m_view;
    CarromAI    m_ai;

    float m_padding;
    float m_boardSize;

    Turn  m_turn;
    Turn  m_winner       = Turn::Human;
    State m_state        = State::Idle;
    bool  m_queenSecured = false;
    float m_aiTimer      = 0.0f;
    float m_aiTargetX    = 0.0f;
    Vec2  m_aiShootVel   = {};
    Vec2  m_dragOrigin;

    static constexpr float kAIThinkDelay    = 1.5f;
    static constexpr float kAIStrikerSpeed  = 400.0f;  // sim units per second
};
