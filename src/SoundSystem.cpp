#include "SoundSystem.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

static constexpr float kSampleRate = 44100.0f;
static constexpr float k2Pi        = 6.28318530f;

// Build a mono 16-bit Sound from a set of sine-wave harmonics with an
// exponential-decay amplitude envelope.  `freqs` and `amps` must have
// `count` entries.  `decayRate` controls how fast the sound fades: a value
// of 30 means ~97 % gone after 100 ms (e^{-30×0.1}).
static Sound buildSound(const float* freqs, const float* amps, int count,
                        float durationSec, float decayRate)
{
    const int frames = static_cast<int>(kSampleRate * durationSec);
    auto* pcm = static_cast<int16_t*>(std::malloc(frames * sizeof(int16_t)));

    for (int i = 0; i < frames; ++i) {
        const float t   = static_cast<float>(i) / kSampleRate;
        const float env = std::expf(-decayRate * t);
        float val = 0.0f;
        for (int h = 0; h < count; ++h)
            val += amps[h] * std::sinf(k2Pi * freqs[h] * t);
        val *= env;
        val  = std::clamp(val, -1.0f, 1.0f);
        pcm[i] = static_cast<int16_t>(val * 32767.0f);
    }

    Wave w = {};
    w.frameCount = static_cast<unsigned int>(frames);
    w.sampleRate = static_cast<unsigned int>(kSampleRate);
    w.sampleSize = 16;
    w.channels   = 1;
    w.data       = pcm;

    Sound snd = LoadSoundFromWave(w);
    std::free(pcm);
    return snd;
}

// ---------------------------------------------------------------------------
// SoundSystem
// ---------------------------------------------------------------------------
SoundSystem::SoundSystem()
{
    // Woody piece-on-piece click: fundamental + harmonics, fast decay
    {
        const float f[] = {600.0f, 1200.0f, 2000.0f, 3200.0f};
        const float a[] = {0.50f,  0.28f,   0.14f,   0.08f};
        m_collision = buildSound(f, a, 4, 0.12f, 38.0f);
    }
    // Hollow pocket thud: low fundamental, slower decay
    {
        const float f[] = {180.0f, 360.0f, 550.0f};
        const float a[] = {0.55f,  0.30f,  0.15f};
        m_pocket = buildSound(f, a, 3, 0.30f, 10.0f);
    }
    // Sharp snap on striker release
    {
        const float f[] = {900.0f, 1800.0f, 3000.0f};
        const float a[] = {0.45f,  0.35f,   0.20f};
        m_shoot = buildSound(f, a, 3, 0.07f, 65.0f);
    }
    // Win: bright major-chord shimmer (C5–E5–G5)
    {
        const float f[] = {523.0f, 659.0f, 784.0f};
        const float a[] = {0.36f,  0.36f,  0.28f};
        m_win = buildSound(f, a, 3, 0.85f, 2.5f);
    }
    // Lose: descending minor tones (A4–F#4–D4)
    {
        const float f[] = {440.0f, 370.0f, 294.0f};
        const float a[] = {0.36f,  0.36f,  0.28f};
        m_lose = buildSound(f, a, 3, 0.85f, 2.5f);
    }
}

SoundSystem::~SoundSystem()
{
    UnloadSound(m_collision);
    UnloadSound(m_pocket);
    UnloadSound(m_shoot);
    UnloadSound(m_win);
    UnloadSound(m_lose);
}

void SoundSystem::playCollision(float impulse)
{
    SetSoundVolume(m_collision, std::clamp(impulse / 80.0f, 0.08f, 1.0f));
    PlaySound(m_collision);
}

void SoundSystem::playPocket()
{
    SetSoundVolume(m_pocket, 0.85f);
    PlaySound(m_pocket);
}

void SoundSystem::playShoot()
{
    SetSoundVolume(m_shoot, 0.70f);
    PlaySound(m_shoot);
}

void SoundSystem::playGameOver(bool won)
{
    Sound& snd = won ? m_win : m_lose;
    SetSoundVolume(snd, 0.80f);
    PlaySound(snd);
}
