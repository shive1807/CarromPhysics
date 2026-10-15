#include "CarromView.h"
#include "CarromModel.h"
#include "raylib.h"
#include <cmath>

// ---------------------------------------------------------------------------
// Board colour palette
// ---------------------------------------------------------------------------
static constexpr Color kSurface  = {224, 202, 140, 255};   // light wood playing surface
static constexpr Color kFrame    = { 68,  38,   8, 255};   // dark mahogany frame
static constexpr Color kMark     = {140,  88,  28, 255};   // board marking lines
static constexpr Color kPocketBg = { 18,  12,   4, 255};   // pocket hole (near-black)

// ---------------------------------------------------------------------------
// Free helpers — raylib types stay out of the header
// ---------------------------------------------------------------------------
static Vector2 toScreen(Vec2 p, float padding, float boardSize) {
    return {
        padding + boardSize * 0.5f + p.x,
        padding + boardSize * 0.5f - p.y,
    };
}

static Color colorOf(PieceType type) {
    switch (type) {
        case PieceType::Queen:   return {210,  35,  35, 255};
        case PieceType::Black:   return { 32,  26,  20, 255};
        case PieceType::White:   return {242, 238, 222, 255};
        case PieceType::Striker: return { 80, 140, 200, 255};
    }
    return WHITE;
}

// ---------------------------------------------------------------------------
// CarromView
// ---------------------------------------------------------------------------
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
    ClearBackground({38, 28, 16, 255});   // dark table surface outside the board
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

// ---------------------------------------------------------------------------
// Board
// ---------------------------------------------------------------------------
void CarromView::drawBoard(const Board& board) const {
    const float bs  = m_boardSize;
    const float px  = m_padding;
    const Vector2 ctr = toScreen({0.0f, 0.0f}, m_padding, m_boardSize);

    // --- 1. Dark wooden frame (extends beyond the playing surface) ---
    static constexpr float kFrameW = 22.0f;
    DrawRectangle((int)(px - kFrameW), (int)(px - kFrameW),
                  (int)(bs + kFrameW * 2.0f), (int)(bs + kFrameW * 2.0f), kFrame);

    // Thin inner rim — lighter strip between frame and surface
    DrawRectangle((int)(px - 3), (int)(px - 3),
                  (int)(bs + 6.0f), (int)(bs + 6.0f), {108, 65, 18, 255});

    // --- 2. Playing surface ---
    DrawRectangle((int)px, (int)px, (int)bs, (int)bs, kSurface);

    // --- 3. Double inner border on the playing surface ---
    DrawRectangleLines((int)px,           (int)px,           (int)bs,        (int)bs,        kMark);
    DrawRectangleLines((int)(px + 5.0f),  (int)(px + 5.0f),  (int)(bs - 10), (int)(bs - 10), kMark);

    // --- 4. Corner diagonal markings (characteristic carrom-board feature) ---
    // Each corner: a diagonal line from one edge to the other, creating a triangle.
    static constexpr float kDiag = 88.0f;
    const float tlx = px,        tly = px;
    const float trx = px + bs,   try_ = px;
    const float blx = px,        bly = px + bs;
    const float brx = px + bs,   bry = px + bs;

    DrawLineEx({tlx + kDiag, tly},  {tlx, tly + kDiag},  1.6f, kMark);
    DrawLineEx({trx - kDiag, try_}, {trx, try_ + kDiag}, 1.6f, kMark);
    DrawLineEx({blx + kDiag, bly},  {blx, bly - kDiag},  1.6f, kMark);
    DrawLineEx({brx - kDiag, bry},  {brx, bry - kDiag},  1.6f, kMark);

    // --- 5. Diagonal cross lines from piece ring outward toward each corner ---
    // These run from the outer edge of the piece ring to ~82 % of the way to the corner.
    const float sq2    = 1.0f / sqrtf(2.0f);
    const float dStart = m_pieceRingOuter;
    const float dEnd   = bs * 0.5f * 0.82f;

    const float ddx[4] = {-sq2, +sq2, -sq2, +sq2};
    const float ddy[4] = {-sq2, -sq2, +sq2, +sq2};
    for (int i = 0; i < 4; ++i) {
        DrawLineEx(
            {ctr.x + ddx[i] * dStart, ctr.y + ddy[i] * dStart},
            {ctr.x + ddx[i] * dEnd,   ctr.y + ddy[i] * dEnd},
            1.2f, kMark
        );
    }

    // --- 6. Piece placement rings (concentric circles around centre) ---
    DrawCircleLinesV(ctr, m_pieceRingInner, kMark);
    DrawCircleLinesV(ctr, m_pieceRingOuter, kMark);

    // --- 7. Centre queen circle — small filled red circle ---
    DrawCircleV(ctr, m_centerCircleR, {198, 38, 38, 255});
    DrawCircleLinesV(ctr, m_centerCircleR, {130, 18, 18, 255});

    drawStrikerLines(board);
}

void CarromView::drawStrikerLines(const Board& board) const {
    static constexpr float kEdgeOffset  = 50.0f;
    static constexpr float kLineGap     = 20.0f;
    static constexpr float kCornerInset = 90.0f;

    const float outerOffset = kEdgeOffset;
    const float innerOffset = kEdgeOffset + kLineGap;
    const float spanStart   = board.left()   + kCornerInset;
    const float spanEnd     = board.right()  - kCornerInset;
    const float spanBottom  = board.bottom() + kCornerInset;
    const float spanTop     = board.top()    - kCornerInset;

    const float circleOuterR = kLineGap * 0.5f;
    const float circleInnerR = circleOuterR - 2.0f;

    auto line = [&](Vec2 a, Vec2 b) {
        DrawLineV(toScreen(a, m_padding, m_boardSize),
                  toScreen(b, m_padding, m_boardSize),
                  kMark);
    };
    auto endCircles = [&](Vec2 c) {
        Vector2 sc = toScreen(c, m_padding, m_boardSize);
        DrawCircleLinesV(sc, circleOuterR, kMark);
        DrawCircleLinesV(sc, circleInnerR, kMark);
    };

    const float hMid = outerOffset + kLineGap * 0.5f;
    const float vMid = outerOffset + kLineGap * 0.5f;

    // Bottom
    line({spanStart, board.bottom() + outerOffset}, {spanEnd, board.bottom() + outerOffset});
    line({spanStart, board.bottom() + innerOffset}, {spanEnd, board.bottom() + innerOffset});
    endCircles({spanStart, board.bottom() + hMid});
    endCircles({spanEnd,   board.bottom() + hMid});
    // Top
    line({spanStart, board.top() - outerOffset}, {spanEnd, board.top() - outerOffset});
    line({spanStart, board.top() - innerOffset}, {spanEnd, board.top() - innerOffset});
    endCircles({spanStart, board.top() - hMid});
    endCircles({spanEnd,   board.top() - hMid});
    // Left
    line({board.left() + outerOffset, spanBottom}, {board.left() + outerOffset, spanTop});
    line({board.left() + innerOffset, spanBottom}, {board.left() + innerOffset, spanTop});
    endCircles({board.left() + vMid, spanBottom});
    endCircles({board.left() + vMid, spanTop});
    // Right
    line({board.right() - outerOffset, spanBottom}, {board.right() - outerOffset, spanTop});
    line({board.right() - innerOffset, spanBottom}, {board.right() - innerOffset, spanTop});
    endCircles({board.right() - vMid, spanBottom});
    endCircles({board.right() - vMid, spanTop});
}

// ---------------------------------------------------------------------------
// Pockets — wooden rim + deep dark hole
// ---------------------------------------------------------------------------
void CarromView::drawPockets(const std::array<Pocket, 4>& pockets) const {
    for (const auto& p : pockets) {
        Vector2 sp = toScreen(p.position, m_padding, m_boardSize);
        DrawCircleV(sp, p.radius + 5.0f, {96, 58, 14, 255});   // outer wooden rim
        DrawCircleV(sp, p.radius + 2.0f, {42, 24,  6, 255});   // inner shadow ring
        DrawCircleV(sp, p.radius,         kPocketBg);            // the hole
    }
}

// ---------------------------------------------------------------------------
// Pieces — ivory white / dark black / red queen with gold rim
// ---------------------------------------------------------------------------
void CarromView::drawPieces(const std::vector<Circle2D>& pieces) const {
    for (const auto& c : pieces) {
        Vector2    sp = toScreen(c.position, m_padding, m_boardSize);
        const float r = c.radius;

        switch (c.type) {
            case PieceType::White:
                DrawCircleV(sp, r,            {242, 238, 222, 255}); // ivory base
                DrawCircleV(sp, r * 0.62f,    {252, 250, 245, 210}); // bright highlight
                DrawCircleLinesV(sp, r,        {165, 150, 125, 255}); // rim
                DrawCircleLinesV(sp, r - 3.5f, {185, 175, 155, 140}); // inner ring
                break;

            case PieceType::Black:
                DrawCircleV(sp, r,            { 32,  26,  20, 255}); // dark base
                DrawCircleV(sp, r * 0.42f,    { 60,  52,  42, 170}); // subtle sheen
                DrawCircleLinesV(sp, r,        { 72,  58,  42, 255}); // rim
                DrawCircleLinesV(sp, r - 3.5f, { 55,  45,  35, 120}); // inner ring
                break;

            case PieceType::Queen:
                DrawCircleV(sp, r,            {210,  35,  35, 255}); // red base
                DrawCircleV(sp, r * 0.55f,    {235,  75,  75, 180}); // inner highlight
                DrawCircleLinesV(sp, r,        {205, 162,  38, 255}); // gold rim
                DrawCircleLinesV(sp, r - 3.5f, {205, 162,  38, 160}); // gold inner ring
                break;

            default: break;
        }
    }
}

// ---------------------------------------------------------------------------
// Striker — layered steel-blue disc
// ---------------------------------------------------------------------------
void CarromView::drawStriker(const Circle2D& striker) const {
    Vector2    sp = toScreen(striker.position, m_padding, m_boardSize);
    const float r = striker.radius;
    DrawCircleV(sp, r,           { 80, 140, 200, 255}); // base
    DrawCircleV(sp, r * 0.66f,   {115, 172, 225, 200}); // highlight ring
    DrawCircleV(sp, r * 0.28f,   { 50, 100, 165, 255}); // centre dot
    DrawCircleLinesV(sp, r,       { 38,  82, 148, 255}); // outer rim
    DrawCircleLinesV(sp, r - 3.5f,{ 62, 112, 178, 180}); // inner rim
}

// ---------------------------------------------------------------------------
// Slider
// ---------------------------------------------------------------------------
void CarromView::drawSlider(const Circle2D& striker, bool active) const {
    static constexpr float kSliderHalfSpan = 210.0f;
    static constexpr float kSliderOffsetY  = 25.0f;
    static constexpr float kHandleRadius   = 10.0f;
    static constexpr float kHandleInnerR   = 8.0f;

    const float centerX = m_padding + m_boardSize * 0.5f;
    const float trackY  = m_padding + m_boardSize + kSliderOffsetY;
    const float trackX0 = centerX - kSliderHalfSpan;
    const float trackX1 = centerX + kSliderHalfSpan;

    const Color trackColor  = active ? kMark                       : Color{140,  88, 28, 80};
    const Color outerColor  = active ? kMark                       : Color{140,  88, 28, 80};
    const Color innerColor  = active ? kSurface                    : Color{180, 160, 110, 80};
    const Color borderColor = active ? Color{60, 32, 6, 255}       : Color{100,  70, 30, 80};

    DrawLineV({trackX0, trackY}, {trackX1, trackY}, trackColor);

    const float handleX = centerX + striker.position.x;
    DrawCircleV({handleX, trackY}, kHandleRadius, outerColor);
    DrawCircleV({handleX, trackY}, kHandleInnerR, innerColor);
    DrawCircleLinesV({handleX, trackY}, kHandleRadius, borderColor);
}

// ---------------------------------------------------------------------------
// Rules button & panel
// ---------------------------------------------------------------------------
void CarromView::drawRulesButton() const {
    DrawCircleV({kRulesBtnCX, kRulesBtnCY}, kRulesBtnR,  {50, 50, 50, 220});
    DrawCircleLinesV({kRulesBtnCX, kRulesBtnCY}, kRulesBtnR, LIGHTGRAY);
    DrawText("?", (int)(kRulesBtnCX - 4), (int)(kRulesBtnCY - 8), 16, WHITE);
}

void CarromView::drawRulesPanel() const {
    const Color kText = {50, 35, 20, 255};

    DrawRectangle(0, 0, (int)(m_boardSize + 2.0f * m_padding),
                        (int)(m_boardSize + 2.0f * m_padding), {0, 0, 0, 150});

    DrawRectangleRounded({kPanelX, kPanelY, kPanelW, kPanelH}, 0.04f, 8,
                         {240, 230, 210, 250});
    DrawRectangleRoundedLines({kPanelX, kPanelY, kPanelW, kPanelH}, 0.04f, 8,
                              DARKBROWN);

    DrawCircleV({kCloseBtnCX, kCloseBtnCY}, kCloseBtnR, {160, 60, 60, 230});
    DrawText("x", (int)(kCloseBtnCX - 5), (int)(kCloseBtnCY - 8), 16, WHITE);

    const char* title = "CARROM RULES";
    int titleW = MeasureText(title, 26);
    DrawText(title, (int)(kPanelX + kPanelW * 0.5f) - titleW / 2,
             (int)(kPanelY + 18), 26, DARKBROWN);

    const float divY  = kPanelY + 54.0f;
    const float textX = kPanelX + 28.0f;
    DrawLineEx({kPanelX + 20, divY}, {kPanelX + kPanelW - 20, divY}, 1.5f, DARKBROWN);

    const float row0Y = divY + 16.0f;
    const float chipR = 7.0f;
    const float chipGap = 6.0f;
    const int   fs = 16;

    const char* sYouPlay = "You play ";
    DrawText(sYouPlay, (int)textX, (int)row0Y, fs, kText);
    float cx = textX + MeasureText(sYouPlay, fs) + chipR + chipGap;
    DrawCircleV({cx, row0Y + chipR}, chipR, {245, 245, 245, 255});
    DrawCircleLinesV({cx, row0Y + chipR}, chipR, DARKGRAY);
    float afterChip = cx + chipR + chipGap;
    const char* sWhite = "White";
    DrawText(sWhite, (int)afterChip, (int)row0Y, fs, kText);
    afterChip += MeasureText(sWhite, fs);
    const char* sSep = "   |   AI plays ";
    DrawText(sSep, (int)afterChip, (int)row0Y, fs, kText);
    afterChip += MeasureText(sSep, fs);
    float bcx = afterChip + chipR + chipGap;
    DrawCircleV({bcx, row0Y + chipR}, chipR, {30, 30, 30, 255});
    DrawCircleLinesV({bcx, row0Y + chipR}, chipR, DARKGRAY);
    DrawText("Black", (int)(bcx + chipR + chipGap), (int)row0Y, fs, kText);

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
    float y = row0Y + kLineH + 6.0f;
    for (const auto* ln : kRules) {
        if (ln[0] != '\0')
            DrawText(ln, (int)textX, (int)y, fs, kText);
        y += kLineH;
    }
}

// ---------------------------------------------------------------------------
// Particles & aim line
// ---------------------------------------------------------------------------
void CarromView::drawParticles(const std::vector<Particle>& particles) const {
    for (const auto& p : particles) {
        const float t     = p.life / p.maxLife;
        const auto  alpha = static_cast<uint8_t>(t * 210.0f);
        const float r     = p.radius * (0.4f + 0.6f * t);
        DrawCircleV(toScreen(p.pos, m_padding, m_boardSize), r, {p.r, p.g, p.b, alpha});
    }
}

void CarromView::drawAimLine(Vec2 from, Vec2 to, float powerRadius) const {
    Vector2 sp = toScreen(from, m_padding, m_boardSize);
    DrawCircleLinesV(sp, powerRadius, {70, 130, 180, 200});
    DrawLineV(sp, toScreen(to, m_padding, m_boardSize), {70, 130, 180, 200});
}
