#pragma once

#include <stdint.h>

namespace ld {
  struct Camera {
    int32_t y = 0;
    float yVelocity = 0.0f;
    bool surfaceSnapped = false;
    float surfaceSnapCooldown = 0.0f;

    void Update();
  };
}
