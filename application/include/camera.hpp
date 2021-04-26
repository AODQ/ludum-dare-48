#pragma once

#include <stdint.h>

namespace ld { struct GameState; }

namespace ld {
  struct Camera {
    int32_t y = 0;
    float yVelocity = 0.0f;
    float surfaceSnapCooldown = 0.0f;

    void Update(ld::GameState & state);
  };
}
