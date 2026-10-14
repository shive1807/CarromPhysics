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
        case PieceType::Queen: return {220,  40,  40, 255};
        case PieceType::Black: return { 30,  30,  30, 255};
        case PieceType::White: return {235, 230, 210, 255};
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

void CarromView::drawFrame(const CarromModel& model) const {
    BeginDrawing();
        ClearBackground(RAYWHITE);
        drawBoard(model.board);
        drawPockets(model.pockets);
        drawPieces(model.pieces);
        DrawFPS(10, 10);
    EndDrawing();
}

void CarromView::drawBoard(const Board& board) const {
    DrawRectangle((int)m_padding, (int)m_padding, (int)board.size, (int)board.size, {210, 180, 140, 255});
    DrawRectangleLines((int)m_padding, (int)m_padding, (int)board.size, (int)board.size, DARKBROWN);

    Vector2 center = toScreen(board.center, m_padding, m_boardSize);
    DrawCircleLinesV(center, m_centerCircleR,  DARKBROWN);
    DrawCircleLinesV(center, m_pieceRingInner, DARKBROWN);
    DrawCircleLinesV(center, m_pieceRingOuter, DARKBROWN);
}

void CarromView::drawPockets(const std::array<Pocket, 4>& pockets) const {
    for (const auto& p : pockets)
        DrawCircleV(toScreen(p.position, m_padding, m_boardSize), p.radius, BLACK);
}

void CarromView::drawPieces(const std::vector<Circle2D>& pieces) const {
    for (const auto& c : pieces) {
        Vector2 sp = toScreen(c.position, m_padding, m_boardSize);
        DrawCircleV(sp, c.radius, colorOf(c.type));
        DrawCircleLines((int)sp.x, (int)sp.y, (int)c.radius, BLACK);
    }
}
