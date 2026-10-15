<div align="center">

<img src="https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcQNjim729SDv4X3c0MzrFrCPoISuKgYnEKVR7MHKNdutg&s=10" alt="Carrom Physics" width="600"/>

# 🎯 Carrom Physics

*A fully physics-driven Carrom board game built from scratch in C++ — runs natively and in the browser*

<br>

[![▶ Play Now — No Install Required](https://img.shields.io/badge/▶%20%20Play%20Now%20%E2%80%94%20No%20Install%20Required-1a7f37?style=for-the-badge&logo=googlechrome&logoColor=white&labelColor=1a7f37)](https://shive1807.github.io/CarromPhysics/)

<br>

[![Build & Deploy](https://github.com/shive1807/CarromPhysics/actions/workflows/deploy.yml/badge.svg)](https://github.com/shive1807/CarromPhysics/actions/workflows/deploy.yml)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue?logo=cplusplus)](https://en.cppreference.com/w/cpp/20)
[![raylib 5.5](https://img.shields.io/badge/raylib-5.5-orange)](https://www.raylib.com/)
[![WebAssembly](https://img.shields.io/badge/WebAssembly-654FF0?logo=webassembly&logoColor=white)](https://webassembly.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

</div>

---

## ✨ What is this?

**Carrom Physics** is a from-scratch simulation of the classic South Asian board game Carrom, built entirely in C++ without any game engine. Every aspect of the game — physics, collision resolution, AI, particle effects, and rendering — is hand-crafted.

The game compiles natively on macOS and Linux and also targets **WebAssembly** via Emscripten, letting anyone play it instantly in a browser with no installation.

---

## 🎮 Play Now

> **Just click the button below — no download, no install.**

<div align="center">

[![▶ Open in Browser](https://img.shields.io/badge/▶%20Open%20in%20Browser-Play%20Carrom%20Physics-success?style=for-the-badge&logo=googlechrome)](https://shive1807.github.io/CarromPhysics/)

*Works on Chrome, Firefox, Safari, Edge — desktop only*

</div>

---

## 🖼️ Features at a Glance

| Feature | Details |
|---|---|
| ⚙️ **Custom Physics** | Iterative elastic circle–circle & wall collisions, kinetic + static friction |
| 🤖 **AI Opponent** | Ghost-ball method with line-of-sight obstacle detection & power calibration |
| ✨ **Particle Effects** | Motion trails and collision spark bursts with alpha-fade & shrink |
| 🎯 **Slingshot Aiming** | Drag the striker to aim; a power ring grows as you pull further back |
| 🎪 **Full Game Rules** | Queen cover, last-piece rule, bonus turns, win detection |
| 🖱️ **Slider UI** | Slide the striker along its baseline line with a draggable handle |
| 📱 **WebAssembly** | Compiled with Emscripten — runs at native speed in any modern browser |
| 🏗️ **MVP Architecture** | Clean Model / View / Presenter separation throughout |

---

## 🕹️ How to Play

### Your Goal
You are **Player 1 — White pieces ⚪**. The AI plays **Black pieces ⚫**. Pot all your white pieces and secure the Queen to win.

### Controls

| Action | How to do it |
|---|---|
| **Position striker** | Drag the circular slider handle at the bottom of the board |
| **Aim** | Click and hold on the striker, then drag in the opposite direction you want to shoot |
| **Shoot** | Release the mouse button — the further you dragged, the more power |
| **Power indicator** | The ring around the striker grows from 1× to 2× radius as you pull back |
| **View Rules** | Click the **`?`** button in the top-right corner |

### Game Flow

```
Your turn (bottom baseline)  →  Shoot  →  Pieces settle
  ↓ potted own piece?               ↓ no own piece potted?
  Bonus turn!                       AI's turn (top baseline)
```

---

## 📜 Official Rules (as implemented)

<details>
<summary><b>Click to expand full rules</b></summary>

<br>

**1. Pieces**
- 9 ⚫ Black pieces (AI), 9 ⚪ White pieces (You), 1 🔴 Queen
- Striker is the large steel-blue disc — it's the only piece you shoot

**2. Taking a shot**
- Position the striker within the two baseline lines at your side of the board
- Drag and release to shoot

**3. Scoring your pieces**
- If you pocket one or more of your own colour in a turn → **bonus turn**, shoot again
- Pocketing the opponent's piece counts for nothing (no penalty, but no bonus)

**4. The Queen 👑**
- Either player can pocket the Queen at any time
- When you pocket the Queen you **must also pocket one of your own colour in the same turn** (the "cover")
- Fail to cover → **Queen returns to the centre**
- Once covered, the Queen is yours and stays off the board

**5. Last piece rule**
- You **cannot win by pocketing your last piece** until the Queen has been secured
- If you pocket your last piece with the Queen still on the board → the piece returns to centre

**6. Winning**
- First player to pocket all pieces of their colour **and** have the Queen secured wins

</details>

---

## 🧠 Technical Deep-Dive

### Architecture — MVP Pattern

```
┌─────────────────────────────────────────────────┐
│                 CarromPresenter                  │
│  (input handling · state machine · game rules)  │
└────────────────┬───────────────┬────────────────┘
                 │               │
        ┌────────▼──────┐ ┌──────▼────────┐
        │  CarromModel  │ │  CarromView   │
        │  (pure data)  │ │  (rendering)  │
        └───────────────┘ └───────────────┘
                 │
        ┌────────▼──────┐
        │  Simulation   │
        │  (physics)    │
        └───────────────┘
```

| Class | Responsibility |
|---|---|
| `CarromModel` | Pure data — board, pieces, pockets, striker position |
| `Simulation` | Physics step: integrate → wall resolve → circle collisions ×8 → friction |
| `CarromView` | All rendering via raylib: board, pieces, particles, UI overlays |
| `CarromPresenter` | Input, state machine, game rule enforcement, turn management |
| `CarromAI` | Shot selection: ghost ball + obstacle detection + power calibration |
| `ParticleSystem` | Trail and burst particles with lifetime, drag and fade |

---

### ⚙️ Physics Engine

The physics runs at 60 Hz with the following step order (order matters to prevent tunnelling and sticking):

```
1. Integrate   — position += velocity × dt
2. Wall clamp  — reflect velocity on board edges
3. Collisions  — resolve circle pairs × 8 iterations
4. Friction    — kinetic decay then zero below static threshold
5. Pockets     — remove pieces that entered a pocket region
```

**Elastic circle–circle collision** (equal restitution):

```
normal     = normalize(b.pos − a.pos)
relVel     = (va − vb) · normal          // positive = approaching
impulse    = 2 × relVel / (ma + mb)
va        −= normal × impulse × mb
vb        += normal × impulse × ma
```

Overlapping pairs are also positionally corrected proportional to their mass ratio to prevent objects sinking into each other across multiple frames.

---

### 🤖 AI System

The AI uses the **ghost ball method** — a technique from billiards:

```
pocketDir  = normalize(pocket − piece)
ghostBall  = piece.pos − pocketDir × (piece.r + striker.r)
```

The striker must travel to `ghostBall`, at which point it is perfectly aligned to redirect the piece into the pocket.

**Shot validation pipeline** (each check can reject the shot):

```
1. Reachability  — ghost ball must be in front of the striker's baseline
2. Path clear A  — no piece blocks the striker → ghost ball line
3. Path clear B  — no piece blocks the piece → pocket line
4. Power calc    — velocity = f(striker travel + piece-to-pocket distance)
5. Scoring       — prefer short piece-to-pocket, penalise long striker travel
```

When no clean potting shot is found the AI falls back to a **setup shot** — it aims directly at the piece closest to any pocket to advance the board position.

---

### ✨ Particle System

Two emitter types, both in sim-space coordinates converted to screen-space at draw time:

| Emitter | Trigger | Particles | Lifetime |
|---|---|---|---|
| **Trail** | Any piece moving > 25 units/s | 2 per piece per frame, scattered behind the velocity vector | 0.12 – 0.28 s |
| **Burst** | Collision impulse > 5 units | 5 – 18 radial sparks, orange–yellow gradient, count scales with impulse | 0.15 – 0.40 s |

Particles alpha-fade and shrink as `t = life / maxLife → 0`, drawn **below** pieces but above the board using a split draw pass:

```
beginFrame → drawBackground → drawParticles → drawForeground → UI → endFrame
```

---

### 🌐 WebAssembly Build

The game compiles to WebAssembly with **zero source changes** thanks to a compile-time guard:

```cpp
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(gameLoop, 0, 1);  // browser owns the event loop
#else
    while (!WindowShouldClose()) gameLoop();   // native owns the event loop
    CloseWindow();
#endif
```

raylib automatically switches from OpenGL 4.1 → WebGL 2 when targeting Emscripten. GLFW is provided by Emscripten's ports (`-sUSE_GLFW=3`) rather than compiled from source.

---

## 🗂️ Project Structure

```
CarromPhysics/
├── include/
│   ├── Vec2.h              # 2D math primitive
│   ├── PieceType.h         # Queen / Black / White / Striker enum
│   ├── Circle2D.h          # Physics body (pos, vel, radius, mass, type)
│   ├── Board.h             # Board bounds helper
│   ├── Pocket.h            # Corner pocket with contains() test
│   ├── CarromModel.h       # Pure data aggregate
│   ├── Simulation.h        # Physics engine interface
│   ├── CarromLayout.h      # Initial piece placement
│   ├── CarromView.h        # Renderer interface
│   ├── CarromPresenter.h   # Game controller + state machine
│   ├── CarromAI.h          # AI shot selector
│   └── ParticleSystem.h    # Trail & burst particle system
├── src/
│   ├── main.cpp            # Entry point (native + Emscripten)
│   ├── Simulation.cpp      # Physics step implementation
│   ├── CarromLayout.cpp    # Ring-based piece layout
│   ├── CarromView.cpp      # raylib draw calls
│   ├── CarromPresenter.cpp # Input, state machine, rules
│   ├── CarromAI.cpp        # Ghost ball AI
│   └── ParticleSystem.cpp  # Particle update & emission
├── web/
│   └── shell.html          # Emscripten HTML shell
├── .github/
│   └── workflows/
│       └── deploy.yml      # CI: Emscripten build → GitHub Pages
└── CMakeLists.txt          # CMake build (native + web)
```

---

## 🛠️ Build Locally

### Prerequisites

- CMake ≥ 3.20
- C++20 compiler (AppleClang 15, GCC 12, Clang 14+)
- Git (CMake fetches raylib automatically)

### Native

```bash
git clone https://github.com/shive1807/CarromPhysics.git
cd CarromPhysics

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

./build/CarromPhysics
```

### Web (Emscripten)

```bash
# Install emsdk (one-time)
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
cd ~/emsdk && ./emsdk install 3.1.64 && ./emsdk activate 3.1.64
source ~/emsdk/emsdk_env.sh
cd -

# Build
emcmake cmake -B web-build -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Release
emmake cmake --build web-build --parallel

# Serve (browsers require HTTP for WASM)
python3 -m http.server 8080 --directory web-build
# → open http://localhost:8080
```

---

## 🚀 CI / CD

Every push to `main` triggers the GitHub Actions workflow:

```
Push to main
    └─► Install Emscripten (cached)
    └─► emcmake cmake + emmake build (deps cached)
    └─► Upload web-build/ as Pages artifact
    └─► Deploy to GitHub Pages
```

Live URL updates within ~2 minutes of each push.

---

## 📄 License

MIT — see [LICENSE](LICENSE)

---

<div align="center">

Built with ❤️ using **C++20** · **raylib** · **Emscripten**

[![▶ Play Now](https://img.shields.io/badge/▶%20Play%20Now-shive1807.github.io%2FCarromPhysics-brightgreen?style=for-the-badge)](https://shive1807.github.io/CarromPhysics/)

</div>
