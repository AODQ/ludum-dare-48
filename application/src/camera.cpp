#include <camera.hpp>

#include <raylib.h>

#include <cmath>

void ld::Camera::Update() {
  auto & self = *this;

  if (self.surfaceSnapCooldown > 0.0f) {
    self.surfaceSnapCooldown -= 1.0f/60.0f;
    return;
  }

  // snap surface
  if (self.y > -32.0f && self.y < 0.0f && self.yVelocity == 0.0f) {
    self.y = 0.0f;
  }

  self.yVelocity += GetMouseWheelMove()*16.0f;

  self.y -= self.yVelocity;

  if (self.y < -612.0f) {
    self.y = -612.0f;
    self.yVelocity = fmin(self.yVelocity, 0.0f);
  }

  if (abs(self.yVelocity) < 1.0f) self.yVelocity = 0.0f;

  if (self.y < 0.0f)
    self.yVelocity *= 0.95f;
  else
    self.yVelocity *= 0.85f;
}
