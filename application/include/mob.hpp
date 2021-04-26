#pragma once

#include <cstdint>

#include <raylib.h>

#include <array>
#include <vector>

namespace ld { struct GameState; }

namespace ld {
  struct MobSlime {
    int32_t animationIdx = 0;

    int32_t positionX, positionY;

    int32_t targetTileX = -1, targetTileY = -1;
    int32_t chasingMinerId = -1;

    int32_t sleepTimer = 0;
    int32_t attackTimer = 0;

    std::array<::Vector2, 4> path;
    size_t pathIdx = 0;
    size_t pathSize = 0;
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
