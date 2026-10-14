#pragma once

#include "Simulation.h"
#include "CarromView.h"

class CarromPresenter {
public:
    CarromPresenter(Simulation& sim, CarromView& view, int windowSize, float boardSize);

    void update(float dt);

private:
    enum class State { Idle, Aiming, Moving };

    void handleInput();
    void draw() const;

    Vec2 mouseToSim() const;
    bool isOnStriker(Vec2 simPos) const;
    bool allStopped() const;

    Simulation& m_sim;
    CarromView& m_view;
    float       m_padding;
    float       m_boardSize;

    State m_state = State::Idle;
    Vec2  m_dragOrigin;   // sim-space position where drag started
};
