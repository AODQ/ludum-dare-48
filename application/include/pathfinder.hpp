#pragma once

#include <raylib.h>

#include <vector>
#include <functional>

namespace ld { struct GameState; }

namespace ld {
  void pathFind(
    ld::GameState const & state,
    std::array<::Vector2, 4> & path, size_t & pathSize,
    int32_t const origTileX,   int32_t const origTileY,
    int32_t       targetTileX, int32_t       targetTileY,
    bool const canMine
  );

  void pathFindInitialize(ld::GameState * state);

  // notably, clear path when a rock has been destroyed
  // this is cheap for multiple continous calls
  void pathClear();
}
