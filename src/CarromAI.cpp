#include "CarromAI.h"

#include <algorithm>
#include <cmath>

std::optional<AiShot> CarromAI::computeShot(const CarromModel& model) const {
    if (model.pieces.empty())
        return fallbackShot(model);

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
        return fallbackShot(model);

    return AiShot{best->strikerX, best->velocity};
}

std::optional<CarromAI::ShotCandidate> CarromAI::evaluatePocketShot(
    const CarromModel& model,
    const Circle2D&    piece,
    const Pocket&      pocket) const
{
    // Ghost ball method:
    // Find where the striker center must be at the moment of contact to send the piece toward the pocket.
    Vec2  toPocket = pocket.position - piece.position;
    float dist     = toPocket.length();
    if (dist < 0.001f)
        return std::nullopt;

    Vec2 pocketDir = toPocket * (1.0f / dist);
    Vec2 ghostBall = piece.position - pocketDir * (piece.radius + model.striker.radius);

    // Striker is constrained to baseline Y — clamp X to valid zone
    float strikerX = ghostBall.x;
    if (strikerX < -kStrikerHalfSpan) strikerX = -kStrikerHalfSpan;
    if (strikerX >  kStrikerHalfSpan) strikerX =  kStrikerHalfSpan;

    Vec2  strikerPos = {strikerX, model.striker.position.y};
    Vec2  toGhost    = ghostBall - strikerPos;
    float toGhostLen = toGhost.length();
    if (toGhostLen < 0.001f)
        return std::nullopt;

    float power    = std::clamp(toGhostLen * kPowerFactor, kMinPower, kMaxPower);
    Vec2  velocity = toGhost * (power / toGhostLen);

    return ShotCandidate{strikerX, velocity, scoreShot(piece, pocket)};
}

float CarromAI::scoreShot(const Circle2D& piece, const Pocket& pocket) {
    float dist  = (pocket.position - piece.position).length();
    float score = 1000.0f / (dist + 1.0f);

    if (piece.type == PieceType::Queen)
        score *= 2.0f;

    return score;
}

AiShot CarromAI::fallbackShot(const CarromModel& model) {
    // Shoot toward board center — direction depends on which baseline the striker is on
    const float dir = model.striker.position.y > 0.0f ? -1.0f : 1.0f;
    return {0.0f, {0.0f, kMaxPower * dir}};
}
