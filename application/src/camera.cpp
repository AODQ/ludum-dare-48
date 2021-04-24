#include <camera.hpp>

#include <raylib.h>

#include <cmath>

void ld::Camera::Update() {
  auto & self = *this;
  // snap surface
  if (self.y > -128.0f && self.y < 0.0f && self.yVelocity == 0.0f) {
    self.y = 0.0f;
  }

  if (surfaceSnapped && self.yVelocity < -16.0f) {
    self.y = 0.0f;
    surfaceSnapped = false;
  }

  if (self.y < -128.0f) {
    self.y = -612.0f;
    surfaceSnapped = true;
  }

  self.yVelocity += GetMouseWheelMove()*16.0f;

  self.y -= self.yVelocity;

  if (abs(self.yVelocity) < 1.0f) self.yVelocity = 0.0f;
  self.yVelocity *= 0.85f;
}
