#include "CarromView.h"
#include "CarromModel.h"
#include "raylib.h"

// Free helpers — raylib types stay out of the header
static Vector2 toScreen(Vec2 p, float padding, float boardSize) {
    return {
        padding + boardSize * 0.5f + p.x,
        padding + boardSize * 0.5f - p.y,
    };
}

static Color colorOf(PieceType type) {
    switch (type) {
        case PieceType::Queen:   return {220,  40,  40, 255};
        case PieceType::Black:   return { 30,  30,  30, 255};
        case PieceType::White:   return {245, 245, 245, 255};
        case PieceType::Striker: return { 70, 130, 180, 255};  // steel blue
    }
    return WHITE;
}

CarromView::CarromView(int windowSize, float boardSize)
    : m_windowSize(static_cast<float>(windowSize))
    , m_boardSize(boardSize)
    , m_padding((m_windowSize - m_boardSize) * 0.5f)
    , m_centerCircleR(20.0f)
    , m_pieceRingInner(102.0f)
    , m_pieceRingOuter(m_pieceRingInner + 10.0f)
{}

void CarromView::beginFrame() const {
    BeginDrawing();
    ClearBackground(RAYWHITE);
}

void CarromView::endFrame() const {
    DrawFPS(10, 10);
    EndDrawing();
}

void CarromView::drawScene(const CarromModel& model, bool sliderActive) const {
    drawBoard(model.board);
    drawPockets(model.pockets);
    drawPieces(model.pieces);
    drawStriker(model.striker);
    drawSlider(model.striker, sliderActive);
}

void CarromView::drawBoard(const Board& board) const {
    DrawRectangle((int)m_padding, (int)m_padding, (int)board.size, (int)board.size, {210, 180, 140, 255});
    DrawRectangleLines((int)m_padding, (int)m_padding, (int)board.size, (int)board.size, DARKBROWN);

    Vector2 center = toScreen(board.center, m_padding, m_boardSize);
    DrawCircleLinesV(center, m_centerCircleR,  DARKBROWN);
    DrawCircleLinesV(center, m_pieceRingInner, DARKBROWN);
    DrawCircleLinesV(center, m_pieceRingOuter, DARKBROWN);

    drawStrikerLines(board);
}

void CarromView::drawStrikerLines(const Board& board) const {
    // Each side has two parallel lines inset from the edge, forming the striker zone.
    // Lines stop short of the corner pockets on both ends.
    static constexpr float kEdgeOffset  = 50.0f;  // outer line distance from board edge
    static constexpr float kLineGap     = 20.0f;  // gap between the two lines
    static constexpr float kCornerInset = 90.0f;  // how far lines stay from each corner

    const float outerOffset = kEdgeOffset;
    const float innerOffset = kEdgeOffset + kLineGap;
    const float spanStart   = board.left()   + kCornerInset;
    const float spanEnd     = board.right()  - kCornerInset;
    const float spanBottom  = board.bottom() + kCornerInset;
    const float spanTop     = board.top()    - kCornerInset;

    const float circleOuterR = kLineGap * 0.5f;        // diameter == gap between the two lines
    const float circleInnerR = circleOuterR - 2.0f;

    auto line = [&](Vec2 a, Vec2 b) {
        DrawLineV(toScreen(a, m_padding, m_boardSize),
                  toScreen(b, m_padding, m_boardSize),
                  DARKBROWN);
    };

    auto endCircles = [&](Vec2 center) {
        Vector2 sc = toScreen(center, m_padding, m_boardSize);
        DrawCircleLinesV(sc, circleOuterR, DARKBROWN);
        DrawCircleLinesV(sc, circleInnerR, DARKBROWN);
    };

    // Midpoint between the two lines on each side
    const float hMid = outerOffset + kLineGap * 0.5f;  // horizontal sides
    const float vMid = outerOffset + kLineGap * 0.5f;  // vertical sides

    // Bottom side
    line({spanStart, board.bottom() + outerOffset}, {spanEnd, board.bottom() + outerOffset});
    line({spanStart, board.bottom() + innerOffset}, {spanEnd, board.bottom() + innerOffset});
    endCircles({spanStart, board.bottom() + hMid});
    endCircles({spanEnd,   board.bottom() + hMid});

    // Top side
    line({spanStart, board.top() - outerOffset}, {spanEnd, board.top() - outerOffset});
    line({spanStart, board.top() - innerOffset}, {spanEnd, board.top() - innerOffset});
    endCircles({spanStart, board.top() - hMid});
    endCircles({spanEnd,   board.top() - hMid});

    // Left side
    line({board.left() + outerOffset, spanBottom}, {board.left() + outerOffset, spanTop});
    line({board.left() + innerOffset, spanBottom}, {board.left() + innerOffset, spanTop});
    endCircles({board.left() + vMid, spanBottom});
    endCircles({board.left() + vMid, spanTop});

    // Right side
    line({board.right() - outerOffset, spanBottom}, {board.right() - outerOffset, spanTop});
    line({board.right() - innerOffset, spanBottom}, {board.right() - innerOffset, spanTop});
    endCircles({board.right() - vMid, spanBottom});
    endCircles({board.right() - vMid, spanTop});
}

void CarromView::drawPockets(const std::array<Pocket, 4>& pockets) const {
    for (const auto& p : pockets)
        DrawCircleV(toScreen(p.position, m_padding, m_boardSize), p.radius, BLACK);
}

void CarromView::drawSlider(const Circle2D& striker, bool active) const {
    static constexpr float kSliderHalfSpan = 210.0f;
    static constexpr float kSliderOffsetY  = 25.0f;
    static constexpr float kHandleRadius   = 10.0f;
    static constexpr float kHandleInnerR   = 8.0f;

    const float centerX = m_padding + m_boardSize * 0.5f;
    const float trackY  = m_padding + m_boardSize + kSliderOffsetY;
    const float trackX0 = centerX - kSliderHalfSpan;
    const float trackX1 = centerX + kSliderHalfSpan;

    const Color trackColor  = active ? DARKBROWN : Color{160, 140, 120, 120};
    const Color outerColor  = active ? DARKBROWN : Color{160, 140, 120, 120};
    const Color innerColor  = active ? Color{210, 180, 140, 255} : Color{200, 200, 200, 120};
    const Color borderColor = active ? BLACK     : Color{160, 160, 160, 120};

    DrawLineV({trackX0, trackY}, {trackX1, trackY}, trackColor);

    const float handleX = centerX + striker.position.x;
    DrawCircleV({handleX, trackY}, kHandleRadius, outerColor);
    DrawCircleV({handleX, trackY}, kHandleInnerR, innerColor);
    DrawCircleLinesV({handleX, trackY}, kHandleRadius, borderColor);
}

void CarromView::drawAimLine(Vec2 from, Vec2 to, float powerRadius) const {
    Vector2 sp = toScreen(from, m_padding, m_boardSize);
    // Power ring: grows from striker radius to 2× striker radius
    DrawCircleLinesV(sp, powerRadius, {70, 130, 180, 200});
    DrawLineV(sp, toScreen(to, m_padding, m_boardSize), {70, 130, 180, 200});
}

void CarromView::drawStriker(const Circle2D& striker) const {
    Vector2 sp = toScreen(striker.position, m_padding, m_boardSize);
    DrawCircleV(sp, striker.radius, colorOf(striker.type));
    DrawCircleLinesV(sp, striker.radius, DARKBLUE);
    DrawCircleLinesV(sp, striker.radius * 0.4f, DARKBLUE);  // inner ring detail
}

void CarromView::drawPieces(const std::vector<Circle2D>& pieces) const {
    for (const auto& c : pieces) {
        Vector2 sp = toScreen(c.position, m_padding, m_boardSize);
        DrawCircleV(sp, c.radius, colorOf(c.type));
        DrawCircleLinesV(sp, c.radius, BLACK);
    }
}
