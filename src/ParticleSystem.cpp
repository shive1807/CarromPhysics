#include "ParticleSystem.h"

#include <algorithm>
#include <cmath>
#include <random>

static constexpr float kPi = 3.14159265f;

static std::mt19937& rng() {
    static std::mt19937 s{42};
    return s;
}

static float randf(float lo, float hi) {
    return std::uniform_real_distribution<float>{lo, hi}(rng());
}

void ParticleSystem::add(Particle p) {
    m_particles.push_back(p);
}

void ParticleSystem::emitTrail(Vec2 pos, Vec2 vel, uint8_t r, uint8_t g, uint8_t b) {
    const float speed = vel.length();
    if (speed < 25.0f)
        return;

    // Two dots per call — scattered slightly behind and around the piece
    for (int i = 0; i < 2; ++i) {
        Vec2 behind = vel.normalized() * randf(-6.0f, -1.0f);
        Vec2 scatter{randf(-3.0f, 3.0f), randf(-3.0f, 3.0f)};

        Particle p;
        p.pos     = pos + behind + scatter;
        p.vel     = vel * randf(0.02f, 0.08f);  // nearly stationary
        p.life    = randf(0.12f, 0.28f);
        p.maxLife = p.life;
        p.radius  = randf(2.0f, 4.5f);
        p.r = r; p.g = g; p.b = b;
        add(p);
    }
}

void ParticleSystem::emitBurst(Vec2 pos, float impulse) {
    const int   count = static_cast<int>(std::clamp(impulse * 0.6f, 5.0f, 18.0f));
    const float speed = std::clamp(impulse * 10.0f, 60.0f, 350.0f);

    for (int i = 0; i < count; ++i) {
        const float angle = (2.0f * kPi * i) / count + randf(-0.4f, 0.4f);
        const float s     = randf(0.4f, 1.0f) * speed;

        Particle p;
        p.pos     = pos;
        p.vel     = {std::cos(angle) * s, std::sin(angle) * s};
        p.life    = randf(0.15f, 0.40f);
        p.maxLife = p.life;
        p.radius  = randf(2.5f, 5.5f);
        // Orange–yellow spark gradient, randomised per particle
        p.r = 255;
        p.g = static_cast<uint8_t>(randf(140.0f, 220.0f));
        p.b = static_cast<uint8_t>(randf(0.0f,   60.0f));
        add(p);
    }
}

void ParticleSystem::update(float dt) {
    for (auto& p : m_particles) {
        p.pos  += p.vel * dt;
        p.vel  *= std::max(0.0f, 1.0f - 6.0f * dt);  // drag — frame-rate approximate is fine
        p.life -= dt;
    }

    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
                       [](const Particle& p) { return p.life <= 0.0f; }),
        m_particles.end());
}

void ParticleSystem::clear() {
    m_particles.clear();
}
