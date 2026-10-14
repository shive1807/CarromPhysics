#include "raylib.h"
#include "Simulation.h"
#include "CarromLayout.h"

#include <array>
#include <vector>

constexpr int   kWindowSize       = 700;
constexpr float kBoardSize        = 600.0f;
constexpr float kPadding          = (kWindowSize - kBoardSize) * 0.5f;
constexpr float kCenterCircleR    = 55.0f;  // decorative ring radius

static Vector2 toScreen(Vec2 p) {
    return {
        kPadding + kBoardSize * 0.5f + p.x,
        kPadding + kBoardSize * 0.5f - p.y,
    };
}

static Color colorOf(PieceType type) {
    switch (type) {
        case PieceType::Queen: return {220,  40,  40, 255};   // red
        case PieceType::Black: return { 30,  30,  30, 255};   // near-black
        case PieceType::White: return {235, 230, 210, 255};   // cream
    }
    return WHITE;
}

static void drawBoard(const Board& board) {
    DrawRectangle((int)kPadding, (int)kPadding, (int)board.size, (int)board.size, {210, 180, 140, 255});
    DrawRectangleLines((int)kPadding, (int)kPadding, (int)board.size, (int)board.size, DARKBROWN);

    // Decorative center circle
    Vector2 center = toScreen(board.center);
    DrawCircleLinesV(center, kCenterCircleR, DARKBROWN);
}

static void drawPockets(const std::array<Pocket, 4>& pockets) {
    for (const auto& p : pockets) {
        DrawCircleV(toScreen(p.position), p.radius, BLACK);
    }
}

static void drawPieces(const std::vector<Circle2D>& pieces) {
    for (const auto& c : pieces) {
        Vector2 sp    = toScreen(c.position);
        Color   fill  = colorOf(c.type);
        DrawCircleV(sp, c.radius, fill);
        DrawCircleLines((int)sp.x, (int)sp.y, (int)c.radius, BLACK);
    }
}

int main() {
    Simulation sim(Board{{0.0f, 0.0f}, kBoardSize});
    for (const auto& piece : makeCarromLayout())
        sim.addPiece(piece);

    InitWindow(kWindowSize, kWindowSize, "CarromPhysics");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        sim.step(GetFrameTime());

        BeginDrawing();
            ClearBackground(RAYWHITE);
            drawBoard(sim.board());
            drawPockets(sim.pockets());
            drawPieces(sim.pieces());
            DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
