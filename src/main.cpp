#include "raylib.h"
#include "Simulation.h"

#include <vector>

constexpr int   kWindowSize = 700;
constexpr float kBoardSize  = 600.0f;
constexpr float kPadding    = (kWindowSize - kBoardSize) * 0.5f;

static Vector2 toScreen(Vec2 p) {
    return {
        kPadding + kBoardSize * 0.5f + p.x,
        kPadding + kBoardSize * 0.5f - p.y,
    };
}

static void drawBoard(const Board& board) {
    DrawRectangle((int)kPadding, (int)kPadding, (int)board.size, (int)board.size, {210, 180, 140, 255});
    DrawRectangleLines((int)kPadding, (int)kPadding, (int)board.size, (int)board.size, DARKBROWN);
}

static void drawPieces(const std::vector<Circle2D>& pieces) {
    for (const auto& c : pieces) {
        Vector2 sp = toScreen(c.position);
        DrawCircleV(sp, c.radius, DARKGRAY);
        DrawCircleLines((int)sp.x, (int)sp.y, (int)c.radius, BLACK);
    }
}

int main() {
    Simulation sim(Board{{0.0f, 0.0f}, kBoardSize});
    sim.addPiece({{  0.0f,   0.0f}, { 250.0f,   0.0f}, 20.0f});
    sim.addPiece({{ 50.0f,   0.0f}, {-250.0f,   0.0f}, 20.0f});
    sim.addPiece({{  0.0f,  80.0f}, { 150.0f, -200.0f}, 20.0f});
    sim.addPiece({{  0.0f, -80.0f}, {-130.0f,  160.0f}, 20.0f});

    InitWindow(kWindowSize, kWindowSize, "CarromPhysics");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        sim.step(GetFrameTime());

        BeginDrawing();
            ClearBackground(RAYWHITE);
            drawBoard(sim.board());
            drawPieces(sim.pieces());
            DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
