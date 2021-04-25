#pragma once

#include <cstdint>

#include <raylib.h>

#include <vector>

namespace ld { struct GameState; }

namespace ld {
  struct MobSlime {
    int32_t animationIdx = 0;

    int32_t positionX, positionY;

    int32_t targetTileX = -1, targetTileY = -1;
    int32_t chasingMiner = -1;

    std::vector<::Vector2> path;
    size_t pathIdx = 0;
  };

  struct MobPoisonCloud {
    int32_t animationIdx = 0;

    int32_t positionX, positionY;

    int32_t targetTileX = -1, targetTileY = -1;
  };

  struct MobGroup {
    std::vector<MobSlime> slimes;
    std::vector<MobPoisonCloud> poisonClouds;

    static void Update(ld::GameState & state);
    static MobGroup Initialize();
  };
}
