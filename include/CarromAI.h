#pragma once

#include "CarromModel.h"

#include <optional>

struct AiShot {
    float strikerX;
    Vec2  velocity;
};

class CarromAI {
public:
    std::optional<AiShot> computeShot(const CarromModel& model) const;

private:
    struct ShotCandidate {
        float strikerX;
        Vec2  velocity;
        float score;
    };

    std::optional<ShotCandidate> evaluatePocketShot(
        const CarromModel& model,
        const Circle2D&    piece,
        const Pocket&      pocket) const;

    static float  scoreShot(const Circle2D& piece, const Pocket& pocket, float strikerTravel);
    static AiShot setupShot(const CarromModel& model);

    static constexpr float kStrikerHalfSpan = 210.0f;
    static constexpr float kMinPower        = 300.0f;
    static constexpr float kMaxPower        = 750.0f;
};
