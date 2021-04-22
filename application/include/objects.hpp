#pragma once

#include <raylib.h>

struct Object_Circle {
  static constexpr size_t type = 2;

  uint32_t xPosition = 0, yPosition = 0;
  Color color;
};

struct Object_Rectangle {
  static constexpr size_t type = 0;

  uint32_t xPosition = 0, yPosition = 0;
  Color color;
};
