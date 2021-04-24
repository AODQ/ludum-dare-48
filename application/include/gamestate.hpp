#pragma once

#include <camera.hpp>
#include <mine.hpp>
#include <miner.hpp>

namespace ld {
  struct GameState
  {
      int32_t food = 100;
      int32_t maxFood = 200;
      int32_t gold = 100;
      uint32_t minerCost = 5;
      int32_t selection = -1;
      bool isPaused = false;

      Camera camera;

      MinerGroup minerGroup;
      MineChasm mineChasm;
  };
}
