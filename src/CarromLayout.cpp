#include "CarromLayout.h"

#include <cmath>

static constexpr float kPi          = 3.14159265358979323846f;
static constexpr float kQueenRadius = 15.0f;
static constexpr float kPieceRadius = 18.0f;
static constexpr float kRing1Dist   = 40.0f;   // queen_r + piece_r + small gap
static constexpr float kRing2Dist   = 82.0f;   // ring1 edge + piece_r + small gap

std::vector<Circle2D> makeCarromLayout() {
    std::vector<Circle2D> pieces;
    pieces.reserve(17);

    // Queen at the very center
    pieces.push_back({{0.0f, 0.0f}, {}, kQueenRadius, 1.0f, PieceType::Queen});

    // Inner ring — 6 pieces (3 black, 3 white), evenly spaced
    for (int i = 0; i < 6; ++i) {
        float angle = (2.0f * kPi * i) / 6.0f;
        Vec2 pos = {kRing1Dist * std::cos(angle), kRing1Dist * std::sin(angle)};
        PieceType type = (i % 2 == 0) ? PieceType::Black : PieceType::White;
        pieces.push_back({pos, {}, kPieceRadius, 1.0f, type});
    }

    // Outer ring — 10 pieces (5 black, 5 white), evenly spaced
    for (int i = 0; i < 10; ++i) {
        float angle = (2.0f * kPi * i) / 10.0f;
        Vec2 pos = {kRing2Dist * std::cos(angle), kRing2Dist * std::sin(angle)};
        PieceType type = (i % 2 == 0) ? PieceType::Black : PieceType::White;
        pieces.push_back({pos, {}, kPieceRadius, 1.0f, type});
    }

    return pieces;
}
