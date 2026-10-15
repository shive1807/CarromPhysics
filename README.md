<div align="center">

# 🎯 Carrom Physics

**A physics-based Carrom board game — C++ · raylib · WebAssembly**

<br>

[![Play Now](https://img.shields.io/badge/▶%20Play%20Now-shive1807.github.io-brightgreen?style=for-the-badge&logo=googlechrome&logoColor=white)](https://shive1807.github.io/CarromPhysics/)

<br>

[![Build & Deploy](https://github.com/shive1807/CarromPhysics/actions/workflows/deploy.yml/badge.svg)](https://github.com/shive1807/CarromPhysics/actions/workflows/deploy.yml)

</div>

---

## Gameplay

| | Player | AI |
|---|---|---|
| **Pieces** | ⚪ White | ⚫ Black |
| **Baseline** | Bottom | Top |
| **Controls** | Drag striker to aim, release to shoot | Automatic |

**Rules**
- Pot your own colour to score. Potting your own piece earns a **bonus turn**.
- Pot the **Queen** and immediately pot one of your own pieces in the same turn to **secure** it. Fail the cover and the Queen returns to centre.
- You cannot pot your **last piece** until the Queen is secured.
- First player to clear all their pieces with the Queen secured **wins**.

---

## Controls

| Action | How |
|---|---|
| Move striker | Drag the slider at the bottom |
| Aim & shoot | Click and drag the striker — release to fire |
| Power | Drag further for more power (ring around striker shows level) |
| Rules | Click the **?** button (top-right) |

---

## Tech

- **Language** C++20
- **Rendering** [raylib 5.5](https://www.raylib.com/) (OpenGL 4.1 / WebGL 2)
- **Physics** Custom elastic collision engine with kinetic + static friction
- **AI** Ghost-ball method with line-of-sight obstacle detection
- **Web** Compiled to WebAssembly via [Emscripten](https://emscripten.org/)
- **Build** CMake + FetchContent

---

## Build locally

**Native (macOS / Linux)**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/CarromPhysics
```

**Web (requires [emsdk](https://emscripten.org/docs/getting_started/downloads.html))**
```bash
emcmake cmake -B web-build -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Release
emmake cmake --build web-build --parallel
python3 -m http.server 8080 --directory web-build
# open http://localhost:8080
```
