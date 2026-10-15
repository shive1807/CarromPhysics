cmake_minimum_required(VERSION 3.20)
project(CarromPhysics VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/Simulation.cpp
    src/CarromLayout.cpp
    src/CarromView.cpp
    src/CarromPresenter.cpp
    src/CarromAI.cpp
    src/ParticleSystem.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# ── Web / Emscripten target ─────────────────────────────────────────────────
if(EMSCRIPTEN)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")

    target_link_options(${PROJECT_NAME} PRIVATE
        -sUSE_GLFW=3                 # GLFW from Emscripten ports (no source build)
        -sWASM=1
        -sALLOW_MEMORY_GROWTH=1
        -sTOTAL_MEMORY=67108864      # 64 MB initial heap
        -sGL_ENABLE_GET_PROC_ADDRESS=1
        -sASSERTIONS=0               # disable for smaller/faster release build
        --shell-file ${CMAKE_SOURCE_DIR}/web/shell.html
    )
endif()
