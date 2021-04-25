#pragma once

#include <raylib.h>

#include <vector>
#include <functional>

namespace ld { struct GameState; }

namespace ld {
  void pathFind(
    ld::GameState const & state,
    std::vector<::Vector2> & path,
    int32_t const origTileX,   int32_t const origTileY,
    int32_t const targetTileX, int32_t const targetTileY,
    bool const canMine,
    int32_t (*tileValue)(GameState const & state, int32_t tileX, int32_t tileY)
  );
}
