#include "CarromPresenter.h"
#include "raylib.h"
#include <algorithm>
#include <cmath>

static constexpr float kPowerScale    = 6.0f;
static constexpr float kMaxDrag       = 120.0f;
static constexpr float kMaxAimLength  = 180.0f;
static constexpr float kSliderHalfSpan = 210.0f;
static constexpr float kSliderOffsetY  = 25.0f;
static constexpr float kHandleRadius   = 10.0f;

CarromPresenter::CarromPresenter(Simulation& sim, CarromView& view,
                                 int windowSize, float boardSize, Turn firstTurn)
    : m_sim(sim)
    , m_view(view)
    , m_padding((static_cast<float>(windowSize) - boardSize) * 0.5f)
    , m_boardSize(boardSize)
    , m_turn(firstTurn)
{
    m_sim.resetStriker(m_turn == Turn::AI);
    if (m_turn == Turn::AI) {
        m_state   = State::AIThinking;
        m_aiTimer = 0.0f;
    }
}

void CarromPresenter::update(float dt) {
    switch (m_state) {
        case State::AIThinking:
            handleAIThink(dt);
            break;
        case State::AIMoveStriker:
            handleAIMoveStriker(dt);
            break;
        case State::Moving:
            m_sim.step(dt);
            if (allStopped())
                resolveTurnEnd();
            break;
        case State::GameOver:
            break;
        default:
            handleInput();
            break;
    }
    draw();
}

void CarromPresenter::handleAIThink(float dt) {
    m_aiTimer += dt;
    if (m_aiTimer < kAIThinkDelay)
        return;

    auto shot = m_ai.computeShot(m_sim.model());
    if (shot) {
        m_aiTargetX  = shot->strikerX;
        m_aiShootVel = shot->velocity;
    } else {
        m_aiTargetX  = 0.0f;
        m_aiShootVel = {0.0f, -650.0f};  // fallback: shoot toward center
    }
    m_state   = State::AIMoveStriker;
    m_aiTimer = 0.0f;
}

void CarromPresenter::handleAIMoveStriker(float dt) {
    const float currentX = m_sim.model().striker.position.x;
    const float diff     = m_aiTargetX - currentX;
    const float step     = kAIStrikerSpeed * dt;

    if (std::abs(diff) <= step) {
        m_sim.setStrikerX(m_aiTargetX);
        m_sim.clearPottedThisTurn();
        m_sim.shoot(m_aiShootVel);
        m_state = State::Moving;
    } else {
        const float dir = diff > 0.0f ? 1.0f : -1.0f;
        m_sim.setStrikerX(currentX + dir * step);
    }
}

void CarromPresenter::resolveTurnEnd() {
    const auto& potted  = m_sim.pottedThisTurn();
    const PieceType own = (m_turn == Turn::Human) ? PieceType::White : PieceType::Black;

    int  ownCount   = 0;
    bool queenThisTurn = false;
    for (const auto& p : potted) {
        if (p.type == own)              ++ownCount;
        if (p.type == PieceType::Queen) queenThisTurn = true;
    }

    // Queen cover rule: queen needs a same-turn own-piece cover or it returns to center
    if (queenThisTurn && !m_queenSecured) {
        if (ownCount > 0) {
            m_queenSecured = true;
        } else {
            m_sim.returnPieceToCenter(PieceType::Queen);
            queenThisTurn = false;
        }
    }

    // Last-piece rule: cannot finish without securing the queen first
    int remainingOwn = 0;
    for (const auto& p : m_sim.model().pieces)
        if (p.type == own) remainingOwn++;

    if (remainingOwn == 0 && !m_queenSecured) {
        m_sim.returnPieceToCenter(own);
        ownCount = std::max(0, ownCount - 1);
    }

    m_sim.clearPottedThisTurn();

    // Recount after any returns
    remainingOwn = 0;
    for (const auto& p : m_sim.model().pieces)
        if (p.type == own) remainingOwn++;

    // Win: all own pieces gone and queen secured
    if (remainingOwn == 0 && m_queenSecured) {
        m_winner = m_turn;
        m_state  = State::GameOver;
        return;
    }

    // Bonus turn: potted at least one own piece that stayed on the score sheet
    if (ownCount > 0) {
        m_sim.resetStriker(m_turn == Turn::AI);
        if (m_turn == Turn::AI) {
            m_state   = State::AIThinking;
            m_aiTimer = 0.0f;
        } else {
            m_state = State::Idle;
        }
    } else {
        m_state = State::Idle;
        switchTurn();
    }
}

void CarromPresenter::switchTurn() {
    m_turn = (m_turn == Turn::Human) ? Turn::AI : Turn::Human;
    m_sim.resetStriker(m_turn == Turn::AI);
    if (m_turn == Turn::AI) {
        m_state   = State::AIThinking;
        m_aiTimer = 0.0f;
    }
}

void CarromPresenter::handleInput() {
    const Vector2 mp    = GetMousePosition();
    const Vec2    mouse = mouseToSim();

    switch (m_state) {
        case State::Idle:
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (isOnSliderHandle(mp.x, mp.y)) {
                    m_state = State::DraggingSlider;
                } else if (isOnStriker(mouse)) {
                    m_dragOrigin = mouse;
                    m_state      = State::Aiming;
                }
            }
            break;

        case State::DraggingSlider:
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                float simX = mp.x - m_padding - m_boardSize * 0.5f;
                m_sim.setStrikerX(simX);
            } else {
                m_state = State::Idle;
            }
            break;

        case State::Aiming:
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                Vec2  drag    = mouse - m_dragOrigin;
                float dragLen = drag.length();
                if (dragLen > 1.0f) {
                    float clamped  = dragLen < kMaxDrag ? dragLen : kMaxDrag;
                    Vec2  shootDir = drag.normalized() * -1.0f;
                    m_sim.clearPottedThisTurn();
                    m_sim.shoot(shootDir * (clamped * kPowerScale));
                    m_state = State::Moving;
                } else {
                    m_state = State::Idle;
                }
            }
            break;

        default:
            break;
    }
}

void CarromPresenter::draw() const {
    const bool sliderActive = (m_state == State::Idle && m_turn == Turn::Human);

    m_view.beginFrame();
    m_view.drawScene(m_sim.model(), sliderActive);

    if (m_state == State::Aiming) {
        const Circle2D& striker = m_sim.model().striker;
        const Vec2      mouse   = mouseToSim();
        const Vec2      drag    = mouse - m_dragOrigin;
        const float     dragLen = drag.length();
        const Vec2      aimDir  = dragLen > 0.0f ? drag.normalized() * -1.0f : Vec2{};
        const float     t       = std::min(dragLen, kMaxDrag) / kMaxDrag;
        const float     powerR  = striker.radius * (1.0f + t);   // striker.r → 2×striker.r
        m_view.drawAimLine(striker.position, striker.position + aimDir * kMaxAimLength, powerR);
    }

    if (m_state == State::GameOver) {
        const float winW  = m_boardSize + 2.0f * m_padding;
        const char* msg   = (m_winner == Turn::Human) ? "You Win!" : "AI Wins!";
        const Color col   = (m_winner == Turn::Human) ? DARKGREEN : MAROON;
        const int   sz    = 48;
        int         textW = MeasureText(msg, sz);
        DrawRectangle(0, 0, (int)winW, (int)winW, {0, 0, 0, 140});
        DrawText(msg, ((int)winW - textW) / 2, (int)winW / 2 - sz / 2, sz, col);
    } else {
        const char* label = (m_turn == Turn::Human) ? "Your Turn" : "AI Thinking...";
        DrawText(label, 10, 40, 18, (m_turn == Turn::Human) ? DARKGREEN : MAROON);
        if (m_queenSecured)
            DrawText("Queen secured", 10, 62, 14, GOLD);
    }

    m_view.endFrame();
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

bool CarromPresenter::isOnSliderHandle(float screenX, float screenY) const {
    const float handleX = m_padding + m_boardSize * 0.5f + m_sim.model().striker.position.x;
    const float handleY = m_padding + m_boardSize + kSliderOffsetY;
    const float dx      = screenX - handleX;
    const float dy      = screenY - handleY;
    return dx * dx + dy * dy <= kHandleRadius * kHandleRadius;
}

bool CarromPresenter::allStopped() const {
    const CarromModel& model = m_sim.model();
    if (model.striker.velocity.lengthSq() > 0.0f)
        return false;
    for (const auto& p : model.pieces)
        if (p.velocity.lengthSq() > 0.0f)
            return false;
    return true;
}
