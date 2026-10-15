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

void CarromView::drawBackground(const CarromModel& model) const {
    drawBoard(model.board);
    drawPockets(model.pockets);
}

void CarromView::drawForeground(const CarromModel& model, bool sliderActive) const {
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

void CarromView::drawRulesButton() const {
    DrawCircleV({kRulesBtnCX, kRulesBtnCY}, kRulesBtnR,  {50, 50, 50, 220});
    DrawCircleLinesV({kRulesBtnCX, kRulesBtnCY}, kRulesBtnR, LIGHTGRAY);
    DrawText("?", (int)(kRulesBtnCX - 4), (int)(kRulesBtnCY - 8), 16, WHITE);
}

void CarromView::drawRulesPanel() const {
    const Color kText = {50, 35, 20, 255};

    // Full-screen dim
    DrawRectangle(0, 0, (int)(m_boardSize + 2.0f * m_padding),
                        (int)(m_boardSize + 2.0f * m_padding), {0, 0, 0, 150});

    // Panel card
    DrawRectangleRounded({kPanelX, kPanelY, kPanelW, kPanelH}, 0.04f, 8,
                         {240, 230, 210, 250});
    DrawRectangleRoundedLines({kPanelX, kPanelY, kPanelW, kPanelH}, 0.04f, 8,
                              DARKBROWN);

    // Close button
    DrawCircleV({kCloseBtnCX, kCloseBtnCY}, kCloseBtnR, {160, 60, 60, 230});
    DrawText("x", (int)(kCloseBtnCX - 5), (int)(kCloseBtnCY - 8), 16, WHITE);

    // Title
    const char* title = "CARROM RULES";
    int titleW = MeasureText(title, 26);
    DrawText(title, (int)(kPanelX + kPanelW * 0.5f) - titleW / 2,
             (int)(kPanelY + 18), 26, DARKBROWN);

    // Divider
    const float divY   = kPanelY + 54.0f;
    const float textX  = kPanelX + 28.0f;
    DrawLineEx({kPanelX + 20, divY}, {kPanelX + kPanelW - 20, divY}, 1.5f, DARKBROWN);

    // --- Row 0: player colours, drawn as explicit segments so chips never overlap text ---
    const float row0Y   = divY + 16.0f;
    const float chipR   = 7.0f;
    const float chipGap = 6.0f;   // gap between chip and neighbouring word
    const int   fs      = 16;

    // "You play "
    const char* sYouPlay = "You play ";
    DrawText(sYouPlay, (int)textX, (int)row0Y, fs, kText);
    float cx = textX + MeasureText(sYouPlay, fs) + chipR + chipGap;
    // white chip
    DrawCircleV({cx, row0Y + chipR}, chipR, {245, 245, 245, 255});
    DrawCircleLinesV({cx, row0Y + chipR}, chipR, DARKGRAY);
    // " White   |   AI plays "
    float afterChip = cx + chipR + chipGap;
    const char* sWhite = "White";
    DrawText(sWhite, (int)afterChip, (int)row0Y, fs, kText);
    afterChip += MeasureText(sWhite, fs);

    const char* sSep = "   |   AI plays ";
    DrawText(sSep, (int)afterChip, (int)row0Y, fs, kText);
    afterChip += MeasureText(sSep, fs);

    // black chip
    float bcx = afterChip + chipR + chipGap;
    DrawCircleV({bcx, row0Y + chipR}, chipR, {30, 30, 30, 255});
    DrawCircleLinesV({bcx, row0Y + chipR}, chipR, DARKGRAY);
    // " Black"
    DrawText("Black", (int)(bcx + chipR + chipGap), (int)row0Y, fs, kText);

    // --- Remaining rules ---
    static const char* kRules[] = {
        "Pot your colored pieces to score.",
        "",
        "Pot the Queen and cover it by pocketing",
        "one of your own pieces in the same turn.",
        "",
        "Pot the Queen without a cover and it",
        "returns to the center of the board.",
        "",
        "You cannot pot your last piece until",
        "the Queen has been secured.",
        "",
        "Pot your own piece  \xe2\x86\x92  earn a bonus turn.",
        "",
        "First to pot all pieces (Queen secured)",
        "wins the game!",
    };

    const float kLineH = 27.0f;
    float       y      = row0Y + kLineH + 6.0f;  // a little extra gap after the chips row

    for (const auto* line : kRules) {
        if (line[0] != '\0')
            DrawText(line, (int)textX, (int)y, fs, kText);
        y += kLineH;
    }
}

void CarromView::drawParticles(const std::vector<Particle>& particles) const {
    for (const auto& p : particles) {
        const float t     = p.life / p.maxLife;               // 1 → 0
        const auto  alpha = static_cast<uint8_t>(t * 210.0f); // fade out
        const float r     = p.radius * (0.4f + 0.6f * t);    // shrink as it fades
        DrawCircleV(toScreen(p.pos, m_padding, m_boardSize), r, {p.r, p.g, p.b, alpha});
    }
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
