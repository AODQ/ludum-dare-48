#pragma once

#include <raylib.h>

#include <vector>
#include <functional>

namespace ld { struct GameState; }
namespace ld { struct Miner; }

namespace ld {
  // returns real path length
  uint32_t pathFind(
    ld::GameState const & state,
    std::array<::Vector2, 4> & path, size_t & pathSize,
    int32_t const origTileX,   int32_t const origTileY,
    int32_t       targetTileX, int32_t       targetTileY,
    ld::Miner * miner = nullptr
  );

  void pathFindInitialize(ld::GameState * state);

  // notably, clear path when a rock has been destroyed
  // this is cheap for multiple continous calls
  void pathClear();
}
