#include "CarromAI.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

// ---------------------------------------------------------------------------
// Free helpers
// ---------------------------------------------------------------------------

// True if the line segment [from, to] is NOT blocked by any piece in `pieces`
// (ignoring `skip`). `movingR` is the radius of the object travelling the path.
static bool pathClear(Vec2 from, Vec2 to,
                       const std::vector<Circle2D>& pieces,
                       const Circle2D* skip,
                       float movingR)
{
    Vec2  ab  = to - from;
    float len = ab.length();
    if (len < 0.001f)
        return true;

    Vec2 n = ab * (1.0f / len);

    for (const auto& p : pieces) {
        if (&p == skip)
            continue;
        Vec2  ac      = p.position - from;
        float t       = std::clamp(ac.dot(n), 0.0f, len);
        Vec2  closest = from + n * t;
        if ((p.position - closest).length() < p.radius + movingR)
            return false;
    }
    return true;
}

// Friction decay constant: per-second speed loss ≈ 1 - 0.985^60 × 60 = 0.91
// Elastic collision mass ratio — striker(1.5) hitting piece(1.0):
//   piece_vel = 2 × m_striker / (m_striker + m_piece) × v_striker = 1.2 × v_striker
static constexpr float kFrictionDecay = 0.91f;
static constexpr float kMassTransfer  = 1.2f;

// cosAngle: dot product of the striker's travel direction and pocketDir.
// The collision impulse only transfers the component of striker velocity along
// the normal (= pocketDir), so effective piece speed = 1.2 × v × cosAngle.
// We must solve for the striker initial speed accounting for this projection.
static float powerForRange(float strikerToGhost, float pieceToPocket, float cosAngle) {
    // Guard against near-perpendicular shots inflating power to infinity
    const float clampedCos = std::max(cosAngle, 0.45f);
    // Piece must receive this much speed to reach the pocket
    const float pieceNeed = pieceToPocket * kFrictionDecay;
    // Striker arrival speed needed: piece gets 1.2 × v_arrival × cosAngle
    const float strikerArrival = pieceNeed / (kMassTransfer * clampedCos);
    // Striker decelerates under the same friction while travelling to ghost ball
    const float strikerLoss = strikerToGhost * kFrictionDecay;
    return (strikerArrival + strikerLoss) * 1.06f;
}

// ---------------------------------------------------------------------------
// CarromAI::computeShot
// ---------------------------------------------------------------------------

std::optional<AiShot> CarromAI::computeShot(const CarromModel& model) const {
    if (model.pieces.empty())
        return setupShot(model);

    std::optional<ShotCandidate> best;

    for (const auto& piece : model.pieces) {
        if (piece.type != PieceType::Black && piece.type != PieceType::Queen)
            continue;
        for (const auto& pocket : model.pockets) {
            auto candidate = evaluatePocketShot(model, piece, pocket);
            if (!candidate)
                continue;
            if (!best || candidate->score > best->score)
                best = candidate;
        }
    }

    if (!best)
        return setupShot(model);

    return AiShot{best->strikerX, best->velocity};
}

// ---------------------------------------------------------------------------
// evaluatePocketShot  (ghost-ball method with validity checks)
// ---------------------------------------------------------------------------

std::optional<CarromAI::ShotCandidate> CarromAI::evaluatePocketShot(
    const CarromModel& model,
    const Circle2D&    piece,
    const Pocket&      pocket) const
{
    // --- 1. Ghost ball position ---
    Vec2  toPocket   = pocket.position - piece.position;
    float pocketDist = toPocket.length();
    if (pocketDist < 0.001f)
        return std::nullopt;

    Vec2 pocketDir = toPocket * (1.0f / pocketDist);
    Vec2 ghostBall = piece.position - pocketDir * (piece.radius + model.striker.radius);

    // --- 2. Clamp striker X to the baseline zone ---
    float strikerX   = std::clamp(ghostBall.x, -kStrikerHalfSpan, kStrikerHalfSpan);
    Vec2  strikerPos = {strikerX, model.striker.position.y};

    // --- 3. Reachability: ghost ball must be in front of the striker ---
    // AI shoots from top baseline (positive Y). "Forward" = smaller Y (downward).
    // If ghost ball is above (higher Y) than striker, the shot would go backward.
    if (model.striker.position.y > 0.0f) {
        if (ghostBall.y >= strikerPos.y - 10.0f)
            return std::nullopt;
    } else {
        if (ghostBall.y <= strikerPos.y + 10.0f)
            return std::nullopt;
    }

    // --- 4. Direction and travel length ---
    Vec2  toGhost    = ghostBall - strikerPos;
    float toGhostLen = toGhost.length();
    if (toGhostLen < 0.001f)
        return std::nullopt;

    // --- 5. Obstacle check: striker path to ghost ball ---
    if (!pathClear(strikerPos, ghostBall, model.pieces, &piece, model.striker.radius))
        return std::nullopt;

    // --- 6. Obstacle check: piece path to pocket ---
    if (!pathClear(piece.position, pocket.position, model.pieces, &piece, piece.radius * 0.85f))
        return std::nullopt;

    // --- 7. Power calibrated to push the piece all the way to the pocket ---
    // cosAngle: how aligned the striker's travel direction is with the pocket direction.
    // Only the projected component of the striker's velocity transfers to the piece.
    const float cosAngle = (toGhost * (1.0f / toGhostLen)).dot(pocketDir);
    if (cosAngle <= 0.05f)
        return std::nullopt;  // shot is nearly perpendicular — skip it

    float rawPower = powerForRange(toGhostLen, pocketDist, cosAngle);
    float power    = std::clamp(rawPower, kMinPower, kMaxPower);
    Vec2  velocity = toGhost * (power / toGhostLen);

    return ShotCandidate{strikerX, velocity, scoreShot(piece, pocket, toGhostLen)};
}

// ---------------------------------------------------------------------------
// Scoring
// ---------------------------------------------------------------------------

float CarromAI::scoreShot(const Circle2D& piece, const Pocket& pocket, float strikerTravel) {
    float pocketDist = (pocket.position - piece.position).length();

    // Prefer pieces closer to pockets; penalise long striker travel
    float score = 1500.0f / (pocketDist + 1.0f)
                - strikerTravel * 0.4f;

    if (piece.type == PieceType::Queen)
        score *= 2.0f;

    return score;
}

// ---------------------------------------------------------------------------
// Setup shot: aim directly at the best-positioned piece when no clean pot exists
// ---------------------------------------------------------------------------

AiShot CarromAI::setupShot(const CarromModel& model) {
    float bestScore = -FLT_MAX;
    Vec2  bestPos   = {0.0f, 0.0f};
    bool  found     = false;

    for (const auto& p : model.pieces) {
        if (p.type != PieceType::Black && p.type != PieceType::Queen)
            continue;
        for (const auto& pocket : model.pockets) {
            float d = (p.position - pocket.position).length();
            float s = 1000.0f / (d + 1.0f) + (p.type == PieceType::Queen ? 500.0f : 0.0f);
            if (s > bestScore) { bestScore = s; bestPos = p.position; found = true; }
        }
    }

    if (!found) {
        // Nothing left to aim at — shoot centre
        const float dir = model.striker.position.y > 0.0f ? -1.0f : 1.0f;
        return {0.0f, {0.0f, kMaxPower * dir}};
    }

    // Aim striker directly at the chosen piece with high power
    float strikerX   = std::clamp(bestPos.x, -kStrikerHalfSpan, kStrikerHalfSpan);
    Vec2  strikerPos = {strikerX, model.striker.position.y};
    Vec2  toTarget   = bestPos - strikerPos;
    float dist       = toTarget.length();
    if (dist < 0.001f) {
        const float dir = model.striker.position.y > 0.0f ? -1.0f : 1.0f;
        return {0.0f, {0.0f, kMaxPower * dir}};
    }

    float power = std::clamp(dist * 1.8f, kMinPower, kMaxPower);
    return {strikerX, toTarget * (power / dist)};
}
