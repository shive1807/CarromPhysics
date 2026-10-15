#pragma once
#include "raylib.h"

class SoundSystem {
public:
    SoundSystem();
    ~SoundSystem();

    void playCollision(float impulse);  // impulse scales volume
    void playPocket();
    void playShoot();
    void playGameOver(bool won);

    SoundSystem(const SoundSystem&)            = delete;
    SoundSystem& operator=(const SoundSystem&) = delete;

private:
    Sound m_collision;
    Sound m_pocket;
    Sound m_shoot;
    Sound m_win;
    Sound m_lose;
};
