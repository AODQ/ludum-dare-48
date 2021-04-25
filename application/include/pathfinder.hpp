#pragma once

#include <raylib.h>

#include <vector>
#include <functional>

namespace ld { struct GameState; }

namespace ld {
  void pathFind(
    ld::GameState const & state,
    std::array<::Vector2, 32> & path, size_t & pathSize,
    int32_t const origTileX,   int32_t const origTileY,
    int32_t const targetTileX, int32_t const targetTileY,
    bool const canMine
  );

  void pathFindInitialize(ld::GameState * state);

  // notably, clear path when a rock has been destroyed
  // this is cheap for multiple continous calls
  void pathClear();
}
