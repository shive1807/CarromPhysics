#include "raylib.h"
#include "Simulation.h"
#include "CarromLayout.h"
#include "CarromView.h"
#include "CarromPresenter.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

constexpr int   kWindowSize = 700;
constexpr float kBoardSize  = 600.0f;

// File-scope so the Emscripten callback can reach them
static Simulation*      g_sim       = nullptr;
static CarromPresenter* g_presenter = nullptr;

static void gameLoop() {
    g_presenter->update(GetFrameTime());
}

int main() {
    static Simulation sim(Board{{0.0f, 0.0f}, kBoardSize});
    for (const auto& piece : makeCarromLayout())
        sim.addPiece(piece);

    InitWindow(kWindowSize, kWindowSize, "CarromPhysics");
    SetTargetFPS(60);

    static CarromView      view(kWindowSize, kBoardSize);
    static CarromPresenter presenter(sim, view, kWindowSize, kBoardSize);

    g_sim       = &sim;
    g_presenter = &presenter;

#ifdef __EMSCRIPTEN__
    // Hand control to the browser — it calls gameLoop() each animation frame.
    // The last arg (1) means simulate infinite loop; execution never returns here.
    emscripten_set_main_loop(gameLoop, 0, 1);
#else
    while (!WindowShouldClose())
        gameLoop();

    CloseWindow();
#endif

    return 0;
}
