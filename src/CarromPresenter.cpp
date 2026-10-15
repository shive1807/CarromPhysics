#include "CarromPresenter.h"
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <random>

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
    // Rules button / panel dismiss — checked before anything else so it works in every state
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        const Vector2 mp = GetMousePosition();
        const float   dx = mp.x - CarromView::kRulesBtnCX;
        const float   dy = mp.y - CarromView::kRulesBtnCY;
        if (dx * dx + dy * dy <= CarromView::kRulesBtnR * CarromView::kRulesBtnR) {
            m_showRules = !m_showRules;
        } else if (m_showRules) {
            const float cx = mp.x - CarromView::kCloseBtnCX;
            const float cy = mp.y - CarromView::kCloseBtnCY;
            if (cx * cx + cy * cy <= CarromView::kCloseBtnR * CarromView::kCloseBtnR)
                m_showRules = false;
        }
    }

    if (m_showRules) {
        draw();
        return;  // freeze game while panel is open
    }

    switch (m_state) {
        case State::AIThinking:
            handleAIThink(dt);
            break;
        case State::AIMoveStriker:
            handleAIMoveStriker(dt);
            break;
        case State::AIAiming:
            handleAIAim(dt);
            break;
        case State::Moving:
            m_sim.step(dt);
            emitParticles(dt);
            if (allStopped()) {
                m_particles.clear();
                resolveTurnEnd();
            }
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
        // Pick a random pause between kAIAimDelayMin and kAIAimDelayMax seconds
        static std::mt19937 s_rng{std::random_device{}()};
        m_aiAimDuration = std::uniform_real_distribution<float>{kAIAimDelayMin, kAIAimDelayMax}(s_rng);
        m_aiAimTimer    = m_aiAimDuration;
        m_state = State::AIAiming;
    } else {
        const float dir = diff > 0.0f ? 1.0f : -1.0f;
        m_sim.setStrikerX(currentX + dir * step);
    }
}

void CarromPresenter::handleAIAim(float dt) {
    m_aiAimTimer -= dt;
    if (m_aiAimTimer <= 0.0f) {
        m_sim.clearPottedThisTurn();
        m_sim.shoot(m_aiShootVel);
        m_sound.playShoot();
        m_state = State::Moving;
    }
}

void CarromPresenter::resolveTurnEnd() {
    if (!m_sim.pottedThisTurn().empty())
        m_sound.playPocket();

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
        m_sound.playGameOver(m_turn == Turn::Human);
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
                    m_sound.playShoot();
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

static void trailColor(PieceType t, uint8_t& r, uint8_t& g, uint8_t& b) {
    switch (t) {
        case PieceType::Queen:   r=220; g=60;  b=60;  break;
        case PieceType::Black:   r=110; g=110; b=110; break;
        case PieceType::White:   r=200; g=200; b=215; break;
        case PieceType::Striker: r=70;  g=130; b=180; break;
    }
}

void CarromPresenter::emitParticles(float dt) {
    // Trails — one call per piece per frame, ParticleSystem filters by speed threshold
    for (const auto& piece : m_sim.model().pieces) {
        uint8_t r, g, b;
        trailColor(piece.type, r, g, b);
        m_particles.emitTrail(piece.position, piece.velocity, r, g, b);
    }
    {
        uint8_t r, g, b;
        trailColor(PieceType::Striker, r, g, b);
        m_particles.emitTrail(m_sim.model().striker.position,
                              m_sim.model().striker.velocity, r, g, b);
    }

    // Collision bursts + sounds — drain events recorded during this frame's step()
    float maxImpulse = 0.0f;
    for (const auto& ev : m_sim.collisionEvents()) {
        m_particles.emitBurst(ev.position, ev.impulse);
        if (ev.impulse > maxImpulse) maxImpulse = ev.impulse;
    }
    m_sim.clearCollisionEvents();

    // Play at most one collision sound per ~60 ms to avoid audio spam
    m_soundCooldown -= dt;
    if (maxImpulse > 5.0f && m_soundCooldown <= 0.0f) {
        m_sound.playCollision(maxImpulse);
        m_soundCooldown = 0.06f;
    }

    m_particles.update(dt);
}

void CarromPresenter::draw() const {
    const bool sliderActive = (m_state == State::Idle && m_turn == Turn::Human);

    m_view.beginFrame();
    m_view.drawBackground(m_sim.model());
    m_view.drawParticles(m_particles.particles());   // under pieces
    m_view.drawForeground(m_sim.model(), sliderActive);

    if (m_state == State::Aiming) {
        const Circle2D& striker = m_sim.model().striker;
        const Vec2      mouse   = mouseToSim();
        const Vec2      drag    = mouse - m_dragOrigin;
        const float     dragLen = drag.length();
        const Vec2      aimDir  = dragLen > 0.0f ? drag.normalized() * -1.0f : Vec2{};
        const float     t       = std::min(dragLen, kMaxDrag) / kMaxDrag;
        const float     powerR  = striker.radius * (1.0f + t);
        m_view.drawAimLine(striker.position, striker.position + aimDir * kMaxAimLength, powerR);
    }

    if (m_state == State::AIAiming) {
        const Circle2D& striker  = m_sim.model().striker;
        const float     shootLen = m_aiShootVel.length();
        if (shootLen > 0.0f) {
            const Vec2  aimDir = m_aiShootVel * (1.0f / shootLen);
            // Charge fraction: 0 when aim starts → 1 just before shot fires
            const float charge = (m_aiAimDuration > 0.0f)
                                 ? 1.0f - (m_aiAimTimer / m_aiAimDuration)
                                 : 1.0f;
            const float powerR = striker.radius * (1.0f + charge);  // 1× → 2×
            m_view.drawAimLine(striker.position,
                               striker.position + aimDir * kMaxAimLength,
                               powerR);
        }
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

    m_view.drawRulesButton();

    if (m_showRules)
        m_view.drawRulesPanel();

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
