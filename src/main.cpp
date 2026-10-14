#include "raylib.h"
#include "Circle2D.h"
#include "Board.h"
#include "Physics.h"

#include <vector>

static const int WINDOW = 700;
static const float BOARD_SIZE = 600.0f;
static const float PADDING = (WINDOW - BOARD_SIZE) / 2.0f;

// Convert simulation coords (center-origin) to screen coords
static Vector2 toScreen(Vec2 p) {
    return { PADDING + BOARD_SIZE * 0.5f + p.x,
             PADDING + BOARD_SIZE * 0.5f - p.y };
}

int main() {
    Board board{{0.0f, 0.0f}, BOARD_SIZE};

    std::vector<Circle2D> pieces = {
        {{  0.0f,   0.0f}, {220.0f, 180.0f}, 20.0f, 1.0f},
        {{100.0f,  80.0f}, {-180.0f, 220.0f}, 20.0f, 1.0f},
        {{-120.0f, 60.0f}, {150.0f, -200.0f}, 20.0f, 1.0f},
        {{ 80.0f, -100.0f}, {-130.0f, 160.0f}, 20.0f, 1.0f},
    };

    InitWindow(WINDOW, WINDOW, "CarromPhysics");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        for (auto& p : pieces)
            stepCircle(p, board, dt);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Board
        DrawRectangle((int)PADDING, (int)PADDING, (int)BOARD_SIZE, (int)BOARD_SIZE, { 210, 180, 140, 255 });
        DrawRectangleLines((int)PADDING, (int)PADDING, (int)BOARD_SIZE, (int)BOARD_SIZE, DARKBROWN);

        // Pieces
        for (const auto& p : pieces) {
            Vector2 sp = toScreen(p.position);
            DrawCircleV(sp, p.radius, DARKGRAY);
            DrawCircleLines((int)sp.x, (int)sp.y, p.radius, BLACK);
        }

        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
