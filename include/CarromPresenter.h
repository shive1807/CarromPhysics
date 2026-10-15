#pragma once

#include "CarromAI.h"
#include "CarromView.h"
#include "ParticleSystem.h"
#include "Simulation.h"
#include "SoundSystem.h"

class CarromPresenter {
public:
    enum class Turn { Human, AI };

    CarromPresenter(Simulation& sim, CarromView& view, int windowSize, float boardSize,
                    Turn firstTurn = Turn::Human);

    void update(float dt);

    [[nodiscard]] Turn currentTurn() const { return m_turn; }

private:
    enum class State { Idle, AIThinking, AIMoveStriker, AIAiming, Aiming, DraggingSlider, Moving, GameOver };

    void handleInput();
    void handleAIThink(float dt);
    void handleAIMoveStriker(float dt);
    void handleAIAim(float dt);
    void resolveTurnEnd();
    void emitParticles(float dt);
    void draw() const;

    Vec2 mouseToSim() const;
    bool isOnStriker(Vec2 simPos) const;
    bool isOnSliderHandle(float screenX, float screenY) const;
    bool allStopped() const;
    void switchTurn();

    Simulation&    m_sim;
    CarromView&    m_view;
    CarromAI       m_ai;
    SoundSystem    m_sound;
    ParticleSystem m_particles;
    float          m_soundCooldown = 0.0f;  // throttles rapid collision sounds

    float m_padding;
    float m_boardSize;

    Turn  m_turn;
    Turn  m_winner       = Turn::Human;
    State m_state        = State::Idle;
    bool  m_queenSecured = false;
    bool  m_showRules    = false;
    float m_aiTimer      = 0.0f;
    float m_aiAimTimer    = 0.0f;   // countdown: random pre-shot pause
    float m_aiAimDuration = 0.0f;   // initial value — used to compute charge fraction
    float m_aiTargetX    = 0.0f;
    Vec2  m_aiShootVel   = {};
    Vec2  m_dragOrigin;

    static constexpr float kAIThinkDelay    = 1.5f;
    static constexpr float kAIStrikerSpeed  = 400.0f;
    static constexpr float kAIAimDelayMin   = 1.0f;    // seconds between arriving and shooting
    static constexpr float kAIAimDelayMax   = 2.0f;
};
