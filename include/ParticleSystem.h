#pragma once

#include "Vec2.h"
#include <vector>
#include <cstdint>

struct Particle {
    Vec2    pos;
    Vec2    vel;
    float   life;
    float   maxLife;
    float   radius;
    uint8_t r, g, b;
};

class ParticleSystem {
public:
    // Emit a comet-tail dot behind a moving piece (no-op below speed threshold)
    void emitTrail(Vec2 pos, Vec2 vel, uint8_t r, uint8_t g, uint8_t b);

    // Radial spark burst at a collision point, sized by impulse magnitude
    void emitBurst(Vec2 pos, float impulse);

    void update(float dt);
    void clear();

    [[nodiscard]] const std::vector<Particle>& particles() const { return m_particles; }

private:
    void add(Particle p);
    std::vector<Particle> m_particles;
};
