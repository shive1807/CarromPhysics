#include "CarromPresenter.h"
#include "raylib.h"

static constexpr float kPowerScale    = 6.0f;   // drag distance → speed multiplier
static constexpr float kMaxDrag       = 120.0f;  // max drag distance in sim units
static constexpr float kMaxAimLength  = 180.0f;  // visual aim line length
static constexpr float kStopSpeedSq   = 25.0f;   // (5 units/sec)^2 — considered stopped

CarromPresenter::CarromPresenter(Simulation& sim, CarromView& view, int windowSize, float boardSize)
    : m_sim(sim)
    , m_view(view)
    , m_padding((static_cast<float>(windowSize) - boardSize) * 0.5f)
    , m_boardSize(boardSize)
{}

void CarromPresenter::update(float dt) {
    handleInput();
    if (m_state == State::Moving) {
        m_sim.step(dt);
        if (allStopped())
            m_state = State::Idle;
    }
    draw();
}

void CarromPresenter::handleInput() {
    const Vec2 mouse = mouseToSim();

    switch (m_state) {
        case State::Idle:
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && isOnStriker(mouse)) {
                m_dragOrigin = mouse;
                m_state = State::Aiming;
            }
            break;

        case State::Aiming:
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                Vec2  drag     = mouse - m_dragOrigin;
                float dragLen  = drag.length();

                if (dragLen > 1.0f) {
                    float clampedLen = dragLen < kMaxDrag ? dragLen : kMaxDrag;
                    Vec2  shootDir   = drag.normalized() * -1.0f;  // opposite to drag = slingshot
                    m_sim.shoot(shootDir * (clampedLen * kPowerScale));
                    m_state = State::Moving;
                } else {
                    m_state = State::Idle;
                }
            }
            break;

        case State::Moving:
            break;
    }
}

void CarromPresenter::draw() const {
    m_view.drawFrame(m_sim.model());

    if (m_state == State::Aiming) {
        const Vec2 strikerPos = m_sim.model().striker.position;
        const Vec2 mouse      = mouseToSim();
        const Vec2 drag       = mouse - m_dragOrigin;
        const Vec2 aimDir     = drag.length() > 0.0f ? drag.normalized() * -1.0f : Vec2{};
        m_view.drawAimLine(strikerPos, strikerPos + aimDir * kMaxAimLength);
    }
}

Vec2 CarromPresenter::mouseToSim() const {
    Vector2 mp = GetMousePosition();
    return {
         mp.x - m_padding - m_boardSize * 0.5f,
        -(mp.y - m_padding - m_boardSize * 0.5f),
    };
}

bool CarromPresenter::isOnStriker(Vec2 simPos) const {
    const Circle2D& s = m_sim.model().striker;
    return (simPos - s.position).lengthSq() <= s.radius * s.radius;
}

bool CarromPresenter::allStopped() const {
    const CarromModel& model = m_sim.model();
    if (model.striker.velocity.lengthSq() >= kStopSpeedSq)
        return false;
    for (const auto& p : model.pieces)
        if (p.velocity.lengthSq() >= kStopSpeedSq)
            return false;
    return true;
}
