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
  if (self.y > -128.0f && self.y < 0.0f && self.yVelocity == 0.0f) {
    self.y = 0.0f;
  }

  // check if user wants to goto surface
  if (!surfaceSnapped && self.y < -64.0f) {
    self.y = -612.0f;
    surfaceSnapped = true;
    self.yVelocity = 0.0f;
    self.surfaceSnapCooldown = 0.1f;
  }

  self.yVelocity += GetMouseWheelMove()*16.0f;

  // check if user wants to go into chasm
  if (surfaceSnapped && self.yVelocity < 0.0f) {
    self.y = 0.0f;
    surfaceSnapped = false;
    self.yVelocity = 0.0f;
    self.surfaceSnapCooldown = 0.1f;
  }


  // make sure user cant see above surface limit
  if (surfaceSnapped) {
    self.yVelocity = fmin(self.yVelocity, 0.0f);
  }

  self.y -= self.yVelocity;

  if (abs(self.yVelocity) < 1.0f) self.yVelocity = 0.0f;
  self.yVelocity *= 0.85f;
}
