#include "raylib.h"
#include "Simulation.h"
#include "CarromLayout.h"
#include "CarromView.h"

constexpr int   kWindowSize = 700;
constexpr float kBoardSize  = 600.0f;

int main() {
    Simulation sim(Board{{0.0f, 0.0f}, kBoardSize});
    for (const auto& piece : makeCarromLayout())
        sim.addPiece(piece);

    InitWindow(kWindowSize, kWindowSize, "CarromPhysics");
    SetTargetFPS(60);

    CarromView view(kWindowSize, kBoardSize);

    while (!WindowShouldClose()) {
        sim.step(GetFrameTime());
        view.drawFrame(sim);
    }

    CloseWindow();
    return 0;
}
